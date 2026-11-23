// SPDX-License-Identifier: Apache-2.0
// SPDX-FileCopyrightText: Copyright OpenBMC Authors
#pragma once

#include "app.hpp"
#include "async_resp.hpp"
#include "error_messages.hpp"
#include "http_request.hpp"
#include "http_response.hpp"
#include "logging.hpp"
#include "ossl_random.hpp"
#include "query.hpp"
#include "redfish_aggregator.hpp"
#include "registries/privilege_registry.hpp"
#include "utility.hpp"
#include "utils/json_utils.hpp"

#include <boost/beast/http/field.hpp>
#include <boost/beast/http/verb.hpp>
#include <boost/system/result.hpp>
#include <boost/url/format.hpp>
#include <boost/url/parse.hpp>
#include <boost/url/url.hpp>
#include <nlohmann/json.hpp>

#include <cstddef>
#include <functional>
#include <memory>
#include <unordered_map>
#include <utility>

namespace redfish
{

inline void handleAggregationServiceHead(
    App& app, const crow::Request& req,
    const std::shared_ptr<bmcweb::AsyncResp>& asyncResp)
{
    if (!redfish::setUpRedfishRoute(app, req, asyncResp))
    {
        return;
    }
    asyncResp->res.addHeader(
        boost::beast::http::field::link,
        "</redfish/v1/JsonSchemas/AggregationService/AggregationService.json>; rel=describedby");
}

inline void handleAggregationServiceGet(
    App& app, const crow::Request& req,
    const std::shared_ptr<bmcweb::AsyncResp>& asyncResp)
{
    if (!redfish::setUpRedfishRoute(app, req, asyncResp))
    {
        return;
    }
    asyncResp->res.addHeader(
        boost::beast::http::field::link,
        "</redfish/v1/JsonSchemas/AggregationService/AggregationService.json>; rel=describedby");
    nlohmann::json& json = asyncResp->res.jsonValue;
    json["@odata.id"] = "/redfish/v1/AggregationService";
    json["@odata.type"] = "#AggregationService.v1_0_1.AggregationService";
    json["Id"] = "AggregationService";
    json["Name"] = "Aggregation Service";
    json["Description"] = "Aggregation Service";
    json["ServiceEnabled"] = true;
    json["AggregationSources"]["@odata.id"] =
        "/redfish/v1/AggregationService/AggregationSources";
}

inline void requestRoutesAggregationService(App& app)
{
    BMCWEB_ROUTE(app, "/redfish/v1/AggregationService/")
        .privileges(redfish::privileges::headAggregationService)
        .methods(boost::beast::http::verb::head)(
            std::bind_front(handleAggregationServiceHead, std::ref(app)));
    BMCWEB_ROUTE(app, "/redfish/v1/AggregationService/")
        .privileges(redfish::privileges::getAggregationService)
        .methods(boost::beast::http::verb::get)(
            std::bind_front(handleAggregationServiceGet, std::ref(app)));
}

inline void populateAggregationSourceCollection(
    const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
    const std::unordered_map<std::string, boost::urls::url>& satelliteInfo)
{
    nlohmann::json::array_t members;
    for (const auto& sat : satelliteInfo)
    {
        nlohmann::json::object_t member;
        member["@odata.id"] = boost::urls::format(
            "/redfish/v1/AggregationService/AggregationSources/{}", sat.first);
        members.emplace_back(std::move(member));
    }
    asyncResp->res.jsonValue["Members@odata.count"] = members.size();
    asyncResp->res.jsonValue["Members"] = std::move(members);
}

inline void handleAggregationSourceCollectionGet(
    App& app, const crow::Request& req,
    const std::shared_ptr<bmcweb::AsyncResp>& asyncResp)
{
    if (!redfish::setUpRedfishRoute(app, req, asyncResp))
    {
        return;
    }
    asyncResp->res.addHeader(
        boost::beast::http::field::link,
        "</redfish/v1/JsonSchemas/AggregationSourceCollection/AggregationSourceCollection.json>; rel=describedby");
    nlohmann::json& json = asyncResp->res.jsonValue;
    json["@odata.id"] = "/redfish/v1/AggregationService/AggregationSources";
    json["@odata.type"] =
        "#AggregationSourceCollection.AggregationSourceCollection";
    json["Name"] = "Aggregation Source Collection";

    // Query D-Bus for satellite configs and add them to the Members array
    // 查询 D-Bus 获取卫星（satellite）配置，并将其加入 Members 数组。
    RedfishAggregator::getInstance().getSatelliteConfigs(
        std::bind_front(populateAggregationSourceCollection, asyncResp));
}

inline void handleAggregationSourceCollectionHead(
    App& app, const crow::Request& req,
    const std::shared_ptr<bmcweb::AsyncResp>& asyncResp)
{
    if (!redfish::setUpRedfishRoute(app, req, asyncResp))
    {
        return;
    }
    asyncResp->res.addHeader(
        boost::beast::http::field::link,
        "</redfish/v1/JsonSchemas/AggregationService/AggregationSourceCollection.json>; rel=describedby");
}

inline void requestRoutesAggregationSourceCollection(App& app)
{
    BMCWEB_ROUTE(app, "/redfish/v1/AggregationService/AggregationSources/")
        .privileges(redfish::privileges::getAggregationSourceCollection)
        .methods(boost::beast::http::verb::get)(std::bind_front(
            handleAggregationSourceCollectionGet, std::ref(app)));

    BMCWEB_ROUTE(app, "/redfish/v1/AggregationService/AggregationSources/")
        .privileges(redfish::privileges::headAggregationSourceCollection)
        .methods(boost::beast::http::verb::head)(std::bind_front(
            handleAggregationSourceCollectionHead, std::ref(app)));
}

inline void populateAggregationSource(
    const std::string& aggregationSourceId,
    const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
    const std::unordered_map<std::string, boost::urls::url>& satelliteInfo)
{
    asyncResp->res.addHeader(
        boost::beast::http::field::link,
        "</redfish/v1/JsonSchemas/AggregationSource/AggregationSource.json>; rel=describedby");

    const auto& sat = satelliteInfo.find(aggregationSourceId);
    if (sat == satelliteInfo.end())
    {
        messages::resourceNotFound(asyncResp->res, "AggregationSource",
                                   aggregationSourceId);
        return;
    }

    asyncResp->res.jsonValue["@odata.id"] = boost::urls::format(
        "/redfish/v1/AggregationService/AggregationSources/{}",
        aggregationSourceId);
    asyncResp->res.jsonValue["@odata.type"] =
        "#AggregationSource.v1_3_1.AggregationSource";
    asyncResp->res.jsonValue["Id"] = aggregationSourceId;

    // TODO: We may want to change this whenever we support aggregating multiple
    // satellite BMCs.  Otherwise all AggregationSource resources will have the
    // same "Name".
    // TODO: We should use the "Name" from the satellite config whenever we add
    // support for including it in the data returned in satelliteInfo.
    // TODO：当我们支持聚合多个卫星 BMC 时，可能需要修改此处。
    // 否则所有 AggregationSource 资源将具有相同的 "Name"。
    // TODO：当支持将卫星配置包含在 satelliteInfo 返回的数据中时，
    // 应使用其中的 "Name"。
    asyncResp->res.jsonValue["Name"] = "Aggregation source";
    std::string hostName(sat->second.encoded_origin());
    asyncResp->res.jsonValue["HostName"] = std::move(hostName);

    // Include UserName property, defaulting to null
    // 包含 UserName 属性，默认值为 null。
    auto& aggregator = RedfishAggregator::getInstance();
    auto it = aggregator.aggregationSources.find(aggregationSourceId);
    if (it != aggregator.aggregationSources.end() &&
        !it->second.username.empty())
    {
        asyncResp->res.jsonValue["UserName"] = it->second.username;
    }
    else
    {
        asyncResp->res.jsonValue["UserName"] = nullptr;
    }

    // The Redfish spec requires Password to be null in responses
    // Redfish 规范要求响应中的 Password 为 null。
    asyncResp->res.jsonValue["Password"] = nullptr;
}

inline void handleAggregationSourceGet(
    App& app, const crow::Request& req,
    const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
    const std::string& aggregationSourceId)
{
    if (!redfish::setUpRedfishRoute(app, req, asyncResp))
    {
        return;
    }

    // Query D-Bus for satellite config corresponding to the specified
    // AggregationSource
    // 查询 D-Bus 获取指定 AggregationSource 对应的卫星配置。
    RedfishAggregator::getInstance().getSatelliteConfigs(std::bind_front(
        populateAggregationSource, aggregationSourceId, asyncResp));
}

inline void handleAggregationSourceHead(
    App& app, const crow::Request& req,
    const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
    const std::string& aggregationSourceId)
{
    if (!redfish::setUpRedfishRoute(app, req, asyncResp))
    {
        return;
    }
    asyncResp->res.addHeader(
        boost::beast::http::field::link,
        "</redfish/v1/JsonSchemas/AggregationService/AggregationSource.json>; rel=describedby");

    // Needed to prevent unused variable error
    // 用于避免未使用变量错误。
    BMCWEB_LOG_DEBUG("Added link header to response from {}",
                     aggregationSourceId);
}

inline bool validateCredentialField(const std::optional<std::string>& field,
                                    const std::string& fieldName,
                                    crow::Response& res)
{
    if (!field.has_value())
    {
        return true; // Field not provided, that's okay
    }

    if (field->empty())
    {
        messages::stringValueTooShort(res, fieldName, 1);
        return false;
    }

    if (field->contains(':'))
    {
        messages::propertyValueIncorrect(res, *field, fieldName);
        return false;
    }

    if (field->length() > 40)
    {
        messages::stringValueTooLong(res, fieldName, 40);
        return false;
    }

    return true;
}

inline void handleAggregationSourceCollectionPost(
    App& app, const crow::Request& req,
    const std::shared_ptr<bmcweb::AsyncResp>& asyncResp)
{
    if (!redfish::setUpRedfishRoute(app, req, asyncResp))
    {
        return;
    }
    std::string hostname;
    std::optional<std::string> username;
    std::optional<std::string> password;

    if (!json_util::readJsonPatch(req, asyncResp->res, "HostName", hostname,
                                  "UserName", username, "Password", password))
    {
        return;
    }

    boost::system::result<boost::urls::url> url =
        boost::urls::parse_absolute_uri(hostname);
    if (!url)
    {
        messages::propertyValueIncorrect(asyncResp->res, "HostName", hostname);
        return;
    }
    url->normalize();
    if (url->scheme() != "http" && url->scheme() != "https")
    {
        messages::propertyValueIncorrect(asyncResp->res, "HostName", hostname);
        return;
    }
    crow::utility::setPortDefaults(*url);

    // Check for duplicate hostname
    // 检查主机名是否重复。
    auto& aggregator = RedfishAggregator::getInstance();
    for (const auto& [existingPrefix, existingSource] :
         aggregator.aggregationSources)
    {
        if (existingSource.url == *url)
        {
            messages::resourceAlreadyExists(asyncResp->res, "AggregationSource",
                                            "HostName", url->buffer());
            return;
        }
    }

    // Validate username and password
    // 校验用户名和密码。
    if (!validateCredentialField(username, "UserName", asyncResp->res))
    {
        return;
    }
    if (!validateCredentialField(password, "Password", asyncResp->res))
    {
        return;
    }

    std::string prefix = bmcweb::getRandomIdOfLength(8);
    aggregator.aggregationSources.emplace(
        prefix,
        AggregationSource{*url, username.value_or(""), password.value_or("")});

    BMCWEB_LOG_DEBUG("Emplaced {} with url {}", prefix, url->buffer());
    asyncResp->res.addHeader(
        boost::beast::http::field::location,
        boost::urls::format("/redfish/v1/AggregationSources/{}", prefix)
            .buffer());
    messages::created(asyncResp->res);
}

inline void afterGetSatelliteConfigsForPatch(
    const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
    const std::string& aggregationSourceId,
    const std::unordered_map<std::string, boost::urls::url>& satelliteInfo)
{
    // Check if it exists in Entity Manager sources
    // 检查其是否存在于 Entity Manager 的配置源中。
    if (satelliteInfo.contains(aggregationSourceId))
    {
        // Source exists but is read-only (from Entity Manager)
        // 源存在但为只读（来自 Entity Manager）。
        messages::propertyNotWritable(asyncResp->res, "UserName");
        return;
    }

    // Doesn't exist anywhere
    // 在任何地方都不存在。
    messages::resourceNotFound(asyncResp->res, "AggregationSource",
                               aggregationSourceId);
}

inline void handleAggregationSourcePatch(
    App& app, const crow::Request& req,
    const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
    const std::string& aggregationSourceId)
{
    if (!redfish::setUpRedfishRoute(app, req, asyncResp))
    {
        return;
    }

    std::optional<std::string> username;
    std::optional<std::string> password;

    if (!json_util::readJsonPatch(req, asyncResp->res, "UserName", username,
                                  "Password", password))
    {
        return;
    }

    // Validate username and password
    // 校验用户名和密码。
    if (!validateCredentialField(username, "UserName", asyncResp->res))
    {
        return;
    }
    if (!validateCredentialField(password, "Password", asyncResp->res))
    {
        return;
    }

    // Check if the aggregation source exists in writable sources
    // 检查该聚合源是否存在于可写的配置源中。
    auto& aggregator = RedfishAggregator::getInstance();
    auto it = aggregator.aggregationSources.find(aggregationSourceId);
    if (it != aggregator.aggregationSources.end())
    {
        // Update only the fields that were provided
        // 只更新提供的字段。
        if (username.has_value())
        {
            it->second.username = *username;
        }
        if (password.has_value())
        {
            it->second.password = *password;
        }

        messages::success(asyncResp->res);
        return;
    }

    // Not in writable sources, query D-Bus to check if it exists in
    // Entity Manager sources
    // 不在可写配置源中，查询 D-Bus 检查其是否存在于 Entity Manager 的配置源中。
    RedfishAggregator::getInstance().getSatelliteConfigs(std::bind_front(
        afterGetSatelliteConfigsForPatch, asyncResp, aggregationSourceId));
}

inline void handleAggregationSourceDelete(
    App& app, const crow::Request& req,
    const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
    const std::string& aggregationSourceId)
{
    if (!redfish::setUpRedfishRoute(app, req, asyncResp))
    {
        return;
    }
    asyncResp->res.addHeader(
        boost::beast::http::field::link,
        "</redfish/v1/JsonSchemas/AggregationService/AggregationSource.json>; rel=describedby");

    size_t deleted = RedfishAggregator::getInstance().aggregationSources.erase(
        aggregationSourceId);
    if (deleted == 0)
    {
        messages::resourceNotFound(asyncResp->res, "AggregationSource",
                                   aggregationSourceId);
        return;
    }

    messages::success(asyncResp->res);
}

inline void requestRoutesAggregationSource(App& app)
{
    BMCWEB_ROUTE(app,
                 "/redfish/v1/AggregationService/AggregationSources/<str>/")
        .privileges(redfish::privileges::getAggregationSource)
        .methods(boost::beast::http::verb::get)(
            std::bind_front(handleAggregationSourceGet, std::ref(app)));

    BMCWEB_ROUTE(app,
                 "/redfish/v1/AggregationService/AggregationSources/<str>/")
        .privileges(redfish::privileges::patchAggregationSource)
        .methods(boost::beast::http::verb::patch)(
            std::bind_front(handleAggregationSourcePatch, std::ref(app)));

    BMCWEB_ROUTE(app,
                 "/redfish/v1/AggregationService/AggregationSources/<str>/")
        .privileges(redfish::privileges::deleteAggregationSource)
        .methods(boost::beast::http::verb::delete_)(
            std::bind_front(handleAggregationSourceDelete, std::ref(app)));

    BMCWEB_ROUTE(app,
                 "/redfish/v1/AggregationService/AggregationSources/<str>/")
        .privileges(redfish::privileges::headAggregationSource)
        .methods(boost::beast::http::verb::head)(
            std::bind_front(handleAggregationSourceHead, std::ref(app)));

    BMCWEB_ROUTE(app, "/redfish/v1/AggregationService/AggregationSources/")
        .privileges(redfish::privileges::postAggregationSourceCollection)
        .methods(boost::beast::http::verb::post)(std::bind_front(
            handleAggregationSourceCollectionPost, std::ref(app)));
}

} // namespace redfish
