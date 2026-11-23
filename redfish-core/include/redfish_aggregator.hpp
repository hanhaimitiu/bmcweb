// SPDX-License-Identifier: Apache-2.0
// SPDX-FileCopyrightText: Copyright OpenBMC Authors
#pragma once

#include "aggregation_utils.hpp"
#include "async_resp.hpp"
#include "dbus_utility.hpp"
#include "error_messages.hpp"
#include "http_client.hpp"
#include "http_request.hpp"
#include "http_response.hpp"
#include "io_context_singleton.hpp"
#include "logging.hpp"
#include "parsing.hpp"
#include "ssl_key_handler.hpp"
#include "utility.hpp"
#include "utils/collection.hpp"
#include "utils/redfish_aggregator_utils.hpp"

#include <boost/beast/http/field.hpp>
#include <boost/beast/http/status.hpp>
#include <boost/beast/http/verb.hpp>
#include <boost/system/errc.hpp>
#include <boost/system/result.hpp>
#include <boost/url/param.hpp>
#include <boost/url/parse.hpp>
#include <boost/url/segments_ref.hpp>
#include <boost/url/segments_view.hpp>
#include <boost/url/url.hpp>
#include <boost/url/url_view.hpp>
#include <nlohmann/json.hpp>
#include <sdbusplus/message/native_types.hpp>

#include <algorithm>
#include <array>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <limits>
#include <memory>
#include <ranges>
#include <string>
#include <string_view>
#include <system_error>
#include <unordered_map>
#include <utility>
#include <variant>

