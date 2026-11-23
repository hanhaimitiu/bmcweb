// SPDX-License-Identifier: Apache-2.0
// SPDX-FileCopyrightText: Copyright OpenBMC Authors
#pragma once

#include "bmcweb_config.h"

#include "app.hpp"
#include "async_resp.hpp"
#include "error_messages.hpp"
#include "http_request.hpp"
#include "http_response.hpp"
#include "logging.hpp"
#include "redfish_aggregator.hpp"
#include "utils/query_param.hpp"

#include <boost/beast/http/field.hpp>
#include <boost/beast/http/verb.hpp>
#include <boost/url/url_view.hpp>

#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

namespace redfish
{
inline void afterIfMatchRequest(
    crow::App& app, const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
    const std::shared_ptr<crow::Request>& req, const std::string& ifMatchHeader,
    const crow::Response& resIn)
{
    std::string currentEtag = resIn.getCurrentEtag();
    BMCWEB_LOG_DEBUG("User provided if-match etag {} current etag {}",
                     ifMatchHeader, currentEtag);
    if (currentEtag != ifMatchHeader)
    {
        messages::preconditionFailed(asyncResp->res);
        return;
    }
    // Restart the request without if-match
    // 去掉 if-match 后重新开始请求
    req->clearHeader(boost::beast::http::field::if_match);
    BMCWEB_LOG_DEBUG("Restarting request");
    app.handle(req, asyncResp);
}

inline bool handleIfMatch(crow::App& app, const crow::Request& req,
                          const std::shared_ptr<bmcweb::AsyncResp>& asyncResp)
{
    if (req.session == nullptr)
    {
        // If the user isn't authenticated, don't even attempt to parse match
        // parameters
        // 如果用户未通过认证，就不要尝试解析 match
        // 参数
        return true;
    }

    std::string ifMatch{
        req.getHeaderValue(boost::beast::http::field::if_match)};
    if (ifMatch.empty())
    {
        // No If-Match header.  Nothing to do
        // 没有 If-Match 头，无需处理
        return true;
    }
    if (ifMatch == "*")
    {
        // Representing any resource
        // https://developer.mozilla.org/en-US/docs/Web/HTTP/Headers/If-Match
        // 表示任意资源
        // https://developer.mozilla.org/en-US/docs/Web/HTTP/Headers/If-Match
        return true;
    }
    if (req.method() != boost::beast::http::verb::patch &&
        req.method() != boost::beast::http::verb::post &&
        req.method() != boost::beast::http::verb::delete_)
    {
        messages::preconditionFailed(asyncResp->res);
        return false;
    }
    boost::system::error_code ec;

    // Try to GET the same resource
    // 尝试 GET 同一资源
    auto getReq = std::make_shared<crow::Request>(
        crow::Request::Body{boost::beast::http::verb::get,
                            req.url().encoded_path(), 11},
        ec);

    if (ec)
    {
        messages::internalError(asyncResp->res);
        return false;
    }

    // New request has the same credentials as the old request
    // 新请求与旧请求使用相同的凭据
    getReq->session = req.session;

    // Construct a new response object to fill in, and check the hash of before
    // we modify the Resource.
    // 构造一个新的响应对象来填充，并在修改资源之前
    // 检查资源的哈希值。
    std::shared_ptr<bmcweb::AsyncResp> getReqAsyncResp =
        std::make_shared<bmcweb::AsyncResp>();

    // Ideally we would have a shared_ptr to the original Request which we could
    // modify to remove the If-Match and restart it. But instead we have to make
    // a full copy to restart it.
    // 理想情况下，我们应该持有一个指向原始 Request 的
    // shared_ptr，以便修改它、去掉 If-Match 并重新开始。但
    // 事实上我们只能做一份完整拷贝来重新开始。
    getReqAsyncResp->res.setCompleteRequestHandler(std::bind_front(
        afterIfMatchRequest, std::ref(app), asyncResp,
        std::make_shared<crow::Request>(req.copy()), std::move(ifMatch)));

    app.handle(getReq, getReqAsyncResp);
    return false;
}

// Sets up the Redfish Route and delegates some of the query parameter
// processing. |queryCapabilities| stores which query parameters will be
// handled by redfish-core/lib codes, then default query parameter handler won't
// process these parameters.
// 设置 Redfish Route 并委派部分查询参数
// 处理。|queryCapabilities| 存储哪些查询参数将由
// redfish-core/lib 代码处理，这样默认的查询参数处理器就
// 不会处理这些参数。
[[nodiscard]] inline bool setUpRedfishRouteWithDelegation(
    crow::App& app, const crow::Request& req,
    const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
    query_param::Query& delegated,
    const query_param::QueryCapabilities& queryCapabilities)
{
    BMCWEB_LOG_DEBUG("setup redfish route");

    // Section 7.4 of the redfish spec "Redfish Services shall process the
    // [OData-Version header] in the following table as defined by the HTTP 1.1
    // specification..."
    // Required to pass redfish-protocol-validator REQ_HEADERS_ODATA_VERSION
    // redfish 规范第 7.4 节："Redfish 服务应按下表处理
    // [OData-Version header]，其定义遵循 HTTP 1.1
    // 规范……"
    // 为通过 redfish-protocol-validator 的 REQ_HEADERS_ODATA_VERSION 检查所必需
    std::string_view odataHeader = req.getHeaderValue("OData-Version");
    if (!odataHeader.empty() && odataHeader != "4.0")
    {
        messages::preconditionFailed(asyncResp->res);
        return false;
    }

    asyncResp->res.addHeader("OData-Version", "4.0");

    std::optional<query_param::Query> queryOpt =
        query_param::parseParameters(req.url().params(), asyncResp->res);
    if (!queryOpt)
    {
        return false;
    }

    if constexpr (!BMCWEB_INSECURE_DISABLE_AUTH)
    {
        // Handle unauthorized expand query parameters for service root example
        // /redfish/v1/?$expand=< >
        // 处理服务根目录未授权的 expand 查询参数，例如
        // /redfish/v1/?$expand=< >
        if (req.session == nullptr &&
            queryOpt->expandType != query_param::ExpandType::None)
        {
            messages::resourceAtUriUnauthorized(asyncResp->res, req.url(),
                                                "Invalid username or password");
            return false;
        }
    }

    if (!handleIfMatch(app, req, asyncResp))
    {
        return false;
    }

    bool needToCallHandlers = true;

    if constexpr (BMCWEB_REDFISH_AGGREGATION)
    {
        needToCallHandlers =
            RedfishAggregator::getInstance().beginAggregation(req, asyncResp) ==
            Result::LocalHandle;

        // If the request should be forwarded to a satellite BMC then we don't
        // want to write anything to the asyncResp since it will get overwritten
        // later.
        // 如果请求应转发给 satellite BMC，则不要向
        // asyncResp 写入任何内容，因为稍后会被
        // 覆盖。
    }

    // If this isn't a get, no need to do anything with parameters
    // 如果不是 get 请求，则无需对参数做任何处理
    if (req.method() != boost::beast::http::verb::get)
    {
        return needToCallHandlers;
    }

    // make a copy of the request because older request goes out of scope
    // trying to access it after it goes out of scope will cause a crash
    // Create a copy of the request using shared_ptr
    // 复制一份请求，因为旧请求会超出作用域
    // 在超出作用域后访问它会导致崩溃
    // 使用 shared_ptr 创建请求的副本

    // The lambda capture of newReq by value in the lambda is causing problems
    // because the lambda needs to be copy-constructible, but crow::Request
    // isn't copy-constructible. So we use a shared_ptr to the request to avoid
    // copying the request.
    // TODO: making a copy for every GET request is expensive,
    // Cleanup is required here.
    // 在 lambda 中按值捕获 newReq 会引发问题，
    // 因为 lambda 需要是可复制构造的，但 crow::Request
    // 不可复制构造。因此我们使用指向请求的 shared_ptr 来避免
    // 复制请求。
    // TODO: 为每个 GET 请求都做一次拷贝开销很大，
    // 这里需要清理优化。
    auto newReq = std::make_shared<crow::Request>(req.copy());

    delegated = query_param::delegate(queryCapabilities, *queryOpt);
    std::function<void(crow::Response&)> handler =
        asyncResp->res.releaseCompleteRequestHandler();

    asyncResp->res.setCompleteRequestHandler(
        [&app, handler(std::move(handler)), query{std::move(*queryOpt)},
         delegated{delegated},
         newReq{std::move(newReq)}](crow::Response& resIn) mutable {
            processAllParams(app, query, delegated, handler, resIn, *newReq);
        });

    return needToCallHandlers;
}

// Sets up the Redfish Route. All parameters are handled by the default handler.
// 设置 Redfish Route。所有参数都由默认处理器处理。
[[nodiscard]] inline bool setUpRedfishRoute(
    crow::App& app, const crow::Request& req,
    const std::shared_ptr<bmcweb::AsyncResp>& asyncResp)
{
    // This route |delegated| is never used
    // 此路由中的 |delegated| 从未被使用
    query_param::Query delegated;
    return setUpRedfishRouteWithDelegation(app, req, asyncResp, delegated,
                                           query_param::QueryCapabilities{});
}
} // namespace redfish
