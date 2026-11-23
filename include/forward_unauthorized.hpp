// SPDX-License-Identifier: Apache-2.0
// SPDX-FileCopyrightText: Copyright OpenBMC Authors
#pragma once
#include "http_response.hpp"
#include "http_utility.hpp"
#include "sessions.hpp"

#include <boost/beast/http/field.hpp>
#include <boost/beast/http/status.hpp>
#include <boost/url/format.hpp>
#include <boost/url/url.hpp>

#include <string_view>

namespace forward_unauthorized
{

inline bool& hasWebuiRoute()
{
    static bool webuiRoute = false;
    return webuiRoute;
}

inline void sendUnauthorized(std::string_view url,
                             std::string_view xRequestedWith,
                             std::string_view accept, crow::Response& res)
{
    // If it's a browser connecting, don't send the HTTP authenticate
    // header, to avoid possible CSRF attacks with basic auth
    // 如果是浏览器连接，则不要发送 HTTP authenticate
    // 头，以避免通过 basic auth 发起的 CSRF 攻击
    if (http_helpers::isContentTypeAllowed(
            accept, http_helpers::ContentType::HTML, false /*allowWildcard*/))
    {
        // If we have a webui installed, redirect to that login page
        // 如果安装了 webui，则重定向到其登录页面
        if (hasWebuiRoute())
        {
            boost::urls::url forward =
                boost::urls::format("/?next={}#/login", url);
            res.result(boost::beast::http::status::temporary_redirect);
            res.addHeader(boost::beast::http::field::location,
                          forward.buffer());
            return;
        }
        // If we don't have a webui installed, just return an unauthorized
        // body
        // 如果没有安装 webui，则只返回一个 unauthorized
        // 响应体
        res.result(boost::beast::http::status::unauthorized);
        res.write(
            "No authentication provided, and no login UI present to forward to.");
        return;
    }

    res.result(boost::beast::http::status::unauthorized);

    // XHR requests from a browser will set the X-Requested-With header when
    // doing their requests, even though they might not be requesting html.
    // 浏览器发出的 XHR 请求在发起请求时会设置 X-Requested-With 头，
    // 即使它们请求的并不是 html。
    if (!xRequestedWith.empty())
    {
        return;
    }
    // if basic auth is disabled, don't propose it.
    // 如果 basic auth 被禁用，则不要提供它。
    if (!persistent_data::SessionStore::getInstance()
             .getAuthMethodsConfig()
             .basic)
    {
        return;
    }
    res.addHeader(boost::beast::http::field::www_authenticate, "Basic");
}
} // namespace forward_unauthorized