namespace redfish
{

constexpr unsigned int aggregatorReadBodyLimit = 50 * 1024 * 1024; // 50MB

enum class Result
{
    LocalHandle,
    NoLocalHandle
};

enum class SearchType
{
    Collection,
    CollOrCon,
    ContainsSubordinate,
    Resource
};

// clang-format off
// These are all of the properties as of version 2022.2 of the Redfish Resource
// and Schema Guide whose Type is "string (URI)" and the name does not end in a
// case-insensitive form of "uri".  That version of the schema is associated
// with version 1.16.0 of the Redfish Specification.  Going forward, new URI
// properties should end in URI so this list should not need to be maintained as
// the spec is updated.  NOTE: These have been pre-sorted in order to be
// compatible with binary search
// clang-format off
// 以下是 Redfish Resource and Schema Guide 2022.2 版本中
// Type 为 "string (URI)"、且名称不以大小写不敏感的
// "uri" 结尾的所有属性。该 schema 版本对应
// Redfish Specification 1.16.0。今后新的 URI
// 属性都应以 URI 结尾，因此随着规范更新，此列表
// 无需再维护。注意：这些属性已预先排序，以便
// 使用二分查找
constexpr std::array nonUriProperties{
    "@Redfish.ActionInfo",
    // "@odata.context", // We can't fix /redfish/v1/$metadata URIs
    // "@odata.context", // 我们无法修复 /redfish/v1/$metadata 的 URI
    "@odata.id",
    // "Destination", // Only used by EventService and won't be a Redfish URI
    // "HostName", // Isn't actually a Redfish URI
    // "Destination", // 仅供 EventService 使用，不会是 Redfish URI
    // "HostName", // 实际上并不是 Redfish URI
    "Image",
    "MetricProperty",
    // "OriginOfCondition", // Is URI when in request, but is object in response
    // "OriginOfCondition", // 在请求中是 URI，但在响应中是对象
    "TaskMonitor",
    "target", // normal string, but target URI for POST to invoke an action
};
// clang-format on

// Search the top collection array to determine if the passed URI is of a
// desired type
// 搜索顶层集合数组，判断传入的 URI 是否属于
// 所需的类型
inline bool searchCollectionsArray(std::string_view uri,
                                   const SearchType searchType)
{
    boost::system::result<boost::urls::url> parsedUrl =
        boost::urls::parse_relative_ref(uri);
    if (!parsedUrl)
    {
        BMCWEB_LOG_ERROR("Failed to get target URI from {}", uri);
        return false;
    }

    parsedUrl->normalize();
    boost::urls::segments_ref segments = parsedUrl->segments();
    if (!segments.is_absolute())
    {
        return false;
    }

    // The passed URI must begin with "/redfish/v1", but we have to strip it
    // from the URI since topCollections does not include it in its URIs.
    // 传入的 URI 必须以 "/redfish/v1" 开头，但必须将其从
    // URI 中去掉，因为 topCollections 的 URI 中不包含它。
    if (segments.size() < 2)
    {
        return false;
    }
    if (segments.front() != "redfish")
    {
        return false;
    }
    segments.erase(segments.begin());
    if (segments.front() != "v1")
    {
        return false;
    }
    segments.erase(segments.begin());

    // Exclude the trailing "/" if it exists such as in "/redfish/v1/".
    // 去掉末尾的 "/"（如果存在），例如 "/redfish/v1/"。
    if (!segments.empty() && segments.back().empty())
    {
        segments.pop_back();
    }

    // If no segments then the passed URI was either "/redfish/v1" or
    // "/redfish/v1/".
    // 如果没有任何段，则传入的 URI 是 "/redfish/v1" 或
    // "/redfish/v1/"。
    if (segments.empty())
    {
        return (searchType == SearchType::ContainsSubordinate) ||
               (searchType == SearchType::CollOrCon);
    }
    std::string_view url = segments.buffer();
    const auto* it = std::ranges::lower_bound(topCollections, url);
    if (it == topCollections.end())
    {
        // parsedUrl is alphabetically after the last entry in the array so it
        // can't be a top collection or up tree from a top collection
        // parsedUrl 按字母顺序排在数组最后一项之后，因此它
        // 不可能是顶层集合，也不可能是顶层集合的上级路径
        return false;
    }

    boost::urls::url collectionUrl(*it);
    boost::urls::segments_view collectionSegments = collectionUrl.segments();
    boost::urls::segments_view::iterator itCollection =
        collectionSegments.begin();
    const boost::urls::segments_view::const_iterator endCollection =
        collectionSegments.end();

    // Each segment in the passed URI should match the found collection
    // 传入 URI 中的每一段都应与找到的集合匹配
    for (const auto& segment : segments)
    {
        if (itCollection == endCollection)
        {
            // Leftover segments means the target is for an aggregation
            // supported resource
            // 有剩余段意味着目标是受聚合支持的
            // 资源
            return searchType == SearchType::Resource;
        }

        if (segment != (*itCollection))
        {
            return false;
        }
        itCollection++;
    }

    // No remaining segments means the passed URI was a top level collection
    // 没有剩余段意味着传入的 URI 是一个顶层集合
    if (searchType == SearchType::Collection)
    {
        return itCollection == endCollection;
    }
    if (searchType == SearchType::ContainsSubordinate)
    {
        return itCollection != endCollection;
    }

    // Return this check instead of "true" in case other SearchTypes get added
    // 返回此检查结果而不是 "true"，以防将来添加其他 SearchType
    return searchType == SearchType::CollOrCon;
}

// Determines if the passed property contains a URI.  Those property names
// either end with a case-insensitive version of "uri" or are specifically
// defined in the above array.
// 判断传入的属性是否包含 URI。这些属性名要么以
// 大小写不敏感的 "uri" 结尾，要么在上面数组中
// 专门定义。
inline bool isPropertyUri(std::string_view propertyName)
{
    if (propertyName.ends_with("uri") || propertyName.ends_with("Uri") ||
        propertyName.ends_with("URI"))
    {
        return true;
    }
    return std::ranges::binary_search(nonUriProperties, propertyName);
}

inline void addPrefixToStringItem(std::string& strValue,
                                  std::string_view prefix)
{
    // Make sure the value is a properly formatted URI
    // 确保该值是格式正确的 URI
    auto parsed = boost::urls::parse_relative_ref(strValue);
    if (!parsed)
    {
        // Note that DMTF URIs such as
        // https://redfish.dmtf.org/registries/Base.1.15.0.json will fail this
        // check and that's okay
        // 注意，DMTF 的 URI（例如
        // https://redfish.dmtf.org/registries/Base.1.15.0.json）无法通过此
        // 检查，这是可以接受的
        BMCWEB_LOG_DEBUG("Couldn't parse URI from resource {}", strValue);
        return;
    }

    const boost::urls::url_view& thisUrl = *parsed;

    // We don't need to aggregate JsonSchemas due to potential issues such as
    // version mismatches between aggregator and satellite BMCs.  For now
    // assume that the aggregator has all the schemas and versions that the
    // aggregated server has.
    // 由于聚合器与 satellite BMC 之间可能存在版本不匹配等问题，
    // 我们不需要聚合 JsonSchemas。目前
    // 假设聚合器拥有被聚合服务器所拥有的
    // 所有 schema 和版本。
    if (crow::utility::readUrlSegments(thisUrl, "redfish", "v1", "JsonSchemas",
                                       crow::utility::OrMorePaths()))
    {
        BMCWEB_LOG_DEBUG("Skipping JsonSchemas URI prefix fixing");
        return;
    }

    // The first two segments should be "/redfish/v1".  We need to check that
    // before we can search topCollections
    // 前两段应为 "/redfish/v1"。在搜索 topCollections
    // 之前需要先检查这一点
    if (!crow::utility::readUrlSegments(thisUrl, "redfish", "v1",
                                        crow::utility::OrMorePaths()))
    {
        return;
    }

    // Check array adding a segment each time until collection is identified
    // Add prefix to segment after the collection
    // 遍历数组，每次添加一段，直到识别出集合
    // 在集合之后的那一段上添加前缀
    const boost::urls::segments_view urlSegments = thisUrl.segments();
    bool addedPrefix = false;
    boost::urls::url url("/");
    boost::urls::segments_view::const_iterator it = urlSegments.begin();
    const boost::urls::segments_view::const_iterator end = urlSegments.end();

    // Skip past the leading "/redfish/v1"
    // 跳过开头的 "/redfish/v1"
    it++;
    it++;
    for (; it != end; it++)
    {
        // Trailing "/" will result in an empty segment.  In that case we need
        // to return so we don't apply a prefix to top level collections such
        // as "/redfish/v1/Chassis/"
        // 末尾的 "/" 会产生一个空段。此时需要
        // 返回，以免给 "/redfish/v1/Chassis/" 这样的
        // 顶层集合加上前缀
        if ((*it).empty())
        {
            return;
        }

        if (std::ranges::binary_search(topCollections,
                                       std::string_view(url.buffer())))
        {
            std::string collectionItem(prefix);
            collectionItem += "_" + (*it);
            url.segments().push_back(collectionItem);
            it++;
            addedPrefix = true;
            break;
        }

        url.segments().push_back(*it);
    }

    // Finish constructing the URL here (if needed) to avoid additional checks
    // 在此处完成 URL 的构造（如需要），以避免额外检查
    for (; it != end; it++)
    {
        url.segments().push_back(*it);
    }

    if (addedPrefix)
    {
        url.segments().insert(url.segments().begin(), {"redfish", "v1"});
        strValue = url.buffer();
    }
}

inline void addPrefixToItem(nlohmann::json& item, std::string_view prefix)
{
    std::string* strValue = item.get_ptr<std::string*>();
    if (strValue == nullptr)
    {
        // Values for properties like "InvalidURI" and "ResourceMissingAtURI"
        // from within the Base Registry are objects instead of strings and will
        // fall into this check
        // Base Registry 中 "InvalidURI" 和 "ResourceMissingAtURI"
        // 等属性的值是对象而非字符串，会落入
        // 此检查分支
        BMCWEB_LOG_DEBUG("Field was not a string");
        return;
    }
    addPrefixToStringItem(*strValue, prefix);
    item = *strValue;
}

inline void addAggregatedHeaders(crow::Response& asyncResp,
                                 const crow::Response& resp,
                                 std::string_view prefix)
{
    if (!resp.getHeaderValue("Content-Type").empty())
    {
        asyncResp.addHeader(boost::beast::http::field::content_type,
                            resp.getHeaderValue("Content-Type"));
    }
    if (!resp.getHeaderValue("Allow").empty())
    {
        asyncResp.addHeader(boost::beast::http::field::allow,
                            resp.getHeaderValue("Allow"));
    }
    std::string_view header = resp.getHeaderValue("Location");
    if (!header.empty())
    {
        std::string location(header);
        addPrefixToStringItem(location, prefix);
        asyncResp.addHeader(boost::beast::http::field::location, location);
    }
    if (!resp.getHeaderValue("Retry-After").empty())
    {
        asyncResp.addHeader(boost::beast::http::field::retry_after,
                            resp.getHeaderValue("Retry-After"));
    }
    // TODO: we need special handling for Link Header Value
    // TODO: 需要对 Link Header 的值做特殊处理
}

// Fix HTTP headers which appear in responses from Task resources among others
// 修复 Task 资源等响应中出现的 HTTP 头
inline void addPrefixToHeadersInResp(nlohmann::json& json,
                                     std::string_view prefix)
{
    // The passed in "HttpHeaders" should be an array of headers
    // 传入的 "HttpHeaders" 应为头部组成的数组
    nlohmann::json::array_t* array = json.get_ptr<nlohmann::json::array_t*>();
    if (array == nullptr)
    {
        BMCWEB_LOG_ERROR("Field wasn't an array_t????");
        return;
    }

    for (nlohmann::json& item : *array)
    {
        // Each header is a single string with the form "<Field>: <Value>"
        // 每个头部是形如 "<Field>: <Value>" 的单个字符串
        std::string* strHeader = item.get_ptr<std::string*>();
        if (strHeader == nullptr)
        {
            BMCWEB_LOG_CRITICAL("Field wasn't a string????");
            continue;
        }

        constexpr std::string_view location = "Location: ";
        if (strHeader->starts_with(location))
        {
            std::string header = strHeader->substr(location.size());
            addPrefixToStringItem(header, prefix);
            *strHeader = std::string(location) + header;
        }
    }
}

// Search the json for all URIs and add the supplied prefix if the URI is for
// an aggregated resource.
// 在 json 中搜索所有 URI，若该 URI 指向被聚合的
// 资源，则加上给定的前缀。
inline void addPrefixes(nlohmann::json& json, std::string_view prefix)
{
    nlohmann::json::object_t* object =
        json.get_ptr<nlohmann::json::object_t*>();
    if (object != nullptr)
    {
        for (std::pair<const std::string, nlohmann::json>& item : *object)
        {
            if (isPropertyUri(item.first))
            {
                addPrefixToItem(item.second, prefix);
                continue;
            }

            // "HttpHeaders" contains HTTP headers.  Among those we need to
            // attempt to fix the "Location" header
            // "HttpHeaders" 包含 HTTP 头。其中我们需要
            // 尝试修复 "Location" 头
            if (item.first == "HttpHeaders")
            {
                addPrefixToHeadersInResp(item.second, prefix);
                continue;
            }

            // Recursively parse the rest of the json
            // 递归解析 json 的其余部分
            addPrefixes(item.second, prefix);
        }
        return;
    }
    nlohmann::json::array_t* array = json.get_ptr<nlohmann::json::array_t*>();
    if (array != nullptr)
    {
        for (nlohmann::json& item : *array)
        {
            addPrefixes(item, prefix);
        }
    }
}

inline boost::system::error_code aggregationRetryHandler(unsigned int respCode)
{
    // Allow all response codes because we want to surface any satellite
    // issue to the client
    // 允许所有响应码，因为我们希望将 satellite 的任何
    // 问题都呈现给客户端
    BMCWEB_LOG_DEBUG("Received {} response from satellite", respCode);
    return boost::system::errc::make_error_code(boost::system::errc::success);
}

inline crow::ConnectionPolicy getAggregationPolicy()
{
    return {.maxRetryAttempts = 0,
            .requestByteLimit = aggregatorReadBodyLimit,
            .maxConnections = 20,
            .retryPolicyAction = "TerminateAfterRetries",
            .retryIntervalSecs = std::chrono::seconds(0),
            .invalidResp = aggregationRetryHandler};
}

struct AggregationSource
{
    boost::urls::url url;
    std::string username;
    std::string password;
};

class RedfishAggregator
{
  private:
    crow::HttpClient client;

