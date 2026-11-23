// SPDX-License-Identifier: Apache-2.0
// SPDX-FileCopyrightText: Copyright OpenBMC Authors
#pragma once

#include "bmcweb_config.h"

#include "http/http_request.hpp"
#include "http_utility.hpp"
#include "logging.hpp"

#include <boost/beast/http/field.hpp>
#include <nlohmann/json.hpp>

#include <cstddef>
#include <optional>
#include <string_view>

enum class JsonParseResult
{
    BadContentType,
    BadJsonData,
    Success,
};

inline bool isJsonContentType(std::string_view contentType)
{
    return http_helpers::getContentType(contentType) ==
           http_helpers::ContentType::JSON;
}

namespace details
{
class BmcwebSaxParse : public nlohmann::json::json_sax_t
{
  private:
// Note, this is in the nlomann details namespace, but is mentioned in the
// nlohmann documentation. Likely because they don't want to treat it as part of
// ABI. While not ideal, it is done rather than copy/pasting the code from
// nlohmann which would be worse.
// 注意，它位于 nlohmann 的 details 命名空间中，但在
// nlohmann 文档里有提及。可能是因为他们不想把它当作 ABI
// 的一部分。虽然并不理想，但这样做总比从 nlohmann
// 复制粘贴代码要好。
#if NLOHMANN_JSON_VERSION_MAJOR > 3 ||                                         \
    (NLOHMANN_JSON_VERSION_MAJOR == 3 && NLOHMANN_JSON_VERSION_MINOR >= 12)
    nlohmann::detail::json_sax_dom_parser<
        nlohmann::json, nlohmann::detail::string_input_adapter_type>
        parser;
#else
    nlohmann::detail::json_sax_dom_parser<nlohmann::json> parser;
#endif

    // Depth counter treating arrays and objects as the same level
    // 深度计数器，将数组和对象视为同一层级
    int currentDepth = 0;
    constexpr static int maxDepth = 10;

    int totalValues = 0;
    constexpr static int maxValues = 5000;

  public:
    BmcwebSaxParse(nlohmann::json& j) : parser(j, false) {}

    bool null() override
    {
        totalValues++;
        if (totalValues > maxValues)
        {
            return false;
        }
        return parser.null();
    }

    bool boolean(bool val) override
    {
        totalValues++;
        if (totalValues > maxValues)
        {
            return false;
        }
        return parser.boolean(val);
    }

    bool number_integer(std::int64_t val) override
    {
        totalValues++;
        if (totalValues > maxValues)
        {
            return false;
        }
        return parser.number_integer(val);
    }

    bool number_unsigned(std::uint64_t val) override
    {
        totalValues++;
        if (totalValues > maxValues)
        {
            return false;
        }
        return parser.number_unsigned(val);
    }

    bool number_float(double val, const std::string& s) override
    {
        totalValues++;
        if (totalValues > maxValues)
        {
            return false;
        }
        return parser.number_float(val, s);
    }

    bool string(std::string& val) override
    {
        totalValues++;
        if (totalValues > maxValues)
        {
            return false;
        }
        return parser.string(val);
    }

    bool start_object(std::size_t elements) override
    {
        currentDepth++;
        if (currentDepth > maxDepth)
        {
            return false;
        }
        totalValues++;
        if (totalValues > maxValues)
        {
            return false;
        }
        return parser.start_object(elements);
    }

    bool end_object() override
    {
        currentDepth--;
        return parser.end_object();
    }

    bool start_array(std::size_t elements) override
    {
        currentDepth++;
        if (currentDepth > maxDepth)
        {
            return false;
        }
        totalValues++;
        if (totalValues > maxValues)
        {
            return false;
        }

        return parser.start_array(elements);
    }

    bool end_array() override
    {
        currentDepth--;
        return parser.end_array();
    }

    bool key(std::string& val) override
    {
        totalValues++;
        if (totalValues > maxValues)
        {
            return false;
        }
        return parser.key(val);
    }

    bool binary(nlohmann::json::binary_t& val) override
    {
        totalValues++;
        if (totalValues > maxValues)
        {
            return false;
        }
        return parser.binary(val);
    }

    bool parse_error(std::size_t position, const std::string& lastToken,
                     const nlohmann::json::exception& ex) override
    {
        BMCWEB_LOG_WARNING(
            "Stopped parsing at position {}. Last token: {}. Exception: {}",
            position, lastToken, ex.what());
        return parser.parse_error(position, lastToken, ex);
    }
};
} // namespace details

inline std::optional<nlohmann::json> parseStringAsJson(std::string_view body)
{
    nlohmann::json jsonOut;
    // Arbitrarily limit to 1MB payloads
    // 任意限制为 1MB 的 payload
    if (body.size() > 1048576U)
    {
        BMCWEB_LOG_WARNING("Request body is too large");
        return std::nullopt;
    }
    details::BmcwebSaxParse sax(jsonOut);

    if (!nlohmann::json::sax_parse(body, &sax))
    {
        BMCWEB_LOG_WARNING("Failed to parse json in request");
        return std::nullopt;
    }

    return jsonOut;
}

inline JsonParseResult parseRequestAsJson(const crow::Request& req,
                                          nlohmann::json& jsonOut)
{
    std::string_view contentType =
        req.getHeaderValue(boost::beast::http::field::content_type);
    if (!isJsonContentType(contentType))
    {
        if (contentType.empty())
        {
            BMCWEB_LOG_WARNING("Content-Type header was empty");
        }
        if constexpr (!BMCWEB_INSECURE_IGNORE_CONTENT_TYPE)
        {
            return JsonParseResult::BadContentType;
        }
    }
    std::optional<nlohmann::json> obj = parseStringAsJson(req.body());
    if (!obj)
    {
        return JsonParseResult::BadJsonData;
    }

    jsonOut = std::move(*obj);

    return JsonParseResult::Success;
}
