// SPDX-License-Identifier: Apache-2.0
// SPDX-FileCopyrightText: Copyright OpenBMC Authors
#pragma once
#include "http_request.hpp"

#include <boost/beast/http/field.hpp>
#include <boost/beast/http/fields.hpp>
#include <boost/url/url.hpp>

#include <memory>
namespace redfish
{
// Drop any incoming x-auth-token headers and keep Host and Content-Type. Set
// Accept.
// 丢弃传入的 x-auth-token 头，保留 Host 和 Content-Type。
// 设置 Accept。
inline crow::Request createNewRequest(const crow::Request& localReq)
{
    boost::system::error_code ec;
    // Note, this is an expensive copy.  It ideally shouldn't be done, but no
    // option at this point.
    // 注意，这是一次开销很大的拷贝。理想情况下不应这样做，
    // 但目前别无选择。
    crow::Request req(localReq.body(), ec);
    if (ec)
    {
        BMCWEB_LOG_ERROR("Failed to set body.  Continuing");
    }

    // Preserve method and target (URI) from the original request
    // 保留原始请求中的 method 和 target（URI）
    req.method(localReq.method());
    if (!req.target(localReq.target()))
    {
        BMCWEB_LOG_ERROR("Failed to set target on aggregated request");
    }

    for (const auto& field : localReq.fields())
    {
        // Drop any incoming x-auth-token headers and keep Host and
        // Content-Type. Set Accept.
        // 丢弃传入的 x-auth-token 头，保留 Host 和
        // Content-Type。设置 Accept。
        auto headerName = field.name();
        if (headerName == boost::beast::http::field::content_type ||
            headerName == boost::beast::http::field::host)
        {
            req.addHeader(headerName, field.value());
        }
    }
    // Set Accept header to application/json, application/octet-stream
    // 将 Accept 头设置为 application/json, application/octet-stream
    req.addHeader(boost::beast::http::field::accept,
                  "application/json, application/octet-stream");
    return req;
}
} // namespace redfish