    // Dummy callback used by the Constructor so that it can report the number
    // of satellite configs when the class is first created
    // 构造函数使用的占位回调，以便在类首次创建时
    // 报告 satellite 配置的数量
    static void constructorCallback(
        const std::unordered_map<std::string, boost::urls::url>& satelliteInfo)
    {
        BMCWEB_LOG_DEBUG("There were {} satellite configs found at startup",
                         std::to_string(satelliteInfo.size()));
    }

    // Search D-Bus objects for satellite config objects and add their
    // information if valid
    // 在 D-Bus 对象中搜索 satellite 配置对象，若有效则
    // 添加其信息
    static void findSatelliteConfigs(
        const dbus::utility::ManagedObjectType& objects,
        std::unordered_map<std::string, boost::urls::url>& satelliteInfo)
    {
        for (const auto& objectPath : objects)
        {
            for (const auto& interface : objectPath.second)
            {
                if (interface.first ==
                    "xyz.openbmc_project.Configuration.SatelliteController")
                {
                    BMCWEB_LOG_DEBUG("Found Satellite Controller at {}",
                                     objectPath.first.str);

                    if (!satelliteInfo.empty())
                    {
                        BMCWEB_LOG_ERROR(
                            "Redfish Aggregation only supports one satellite!");
                        BMCWEB_LOG_DEBUG("Clearing all satellite data");
                        satelliteInfo.clear();
                        return;
                    }

                    addSatelliteConfig(interface.second, satelliteInfo);
                }
            }
        }
    }

    // Parse the properties of a satellite config object and add the
    // configuration if the properties are valid
    // 解析 satellite 配置对象的属性，若属性有效则
    // 添加该配置
    static void addSatelliteConfig(
        const dbus::utility::DBusPropertiesMap& properties,
        std::unordered_map<std::string, boost::urls::url>& satelliteInfo)
    {
        boost::urls::url url;
        std::string prefix;

        for (const auto& prop : properties)
        {
            if (prop.first == "Hostname")
            {
                const std::string* propVal =
                    std::get_if<std::string>(&prop.second);
                if (propVal == nullptr)
                {
                    BMCWEB_LOG_ERROR("Invalid Hostname value");
                    return;
                }
                url.set_host(*propVal);
            }

            else if (prop.first == "Port")
            {
                const uint64_t* propVal = std::get_if<uint64_t>(&prop.second);
                if (propVal == nullptr)
                {
                    BMCWEB_LOG_ERROR("Invalid Port value");
                    return;
                }

                if (*propVal > std::numeric_limits<uint16_t>::max())
                {
                    BMCWEB_LOG_ERROR("Port value out of range");
                    return;
                }
                url.set_port(std::to_string(static_cast<uint16_t>(*propVal)));
            }

            else if (prop.first == "AuthType")
            {
                const std::string* propVal =
                    std::get_if<std::string>(&prop.second);
                if (propVal == nullptr)
                {
                    BMCWEB_LOG_ERROR("Invalid AuthType value");
                    return;
                }

                // For now assume authentication not required to communicate
                // with the satellite BMC
                // 目前假设与 satellite BMC 通信
                // 不需要认证
                if (*propVal != "None")
                {
                    BMCWEB_LOG_ERROR(
                        "Unsupported AuthType value: {}, only \"none\" is supported",
                        *propVal);
                    return;
                }
                url.set_scheme("http");
            }
            else if (prop.first == "Name")
            {
                const std::string* propVal =
                    std::get_if<std::string>(&prop.second);
                if (propVal != nullptr && !propVal->empty())
                {
                    prefix = *propVal;
                    BMCWEB_LOG_DEBUG("Using Name property {} as prefix",
                                     prefix);
                }
                else
                {
                    BMCWEB_LOG_DEBUG(
                        "Invalid or empty Name property, invalid satellite config");
                    return;
                }
            }
        } // Finished reading properties

        // Make sure all required config information was made available
        // 确保所有必需的配置信息都已提供
        if (url.host().empty())
        {
            BMCWEB_LOG_ERROR("Satellite config {} missing Host", prefix);
            return;
        }

        if (!url.has_port())
        {
            BMCWEB_LOG_ERROR("Satellite config {} missing Port", prefix);
            return;
        }

        if (!url.has_scheme())
        {
            BMCWEB_LOG_ERROR("Satellite config {} missing AuthType", prefix);
            return;
        }

        std::string resultString;
        auto result = satelliteInfo.insert_or_assign(prefix, std::move(url));
        if (result.second)
        {
            resultString = "Added new satellite config ";
        }
        else
        {
            resultString = "Updated existing satellite config ";
        }

        BMCWEB_LOG_DEBUG("{}{} at {}://{}", resultString, prefix,
                         result.first->second.scheme(),
                         result.first->second.encoded_host_and_port());
    }

    enum AggregationType
    {
        Collection,
        ContainsSubordinate,
        Resource,
    };

