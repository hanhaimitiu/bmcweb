// SPDX-License-Identifier: Apache-2.0
// SPDX-FileCopyrightText: Copyright OpenBMC Authors
#pragma once

#include "boost_formatters.hpp"
#include "http_body.hpp"
#include "http_response.hpp"
#include "http_utility.hpp"
#include "json_html_serializer.hpp"
#include "logging.hpp"
#include "security_headers.hpp"

#include <boost/beast/http/field.hpp>
#include <nlohmann/json.hpp>

#include <array>
#include <string>
#include <string_view>
#include <utility>

namespace crow
{

inline bool attemptZstdCompression(Response& res)
{
    using bmcweb::CompressionType;
    using enum bmcweb::CompressionType;
    using http_helpers::Encoding;
    using enum http_helpers::Encoding;

    if (res.response.body().file().is_open())
    {
        return true;
    }

    std::string& strBody = res.response.body().str();
    if (strBody.empty())
    {
        // No need to compress an empty body
        // 空 body 无需压缩
        return true;
    }
    bmcweb::ZstdCompressor zstdCompressor;
    if (!zstdCompressor.init(strBody.size()))
    {
        BMCWEB_LOG_ERROR("Failed to initialize Zstd Compressor");
        return false;
    }

    const uint8_t* dataIn = std::bit_cast<const uint8_t*>(strBody.data());
    std::span<const uint8_t> spanIn(dataIn, strBody.size());
    bool more = false;
    std::optional<std::span<const uint8_t>> compressed =
        zstdCompressor.compress(spanIn, more);
    if (!compressed)
    {
        BMCWEB_LOG_ERROR("Failed to compress content with zstd.");
        return false;
    }
    const char* dataOut = std::bit_cast<const char*>(compressed->data());
    strBody = std::string(dataOut, compressed->size());

    res.addHeader(boost::beast::http::field::content_encoding, "zstd");
    res.response.body().clientCompressionType = Zstd;
    res.response.body().compressionType = Zstd;
    return true;
}

inline void handleEncoding(std::string_view acceptEncoding, Response& res)
{
    using bmcweb::CompressionType;
    using enum bmcweb::CompressionType;
    using http_helpers::Encoding;
    using enum http_helpers::Encoding;
    // If the payload is currently compressed, see if we can avoid
    // decompressing it by sending it to the client directly
    // 如果负载当前已压缩，则考虑能否避免解压，
    // 直接将其发送给客户端
    switch (res.response.body().compressionType)
    {
        case Zstd:
        {
            std::array<Encoding, 1> allowedEnc{ZSTD};
            Encoding encoding =
                http_helpers::getPreferredEncoding(acceptEncoding, allowedEnc);

            if (encoding == ZSTD)
            {
                // If the client supports returning zstd directly, allow that.
                // 如果客户端支持直接返回 zstd，则允许这样做。
                BMCWEB_LOG_DEBUG(
                    "Content is already ztd compressed.  Setting client compression type to Zstd");
                res.response.body().clientCompressionType = Zstd;
            }
        }
        break;
        case Gzip:
        {
            std::array<Encoding, 1> allowedEnc{GZIP};
            Encoding encoding =
                http_helpers::getPreferredEncoding(acceptEncoding, allowedEnc);
            if (encoding != GZIP)
            {
                BMCWEB_LOG_WARNING(
                    "Unimplemented: Returning gzip payload to client that did not explicitly allow it.");
            }
        }
        break;
        case Raw:
        {
            BMCWEB_LOG_DEBUG(
                "Content is raw bytes.  Checking if it can be compressed.");

            std::array<Encoding, 1> allowedEnc{ZSTD};
            Encoding encoding =
                http_helpers::getPreferredEncoding(acceptEncoding, allowedEnc);
            if (encoding == ZSTD)
            {
                BMCWEB_LOG_DEBUG("Content can be compressed with zstd.");
                if (!attemptZstdCompression(res))
                {
                    BMCWEB_LOG_ERROR(
                        "Failed to compress content with zstd.  Continuing.");
                }
            }
        }
        break;
        default:
            break;
    }
}

inline void completeResponseFields(
    std::string_view accepts, std::string_view acceptEncoding, Response& res)
{
    BMCWEB_LOG_INFO("Response: {}", res.resultInt());
    addSecurityHeaders(res);

    res.setResponseEtagAndHandleNotModified();
    if (res.jsonValue.is_structured())
    {
        using http_helpers::ContentType;
        std::array<ContentType, 3> allowed{ContentType::CBOR, ContentType::JSON,
                                           ContentType::HTML};
        ContentType preferred = getPreferredContentType(accepts, allowed);

        if (preferred == ContentType::HTML)
        {
            json_html_util::prettyPrintJson(res);
        }
        else if (preferred == ContentType::CBOR)
        {
            res.addHeader(boost::beast::http::field::content_type,
                          "application/cbor");
            std::string cbor;
            nlohmann::json::to_cbor(res.jsonValue, cbor);
            res.write(std::move(cbor));
        }
        else
        {
            // Technically preferred could also be NoMatch here, but we'd
            // like to default to something rather than return 400 for
            // backward compatibility.
            // 从技术上讲这里也可以用 NoMatch，但为了
            // 向后兼容，我们倾向于给出一个默认值，
            // 而不是返回 400。
            res.addHeader(boost::beast::http::field::content_type,
                          "application/json");
            res.write(res.jsonValue.dump(
                2, ' ', true, nlohmann::json::error_handler_t::replace));
        }
    }

    handleEncoding(acceptEncoding, res);
}
} // namespace crow