    void startAggregation(
        AggregationType aggType, const crow::Request& thisReq,
        const std::shared_ptr<bmcweb::AsyncResp>& asyncResp) const
    {
        if (thisReq.method() != boost::beast::http::verb::get)
        {
            if (aggType == AggregationType::Collection)
            {
                BMCWEB_LOG_DEBUG(
                    "Only aggregate GET requests to top level collections");
                return;
            }

            if (aggType == AggregationType::ContainsSubordinate)
            {
                BMCWEB_LOG_DEBUG(
                    "Only aggregate GET requests when uptree of a top level collection");
                return;
            }
        }

        std::error_code ec;
        // Create a filtered copy of the request
        // 创建请求的过滤副本
        auto localReq =
            std::make_shared<crow::Request>(createNewRequest(thisReq));
        if (ec)
        {
            BMCWEB_LOG_ERROR("Failed to create copy of request");
            if (aggType == AggregationType::Resource)
            {
                messages::internalError(asyncResp->res);
            }
            return;
        }

        boost::urls::url& urlNew = localReq->url();
        if (aggType == AggregationType::Collection)
        {
            auto paramsIt = urlNew.params().begin();
            while (paramsIt != urlNew.params().end())
            {
                const boost::urls::param& param = *paramsIt;
                // only and $skip, params can't be passed to satellite
                // as applying these filters twice results in different results.
                // Removing them will cause them to only be processed in the
                // aggregator. Note, this still doesn't work for collections
                // that might return less than the complete collection by
                // default, but hopefully those are rare/nonexistent in top
                // collections.  bmcweb doesn't implement any of these.
                // only 和 $skip 参数不能传递给 satellite，
                // 因为对这两个过滤器应用两次会得到不同结果。
                // 移除它们会使其只在聚合器中
                // 处理。注意，对于默认可能返回不完整集合的
                // 集合，这仍然不起作用，但希望顶层集合中
                // 这种情况很少或不存在。bmcweb 未实现
                // 其中任何一项。
                if (param.key == "only" || param.key == "$skip")
                {
                    BMCWEB_LOG_DEBUG(
                        "Erasing \"{}\" param from request to top level collection",
                        param.key);

                    paramsIt = urlNew.params().erase(paramsIt);
                    continue;
                }
                // Pass all other parameters
                // 传递所有其他参数
                paramsIt++;
            }
        }
        // Filter headers to only allow Host and Content-Type
        // 过滤头部，只允许 Host 和 Content-Type
        localReq->target(urlNew.buffer());
        getSatelliteConfigs(
            std::bind_front(aggregateAndHandle, aggType, localReq, asyncResp));
    }

    static void findSatellite(
        const crow::Request& req,
        const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
        const std::unordered_map<std::string, boost::urls::url>& satelliteInfo,
        std::string_view memberName)
    {
        // Determine if the resource ID begins with a known prefix
        // 判断资源 ID 是否以已知前缀开头
        for (const auto& satellite : satelliteInfo)
        {
            std::string targetPrefix = satellite.first;
            targetPrefix += "_";
            if (memberName.starts_with(targetPrefix))
            {
                BMCWEB_LOG_DEBUG("\"{}\" is a known prefix", satellite.first);

                // Remove the known prefix from the request's URI and
                // then forward to the associated satellite BMC
                // 从请求的 URI 中去掉已知前缀，
                // 然后转发给相应的 satellite BMC
                getInstance().forwardRequest(req, asyncResp, satellite.first,
                                             satelliteInfo);
                return;
            }
        }

        // We didn't recognize the prefix and need to return a 404
        // 无法识别该前缀，需要返回 404
        std::string nameStr = req.url().segments().back();
        messages::resourceNotFound(asyncResp->res, "", nameStr);
    }

    // Intended to handle an incoming request based on if Redfish Aggregation
    // is enabled.  Forwards request to satellite BMC if it exists.
    // 用于根据 Redfish 聚合功能是否启用来处理传入请求。
    // 若存在 satellite BMC，则将请求转发给它。
    static void aggregateAndHandle(
        AggregationType aggType,
        const std::shared_ptr<crow::Request>& sharedReq,
        const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
        const std::unordered_map<std::string, boost::urls::url>& satelliteInfo)
    {
        if (sharedReq == nullptr)
        {
            return;
        }

        // No satellite configs means we don't need to keep attempting to
        // aggregate
        // 没有 satellite 配置意味着无需继续尝试
        // 聚合
        if (satelliteInfo.empty())
        {
            // For collections or resources that can contain a subordinate
            // top level collection we'll also handle the request locally so we
            // don't need to write an error code
            // 对于可能包含下级顶层集合的集合或资源，我们也会
            // 在本地处理该请求，因此无需
            // 写入错误码
            if (aggType == AggregationType::Resource)
            {
                std::string nameStr = sharedReq->url().segments().back();
                messages::resourceNotFound(asyncResp->res, "", nameStr);
            }
            return;
        }

        const crow::Request& thisReq = *sharedReq;
        BMCWEB_LOG_DEBUG("Aggregation is enabled, begin processing of {}",
                         thisReq.target());

        // We previously determined the request is for a collection.  No need to
        // check again
        // 此前已确定该请求针对的是集合，无需
        // 再次检查
        if (aggType == AggregationType::Collection)
        {
            BMCWEB_LOG_DEBUG("Aggregating a collection");
            // We need to use a specific response handler and send the
            // request to all known satellites
            // 我们需要使用特定的响应处理器，并将请求
            // 发送给所有已知的 satellite
            getInstance().forwardCollectionRequests(thisReq, asyncResp,
                                                    satelliteInfo);
            return;
        }

        // We previously determined the request may contain a subordinate
        // collection.  No need to check again
        // 此前已确定该请求可能包含下级集合，无需
        // 再次检查
        if (aggType == AggregationType::ContainsSubordinate)
        {
            BMCWEB_LOG_DEBUG(
                "Aggregating what may have a subordinate collection");
            // We need to use a specific response handler and send the
            // request to all known satellites
            // 我们需要使用特定的响应处理器，并将请求
            // 发送给所有已知的 satellite
            getInstance().forwardContainsSubordinateRequests(thisReq, asyncResp,
                                                             satelliteInfo);
            return;
        }

        const boost::urls::segments_view urlSegments = thisReq.url().segments();
        boost::urls::url currentUrl("/");
        boost::urls::segments_view::const_iterator it = urlSegments.begin();
        boost::urls::segments_view::const_iterator end = urlSegments.end();

        // Skip past the leading "/redfish/v1"
        // 跳过开头的 "/redfish/v1"
        it++;
        it++;
        for (; it != end; it++)
        {
            if (std::ranges::binary_search(
                    topCollections, std::string_view(currentUrl.buffer())))
            {
                // We've matched a resource collection so this current segment
                // must contain an aggregation prefix
                // 我们已匹配到一个资源集合，因此当前这一段
                // 必定包含聚合前缀
                findSatellite(thisReq, asyncResp, satelliteInfo, *it);
                return;
            }

            currentUrl.segments().push_back(*it);
        }

        // We shouldn't reach this point since we should've hit one of the
        // previous exits
        // 我们不应执行到这里，因为此前应该已经
        // 从某个出口返回了
        messages::internalError(asyncResp->res);
    }

    // Attempt to forward a request to the satellite BMC associated with the
    // prefix.
    // 尝试将请求转发给与该前缀关联的
    // satellite BMC。
    void forwardRequest(
        const crow::Request& thisReq,
        const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
        const std::string& prefix,
        const std::unordered_map<std::string, boost::urls::url>& satelliteInfo)
    {
        const auto& sat = satelliteInfo.find(prefix);
        if (sat == satelliteInfo.end())
        {
            // Realistically this shouldn't get called since we perform an
            // earlier check to make sure the prefix exists
            // 实际上不应调用到这里，因为我们之前已检查
            // 以确保前缀存在
            BMCWEB_LOG_ERROR("Unrecognized satellite prefix \"{}\"", prefix);
            return;
        }

        // We need to strip the prefix from the request's path
        // 我们需要从请求路径中去掉前缀
        boost::urls::url targetURI(thisReq.target());
        std::string path = thisReq.url().path();
        size_t pos = path.find(prefix + "_");
        if (pos == std::string::npos)
        {
            // If this fails then something went wrong
            // 如果失败，则说明出了问题
            BMCWEB_LOG_ERROR("Error removing prefix \"{}_\" from request URI",
                             prefix);
            messages::internalError(asyncResp->res);
            return;
        }
        path.erase(pos, prefix.size() + 1);

        std::function<void(crow::Response&)> cb =
            std::bind_front(processResponse, prefix, asyncResp);

        std::string data = thisReq.body();
        boost::urls::url url(sat->second);
        url.set_path(path);
        if (targetURI.has_query())
        {
            url.set_query(targetURI.query());
        }

        // Prepare request headers
        // 准备请求头
        boost::beast::http::fields requestFields =
            prepareAggregationHeaders(thisReq.fields(), prefix);

        client.sendDataWithCallback(std::move(data), url,
                                    ensuressl::VerifyCertificate::Verify,
                                    requestFields, thisReq.method(), cb);
    }

    // Forward a request for a collection URI to each known satellite BMC
    // 将集合 URI 的请求转发给每个已知的 satellite BMC
    void forwardCollectionRequests(
        const crow::Request& thisReq,
        const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
        const std::unordered_map<std::string, boost::urls::url>& satelliteInfo)
    {
        for (const auto& sat : satelliteInfo)
        {
            std::function<void(crow::Response&)> cb = std::bind_front(
                processCollectionResponse, sat.first, asyncResp);

            boost::urls::url url(sat.second);
            url.set_path(thisReq.url().path());
            if (thisReq.url().has_query())
            {
                url.set_query(thisReq.url().query());
            }
            std::string data = thisReq.body();

            // Prepare request headers
            // 准备请求头
            boost::beast::http::fields requestFields =
                prepareAggregationHeaders(thisReq.fields(), sat.first);

            client.sendDataWithCallback(std::move(data), url,
                                        ensuressl::VerifyCertificate::Verify,
                                        requestFields, thisReq.method(), cb);
        }
    }

    // Forward request for a URI that is uptree of a top level collection to
    // each known satellite BMC
    // 将针对顶层集合上级路径 URI 的请求转发给
    // 每个已知的 satellite BMC
    void forwardContainsSubordinateRequests(
        const crow::Request& thisReq,
        const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
        const std::unordered_map<std::string, boost::urls::url>& satelliteInfo)
    {
        for (const auto& sat : satelliteInfo)
        {
            std::function<void(crow::Response&)> cb = std::bind_front(
                processContainsSubordinateResponse, sat.first, asyncResp);

            // will ignore an expanded resource in the response if that resource
            // is not already supported by the aggregating BMC
            // TODO: Improve the processing so that we don't have to strip query
            // params in this specific case
            // 如果聚合 BMC 尚不支持某个被展开的资源，
            // 则会忽略响应中的该资源
            // TODO: 改进处理逻辑，以便在此特定情况下
            // 无需去掉查询参数
            boost::urls::url url(sat.second);
            url.set_path(thisReq.url().path());

            std::string data = thisReq.body();

            // Prepare request headers
            // 准备请求头
            boost::beast::http::fields requestFields =
                prepareAggregationHeaders(thisReq.fields(), sat.first);

            client.sendDataWithCallback(std::move(data), url,
                                        ensuressl::VerifyCertificate::Verify,
                                        requestFields, thisReq.method(), cb);
        }
    }

  public:
    explicit RedfishAggregator() :
        client(getIoContext(),
               std::make_shared<crow::ConnectionPolicy>(getAggregationPolicy()))
    {
        getSatelliteConfigs(constructorCallback);
    }
    RedfishAggregator(const RedfishAggregator&) = delete;
    RedfishAggregator& operator=(const RedfishAggregator&) = delete;
    RedfishAggregator(RedfishAggregator&&) = delete;
    RedfishAggregator& operator=(RedfishAggregator&&) = delete;
    ~RedfishAggregator() = default;

    static RedfishAggregator& getInstance()
    {
        static RedfishAggregator handler;
        return handler;
    }

    // Aggregation sources with their URLs and optional credentials
    // 聚合源及其 URL 和可选凭据
    std::unordered_map<std::string, AggregationSource> aggregationSources;

    // Helper function to prepare headers for aggregated satellite BMC requests
    // 为发往 satellite BMC 的聚合请求准备头部的辅助函数
    boost::beast::http::fields prepareAggregationHeaders(
        const boost::beast::http::fields& originalFields,
        const std::string& prefix) const
    {
        boost::beast::http::fields fields = originalFields;

        // POST AggregationService can only parse JSON
        // POST AggregationService 只能解析 JSON
        fields.set(boost::beast::http::field::accept, "application/json");

        // Add authentication if credentials exist for this prefix
        // 如果该前缀存在凭据，则添加认证信息
        auto it = aggregationSources.find(prefix);
        if (it != aggregationSources.end())
        {
            const auto& source = it->second;
            // Only add auth header if both username and password are provided
            // 仅在同时提供用户名和密码时才添加认证头
            if (!source.username.empty() && !source.password.empty())
            {
                std::string authHeader = crow::utility::createBasicAuthHeader(
                    source.username, source.password);
                fields.set(boost::beast::http::field::authorization,
                           authHeader);
            }
        }
        return fields;
    }

    // Polls D-Bus to get all available satellite config information
    // Expects a handler which interacts with the returned configs
    // 轮询 D-Bus 以获取所有可用的 satellite 配置信息
    // 需要一个与返回的配置进行交互的处理器
    void getSatelliteConfigs(
        std::function<
            void(const std::unordered_map<std::string, boost::urls::url>&)>
            handler) const
    {
        BMCWEB_LOG_DEBUG("Gathering satellite configs");

        // Extract just the URLs from aggregationSources for the handler
        // 从 aggregationSources 中仅为处理器提取 URL
        std::unordered_map<std::string, boost::urls::url> satelliteInfo;
        for (const auto& [prefix, source] : aggregationSources)
        {
            satelliteInfo.emplace(prefix, source.url);
        }

        sdbusplus::object_path path("/xyz/openbmc_project/inventory");
        dbus::utility::getManagedObjects(
            "xyz.openbmc_project.EntityManager", path,
            [handler{std::move(handler)},
             satelliteInfo = std::move(satelliteInfo)](
                const boost::system::error_code& ec,
                const dbus::utility::ManagedObjectType& objects) mutable {
                if (ec)
                {
                    BMCWEB_LOG_WARNING("DBUS response error {}, {}", ec.value(),
                                       ec.message());
                }
                else
                {
                    // Maps a chosen alias representing a satellite BMC to a url
                    // containing the information required to create a http
                    // connection to the satellite
                    // 将表示某个 satellite BMC 的选定别名映射到一个 url，
                    // 该 url 包含创建到该 satellite 的 http
                    // 连接所需的信息
                    findSatelliteConfigs(objects, satelliteInfo);

                    if (!satelliteInfo.empty())
                    {
                        BMCWEB_LOG_DEBUG(
                            "Redfish Aggregation enabled with {} satellite BMCs",
                            std::to_string(satelliteInfo.size()));
                    }
                    else
                    {
                        BMCWEB_LOG_DEBUG(
                            "Redfish aggregation enabled, but no satellite BMCs detected");
                    }
                }
                handler(satelliteInfo);
            });
    }

    // Processes the response returned by a satellite BMC and loads its
    // contents into asyncResp
    // 处理 satellite BMC 返回的响应，并将其内容
    // 加载到 asyncResp 中
    static void processResponse(
        std::string_view prefix,
        const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
        crow::Response& resp)
    {
        // 429 and 502 mean we didn't actually send the request so don't
        // overwrite the response headers in that case
        // 429 和 502 表示请求实际上并未发出，因此这种情况下
        // 不要覆盖响应头
        if ((resp.result() == boost::beast::http::status::too_many_requests) ||
            (resp.result() == boost::beast::http::status::bad_gateway))
        {
            asyncResp->res.result(resp.result());
            return;
        }

        // We want to attempt prefix fixing regardless of response code
        // The resp will not have a json component
        // We need to create a json from resp's stringResponse
        // 无论响应码如何，我们都要尝试修复前缀
        // resp 不包含 json 部分
        // 需要从 resp 的 stringResponse 创建 json
        if (isJsonContentType(resp.getHeaderValue("Content-Type")))
        {
            std::optional<nlohmann::json> jsonVal =
                parseStringAsJson(*resp.body());
            if (!jsonVal)
            {
                BMCWEB_LOG_ERROR("Error parsing satellite response as JSON");
                messages::operationFailed(asyncResp->res);
                return;
            }

            BMCWEB_LOG_DEBUG("Successfully parsed satellite response");

            addPrefixes(*jsonVal, prefix);

            BMCWEB_LOG_DEBUG("Added prefix to parsed satellite response");

            asyncResp->res.result(resp.result());
            asyncResp->res.jsonValue = std::move(*jsonVal);

            BMCWEB_LOG_DEBUG("Finished writing asyncResp");
        }
        else
        {
            // We allow any Content-Type that is not "application/json" now
            // 现在允许任何非 "application/json" 的 Content-Type
            asyncResp->res.result(resp.result());
            asyncResp->res.copyBody(resp);
        }
        addAggregatedHeaders(asyncResp->res, resp, prefix);
    }

    // Processes the collection response returned by a satellite BMC and merges
    // its "@odata.id" values
    // 处理 satellite BMC 返回的集合响应，并合并其
    // "@odata.id" 值
    static void processCollectionResponse(
        const std::string& prefix,
        const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
        crow::Response& resp)
    {
        // 429 and 502 mean we didn't actually send the request so don't
        // overwrite the response headers in that case
        // 429 和 502 表示请求实际上并未发出，因此这种情况下
        // 不要覆盖响应头
        if ((resp.result() == boost::beast::http::status::too_many_requests) ||
            (resp.result() == boost::beast::http::status::bad_gateway))
        {
            return;
        }

        if (resp.resultInt() != 200)
        {
            BMCWEB_LOG_DEBUG(
                "Collection resource does not exist in satellite BMC \"{}\"",
                prefix);
            // Return the error if we haven't had any successes
            // 如果没有任何成功，则返回错误
            if (asyncResp->res.resultInt() != 200)
            {
                asyncResp->res.result(resp.result());
                asyncResp->res.copyBody(resp);
            }
            return;
        }

        // The resp will not have a json component
        // We need to create a json from resp's stringResponse
        // resp 不包含 json 部分
        // 需要从 resp 的 stringResponse 创建 json
        if (isJsonContentType(resp.getHeaderValue("Content-Type")))
        {
            std::optional<nlohmann::json> jsonVal =
                parseStringAsJson(*resp.body());
            if (!jsonVal)
            {
                BMCWEB_LOG_ERROR("Error parsing satellite response as JSON");

                // Notify the user if doing so won't overwrite a valid response
                // 如果这样做不会覆盖有效响应，则通知用户
                if (asyncResp->res.resultInt() != 200)
                {
                    messages::operationFailed(asyncResp->res);
                }
                return;
            }

            BMCWEB_LOG_DEBUG("Successfully parsed satellite response");

            // Now we need to add the prefix to the URIs contained in the
            // response.
            // 现在需要给响应中包含的 URI
            // 加上前缀。
            addPrefixes(*jsonVal, prefix);

            BMCWEB_LOG_DEBUG("Added prefix to parsed satellite response");

            // If this resource collection does not exist on the aggregating bmc
            // and has not already been added from processing the response from
            // a different satellite then we need to completely overwrite
            // asyncResp
            // 如果聚合 bmc 上不存在该资源集合，
            // 且尚未通过处理来自其他 satellite 的响应
            // 将其加入，则需要完全覆盖
            // asyncResp
            if (asyncResp->res.resultInt() != 200)
            {
                // We only want to aggregate collections that contain a
                // "Members" array
                // 我们只想聚合包含
                // "Members" 数组的集合
                nlohmann::json::iterator membersIt = jsonVal->find("Members");
                if (membersIt == jsonVal->end() || !membersIt->is_array())
                {
                    BMCWEB_LOG_DEBUG(
                        "Skipping aggregating unsupported resource");
                    return;
                }

                BMCWEB_LOG_DEBUG(
                    "Collection does not exist, overwriting asyncResp");
                asyncResp->res.result(resp.result());
                asyncResp->res.jsonValue = std::move(*jsonVal);
                asyncResp->res.addHeader("Content-Type", "application/json");

                BMCWEB_LOG_DEBUG("Finished overwriting asyncResp");
            }
            else
            {
                // We only want to aggregate collections that contain a
                // "Members" array
                // 我们只想聚合包含
                // "Members" 数组的集合
                nlohmann::json::iterator membersIt =
                    asyncResp->res.jsonValue.find("Members");
                if (membersIt == asyncResp->res.jsonValue.end() ||
                    !membersIt->is_array())
                {
                    BMCWEB_LOG_DEBUG(
                        "Skipping aggregating unsupported resource");
                    return;
                }

                BMCWEB_LOG_DEBUG(
                    "Adding aggregated resources from \"{}\" to collection",
                    prefix);

                // TODO: This is a potential race condition with multiple
                // satellites and the aggregating bmc attempting to write to
                // update this array.  May need to cascade calls to the next
                // satellite at the end of this function.
                // This is presumably not a concern when there is only a single
                // satellite since the aggregating bmc should have completed
                // before the response is received from the satellite.
                // TODO: 多个卫星 BMC 与聚合 BMC 同时尝试写入
                // 更新此数组时，可能存在竞态条件。
                // 可能需要在本函数末尾将调用级联到
                // 下一个卫星 BMC。
                // 当只有一个卫星时这应该不成问题，
                // 因为在收到卫星的响应之前，聚合 BMC
                // 应该已经完成了处理。

                nlohmann::json::array_t& members =
                    collection_util::getJsonArray(asyncResp->res.jsonValue,
                                                  "Members");
                auto& satMembers = (*jsonVal)["Members"];
                nlohmann::json::array_t* satMembersArr =
                    satMembers.get_ptr<nlohmann::json::array_t*>();
                if (satMembersArr == nullptr)
                {
                    BMCWEB_LOG_WARNING(
                        "Sat collection didn't include a members array");
                    return;
                }
                for (auto& satMem : *satMembersArr)
                {
                    members.emplace_back(std::move(satMem));
                }
                json_util::sortJsonArrayByOData(members);
                asyncResp->res.jsonValue["Members@odata.count"] =
                    members.size();
            }
        }
        else
        {
            BMCWEB_LOG_ERROR("Received unparsable response from \"{}\"",
                             prefix);
            // We received a response that was not a json.
            // Notify the user only if we did not receive any valid responses
            // and if the resource collection does not already exist on the
            // aggregating BMC
            // 我们收到的响应不是 JSON。
            // 仅当未收到任何有效响应，且聚合 BMC 上
            // 尚不存在该资源集合时，
            // 才向用户通知。
            if (asyncResp->res.resultInt() != 200)
            {
                messages::operationFailed(asyncResp->res);
            }
        }
    } // End processCollectionResponse()

    // Processes the response returned by a satellite BMC and merges any
    // properties whose "@odata.id" value is the URI or either a top level
    // collection or is uptree from a top level collection
    // 处理卫星 BMC 返回的响应，并合并那些
    // "@odata.id" 值是顶层集合的 URI，或位于顶层集合上层
    // 路径的属性。
    static void processContainsSubordinateResponse(
        const std::string& prefix,
        const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
        crow::Response& resp)
    {
        // 429 and 502 mean we didn't actually send the request so don't
        // overwrite the response headers in that case
        // 429 和 502 表示请求实际上并未发出，
        // 因此这种情况下不要覆盖响应头。
        if ((resp.result() == boost::beast::http::status::too_many_requests) ||
            (resp.result() == boost::beast::http::status::bad_gateway))
        {
            return;
        }

        if (resp.resultInt() != 200)
        {
            BMCWEB_LOG_DEBUG(
                "Resource uptree from Collection does not exist in satellite BMC \"{}\"",
                prefix);
            // Return the error if we haven't had any successes
            // 如果此前一次都没有成功，则返回错误。
            if (asyncResp->res.resultInt() != 200)
            {
                asyncResp->res.result(resp.result());
                asyncResp->res.copyBody(resp);
            }
            return;
        }

        // The resp will not have a json component
        // We need to create a json from resp's stringResponse
        // resp 中不会有 JSON 部分，
        // 我们需要从 resp 的 stringResponse 创建 JSON。
        if (isJsonContentType(resp.getHeaderValue("Content-Type")))
        {
            bool addedLinks = false;
            std::optional<nlohmann::json> jsonVal =
                parseStringAsJson(*resp.body());
            if (!jsonVal)
            {
                BMCWEB_LOG_ERROR("Error parsing satellite response as JSON");

                // Notify the user if doing so won't overwrite a valid response
                // 如果这样做不会覆盖有效响应，则通知用户。
                if (asyncResp->res.resultInt() != 200)
                {
                    messages::operationFailed(asyncResp->res);
                }
                return;
            }

            BMCWEB_LOG_DEBUG("Successfully parsed satellite response");

            // Parse response and add properties missing from the AsyncResp
            // Valid properties will be of the form <property>.@odata.id and
            // @odata.id is a <URI>.  In other words, the json should contain
            // multiple properties such that
            // {"<property>":{"@odata.id": "<URI>"}}
            // 解析响应，并添加 AsyncResp 中缺失的属性。
            // 有效属性的形式为 <property>.@odata.id，且
            // @odata.id 是一个 <URI>。换句话说，JSON 应包含
            // 多个属性，形如
            // {"<property>":{"@odata.id": "<URI>"}}
            nlohmann::json::object_t* object =
                jsonVal->get_ptr<nlohmann::json::object_t*>();
            if (object == nullptr)
            {
                BMCWEB_LOG_ERROR("Parsed JSON was not an object?");
                return;
            }

            for (std::pair<const std::string, nlohmann::json>& prop : *object)
            {
                if (!prop.second.contains("@odata.id"))
                {
                    continue;
                }

                std::string* strValue =
                    prop.second["@odata.id"].get_ptr<std::string*>();
                if (strValue == nullptr)
                {
                    BMCWEB_LOG_CRITICAL("Field wasn't a string????");
                    continue;
                }
                if (!searchCollectionsArray(*strValue, SearchType::CollOrCon))
                {
                    continue;
                }

                addedLinks = true;
                if (!asyncResp->res.jsonValue.contains(prop.first))
                {
                    // Only add the property if it did not already exist
                    // 仅当该属性尚不存在时才添加。
                    BMCWEB_LOG_DEBUG("Adding link for {} from BMC {}",
                                     *strValue, prefix);
                    asyncResp->res.jsonValue[prop.first]["@odata.id"] =
                        *strValue;
                    continue;
                }
            }

            // If we added links to a previously unsuccessful (non-200) response
            // then we need to make sure the response contains the bare minimum
            // amount of additional information that we'd expect to have been
            // populated.
            // 如果我们向先前不成功（非 200）的响应添加了链接，
            // 那么需要确保该响应包含最少量的、
            // 我们本应期望已被填充的
            // 附加信息。
            if (addedLinks && (asyncResp->res.resultInt() != 200))
            {
                // This resource didn't locally exist or an error
                // occurred while generating the response.  Remove any
                // error messages and update the error code.
                // 该资源在本地不存在，或生成响应时
                // 发生了错误。移除所有错误信息，
                // 并更新错误码。
                asyncResp->res.jsonValue.erase(
                    asyncResp->res.jsonValue.find("error"));
                asyncResp->res.result(resp.result());

                const auto& it1 = object->find("@odata.id");
                if (it1 != object->end())
                {
                    asyncResp->res.jsonValue["@odata.id"] = (it1->second);
                }
                const auto& it2 = object->find("@odata.type");
                if (it2 != object->end())
                {
                    asyncResp->res.jsonValue["@odata.type"] = (it2->second);
                }
                const auto& it3 = object->find("Id");
                if (it3 != object->end())
                {
                    asyncResp->res.jsonValue["Id"] = (it3->second);
                }
                const auto& it4 = object->find("Name");
                if (it4 != object->end())
                {
                    asyncResp->res.jsonValue["Name"] = (it4->second);
                }
            }
        }
        else
        {
            BMCWEB_LOG_ERROR("Received unparsable response from \"{}\"",
                             prefix);
            // We received as response that was not a json
            // Notify the user only if we did not receive any valid responses,
            // and if the resource does not already exist on the aggregating BMC
            // 我们收到的响应不是 JSON。
            // 仅当未收到任何有效响应，且聚合 BMC 上
            // 尚不存在该资源时，才通知用户。
            if (asyncResp->res.resultInt() != 200)
            {
                messages::operationFailed(asyncResp->res);
            }
        }
    }

    // Entry point to Redfish Aggregation
    // Returns Result stating whether or not we still need to locally handle the
    // request
    // Redfish 聚合的入口。
    // 返回 Result，说明我们是否仍需要在本地处理该
    // 请求。
    Result beginAggregation(const crow::Request& thisReq,
                            const std::shared_ptr<bmcweb::AsyncResp>& asyncResp)
    {
        using crow::utility::OrMorePaths;
        using crow::utility::readUrlSegments;
        boost::urls::url_view url = thisReq.url();

        // We don't need to aggregate JsonSchemas due to potential issues such
        // as version mismatches between aggregator and satellite BMCs.  For
        // now assume that the aggregator has all the schemas and versions that
        // the aggregated server has.
        // 我们不需要聚合 JsonSchemas，因为可能存在聚合 BMC 与
        // 卫星 BMC 之间版本不匹配等问题。
        // 目前假设聚合器拥有被聚合服务器所具备的
        // 所有 schema 和版本。
        if (crow::utility::readUrlSegments(url, "redfish", "v1", "JsonSchemas",
                                           crow::utility::OrMorePaths()))
        {
            return Result::LocalHandle;
        }

        // The first two segments should be "/redfish/v1".  We need to check
        // that before we can search topCollections
        // 前两个路径段应为 "/redfish/v1"。在搜索 topCollections 之前
        // 需要先检查这一点。
        if (!crow::utility::readUrlSegments(url, "redfish", "v1",
                                            crow::utility::OrMorePaths()))
        {
            return Result::LocalHandle;
        }

        // Parse the URI to see if it begins with a known top level collection
        // such as:
        // /redfish/v1/Chassis
        // /redfish/v1/UpdateService/FirmwareInventory
        // 解析 URI，查看它是否以已知的顶层集合开头，
        // 例如：
        // /redfish/v1/Chassis
        // /redfish/v1/UpdateService/FirmwareInventory
        const boost::urls::segments_view urlSegments = url.segments();
        boost::urls::url currentUrl("/");
        boost::urls::segments_view::const_iterator it = urlSegments.begin();
        boost::urls::segments_view::const_iterator end = urlSegments.end();

        // Skip past the leading "/redfish/v1"
        // 跳过开头的 "/redfish/v1"。
        it++;
        it++;
        for (; it != end; it++)
        {
            const std::string& collectionItem = *it;
            if (std::ranges::binary_search(
                    topCollections, std::string_view(currentUrl.buffer())))
            {
                // We've matched a resource collection so this current segment
                // might contain an aggregation prefix
                // 我们已经匹配到一个资源集合，因此当前路径段
                // 可能包含聚合前缀。
                if (segmentHasPrefix(collectionItem))
                {
                    BMCWEB_LOG_DEBUG("Need to forward a request");

                    // Extract the prefix from the request's URI, retrieve the
                    // associated satellite config information, and then forward
                    // the request to that satellite.
                    // 从请求的 URI 中提取前缀，获取对应的
                    // 卫星配置信息，然后将请求转发
                    // 给该卫星。
                    startAggregation(AggregationType::Resource, thisReq,
                                     asyncResp);
                    return Result::NoLocalHandle;
                }

                // Handle collection URI with a trailing backslash
                // e.g. /redfish/v1/Chassis/
                // 处理以反斜杠结尾的集合 URI，
                // 例如 /redfish/v1/Chassis/。
                it++;
                if ((it == end) && collectionItem.empty())
                {
                    startAggregation(AggregationType::Collection, thisReq,
                                     asyncResp);
                }

                // We didn't recognize the prefix or it's a collection with a
                // trailing "/".  In both cases we still want to locally handle
                // the request
                // 我们无法识别该前缀，或这是一个以 "/" 结尾的集合。
                // 这两种情况下，我们仍然希望在本地
                // 处理该请求。
                return Result::LocalHandle;
            }

            currentUrl.segments().push_back(collectionItem);
        }

        // If we made it here then currentUrl could contain a top level
        // collection URI without a trailing "/", e.g. /redfish/v1/Chassis
        // 如果执行到这里，currentUrl 可能包含不带结尾 "/" 的
        // 顶层集合 URI，例如 /redfish/v1/Chassis。
        if (std::ranges::binary_search(topCollections,
                                       std::string_view(currentUrl.buffer())))
        {
            startAggregation(AggregationType::Collection, thisReq, asyncResp);
            return Result::LocalHandle;
        }

        // If nothing else then the request could be for a resource which has a
        // top level collection as a subordinate
        // 如果都不是，则该请求可能是针对某个以顶层集合为
        // 下级（subordinate）的资源。
        if (searchCollectionsArray(url.path(), SearchType::ContainsSubordinate))
        {
            startAggregation(AggregationType::ContainsSubordinate, thisReq,
                             asyncResp);
            return Result::LocalHandle;
        }

        BMCWEB_LOG_DEBUG("Aggregation not required for {}", url.buffer());
        return Result::LocalHandle;
    }

    // Check if the given URL segment matches with any satellite prefix
    // Assumes the given segment starts with <prefix>_
    // 检查给定的 URL 路径段是否匹配某个卫星前缀。
    // 假设该路径段以 <prefix>_ 开头。
    bool segmentHasPrefix(const std::string& urlSegment) const
    {
        // TODO: handle this better
        // For now 5B247A_ won't be in the aggregationSources map so
        // check explicitly for now
        // TODO: 需要更好地处理这种情况。
        // 目前 5B247A_ 不在 aggregationSources 映射中，
        // 所以先做显式检查。
        if (urlSegment.starts_with("5B247A_"))
        {
            return true;
        }

        // Find the first underscore
        // 查找第一个下划线。
        std::size_t underscorePos = urlSegment.find('_');
        if (underscorePos == std::string::npos)
        {
            return false; // No underscore, can't be a satellite prefix
        }

        // Extract the prefix
        // 提取前缀。
        std::string prefix = urlSegment.substr(0, underscorePos);

        // Check if this prefix exists
        // 检查该前缀是否存在。
        return aggregationSources.contains(prefix);
    }
};

} // namespace redfish
