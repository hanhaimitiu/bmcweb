// SPDX-License-Identifier: Apache-2.0
// SPDX-FileCopyrightText: Copyright OpenBMC Authors
#pragma once

#include "bmcweb_config.h"

#include "app.hpp"
#include "async_resp.hpp"
#include "error_messages.hpp"
#include "generated/enums/log_entry.hpp"
#include "gzfile.hpp"
#include "http_request.hpp"
#include "human_sort.hpp"
#include "logging.hpp"
#include "query.hpp"
#include "registries/privilege_registry.hpp"
#include "utils/query_param.hpp"

#include <boost/beast/http/verb.hpp>
#include <boost/url/format.hpp>
#include <nlohmann/json.hpp>

#include <algorithm>
#include <charconv>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <format>
#include <functional>
#include <memory>
#include <ranges>
#include <string>
#include <string_view>
#include <system_error>
#include <utility>
#include <vector>

namespace redfish
{
constexpr const char* hostLoggerFolderPath = "/var/log/console";

inline bool getHostLoggerFiles(
    const std::string& hostLoggerFilePath,
    std::vector<std::filesystem::path>& hostLoggerFiles)
{
    std::error_code ec;
    std::filesystem::directory_iterator logPath(hostLoggerFilePath, ec);
    if (ec)
    {
        BMCWEB_LOG_WARNING("{}", ec.message());
        return false;
    }
    for (const std::filesystem::directory_entry& it : logPath)
    {
        std::string filename = it.path().filename();
        // Prefix of each log files is "log". Find the file and save the
        // path
        // 每个日志文件的前缀是 "log"。找到该文件并保存其
        // 路径
        if (filename.starts_with("log"))
        {
            hostLoggerFiles.emplace_back(it.path());
        }
    }
    // As the log files rotate, they are appended with a ".#" that is higher for
    // the older logs. Since we start from oldest logs, sort the name in
    // descending order.
    // 随着日志文件轮转，它们会被追加一个 ".#"，日志越旧该数字
    // 越大。由于我们从最旧的日志开始，因此按降序对名称排序。
    std::ranges::sort(std::ranges::reverse_view(hostLoggerFiles),
                      AlphanumLess<std::string>());

    return true;
}

inline bool getHostLoggerEntries(
    const std::vector<std::filesystem::path>& hostLoggerFiles, uint64_t skip,
    uint64_t top, std::vector<std::string>& logEntries, size_t& logCount)
{
    GzFileReader logFile;

    // Go though all log files and expose host logs.
    // 遍历所有日志文件并公开主机日志。
    for (const std::filesystem::path& it : hostLoggerFiles)
    {
        if (!logFile.gzGetLines(it.string(), skip, top, logEntries, logCount))
        {
            BMCWEB_LOG_ERROR("fail to expose host logs");
            return false;
        }
    }
    // Get lastMessage from constructor by getter
    // 通过 getter 从构造函数中获取 lastMessage
    std::string lastMessage = logFile.getLastMessage();
    if (!lastMessage.empty())
    {
        logCount++;
        if (logCount > skip && logCount <= (skip + top))
        {
            logEntries.push_back(lastMessage);
        }
    }
    return true;
}

inline void fillHostLoggerEntryJson(std::string_view logEntryID,
                                    std::string_view msg,
                                    nlohmann::json::object_t& logEntryJson)
{
    // Fill in the log entry with the gathered data.
    // 用收集到的数据填充日志条目。
    logEntryJson["@odata.type"] = "#LogEntry.v1_9_0.LogEntry";
    logEntryJson["@odata.id"] = boost::urls::format(
        "/redfish/v1/Systems/{}/LogServices/HostLogger/Entries/{}",
        BMCWEB_REDFISH_SYSTEM_URI_NAME, logEntryID);
    logEntryJson["Name"] = "Host Logger Entry";
    logEntryJson["Id"] = logEntryID;
    logEntryJson["Message"] = msg;
    logEntryJson["EntryType"] = log_entry::LogEntryType::Oem;
    logEntryJson["Severity"] = log_entry::EventSeverity::OK;
    logEntryJson["OemRecordFormat"] = "Host Logger Entry";
}

inline void handleSystemsLogServicesHostloggerGet(
    App& app, const crow::Request& req,
    const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
    const std::string& systemName)
{
    if (!redfish::setUpRedfishRoute(app, req, asyncResp))
    {
        return;
    }
    if constexpr (BMCWEB_EXPERIMENTAL_REDFISH_MULTI_COMPUTER_SYSTEM)
    {
        // Option currently returns no systems.  TBD
        // 该选项目前不返回任何系统。TBD
        messages::resourceNotFound(asyncResp->res, "ComputerSystem",
                                   systemName);
        return;
    }
    if (systemName != BMCWEB_REDFISH_SYSTEM_URI_NAME)
    {
        messages::resourceNotFound(asyncResp->res, "ComputerSystem",
                                   systemName);
        return;
    }
    asyncResp->res.jsonValue["@odata.id"] =
        boost::urls::format("/redfish/v1/Systems/{}/LogServices/HostLogger",
                            BMCWEB_REDFISH_SYSTEM_URI_NAME);
    asyncResp->res.jsonValue["@odata.type"] = "#LogService.v1_2_0.LogService";
    asyncResp->res.jsonValue["Name"] = "Host Logger Service";
    asyncResp->res.jsonValue["Description"] = "Host Logger Service";
    asyncResp->res.jsonValue["Id"] = "HostLogger";
    asyncResp->res.jsonValue["Entries"]["@odata.id"] = boost::urls::format(
        "/redfish/v1/Systems/{}/LogServices/HostLogger/Entries",
        BMCWEB_REDFISH_SYSTEM_URI_NAME);
}

inline void handleSystemsLogServicesHostloggerEntriesGet(
    App& app, const crow::Request& req,
    const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
    const std::string& systemName)
{
    query_param::QueryCapabilities capabilities = {
        .canDelegateTop = true,
        .canDelegateSkip = true,
    };
    query_param::Query delegatedQuery;
    if (!redfish::setUpRedfishRouteWithDelegation(app, req, asyncResp,
                                                  delegatedQuery, capabilities))
    {
        return;
    }
    if constexpr (BMCWEB_EXPERIMENTAL_REDFISH_MULTI_COMPUTER_SYSTEM)
    {
        // Option currently returns no systems.  TBD
        // 该选项目前不返回任何系统。TBD
        messages::resourceNotFound(asyncResp->res, "ComputerSystem",
                                   systemName);
        return;
    }
    if (systemName != BMCWEB_REDFISH_SYSTEM_URI_NAME)
    {
        messages::resourceNotFound(asyncResp->res, "ComputerSystem",
                                   systemName);
        return;
    }
    asyncResp->res.jsonValue["@odata.id"] = boost::urls::format(
        "/redfish/v1/Systems/{}/LogServices/HostLogger/Entries",
        BMCWEB_REDFISH_SYSTEM_URI_NAME);
    asyncResp->res.jsonValue["@odata.type"] =
        "#LogEntryCollection.LogEntryCollection";
    asyncResp->res.jsonValue["Name"] = "HostLogger Entries";
    asyncResp->res.jsonValue["Description"] =
        "Collection of HostLogger Entries";
    nlohmann::json& logEntryArray = asyncResp->res.jsonValue["Members"];
    logEntryArray = nlohmann::json::array();
    asyncResp->res.jsonValue["Members@odata.count"] = 0;

    std::vector<std::filesystem::path> hostLoggerFiles;
    if (!getHostLoggerFiles(hostLoggerFolderPath, hostLoggerFiles))
    {
        BMCWEB_LOG_DEBUG("Failed to get host log file path");
        return;
    }
    // If we weren't provided top and skip limits, use the defaults.
    // 如果未提供 top 和 skip 限制，则使用默认值。
    size_t skip = delegatedQuery.skip.value_or(0);
    size_t top = delegatedQuery.top.value_or(query_param::Query::maxTop);
    size_t logCount = 0;
    // This vector only store the entries we want to expose that
    // control by skip and top.
    // 这个 vector 只存储我们想公开的条目，
    // 由 skip 和 top 控制。
    std::vector<std::string> logEntries;
    if (!getHostLoggerEntries(hostLoggerFiles, skip, top, logEntries, logCount))
    {
        messages::internalError(asyncResp->res);
        return;
    }
    // If vector is empty, that means skip value larger than total
    // log count
    // 如果 vector 为空，说明 skip 值大于
    // 日志总数
    if (logEntries.empty())
    {
        asyncResp->res.jsonValue["Members@odata.count"] = logCount;
        return;
    }
    if (!logEntries.empty())
    {
        for (size_t i = 0; i < logEntries.size(); i++)
        {
            nlohmann::json::object_t hostLogEntry;
            fillHostLoggerEntryJson(std::to_string(skip + i), logEntries[i],
                                    hostLogEntry);
            logEntryArray.emplace_back(std::move(hostLogEntry));
        }

        asyncResp->res.jsonValue["Members@odata.count"] = logCount;
        if (skip + top < logCount)
        {
            asyncResp->res.jsonValue["Members@odata.nextLink"] =
                std::format(
                    "/redfish/v1/Systems/{}/LogServices/HostLogger/Entries?$skip=",
                    BMCWEB_REDFISH_SYSTEM_URI_NAME) +
                std::to_string(skip + top);
        }
    }
}

inline void handleSystemsLogServicesHostloggerEntriesEntryGet(
    App& app, const crow::Request& req,
    const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
    const std::string& systemName, const std::string& param)
{
    if (!redfish::setUpRedfishRoute(app, req, asyncResp))
    {
        return;
    }
    if constexpr (BMCWEB_EXPERIMENTAL_REDFISH_MULTI_COMPUTER_SYSTEM)
    {
        // Option currently returns no systems.  TBD
        // 该选项目前不返回任何系统。TBD
        messages::resourceNotFound(asyncResp->res, "ComputerSystem",
                                   systemName);
        return;
    }
    if (systemName != BMCWEB_REDFISH_SYSTEM_URI_NAME)
    {
        messages::resourceNotFound(asyncResp->res, "ComputerSystem",
                                   systemName);
        return;
    }
    std::string_view targetID = param;

    uint64_t idInt = 0;

    auto [ptr, ec] = std::from_chars(targetID.begin(), targetID.end(), idInt);
    if (ec != std::errc{} || ptr != targetID.end())
    {
        messages::resourceNotFound(asyncResp->res, "LogEntry", param);
        return;
    }

    std::vector<std::filesystem::path> hostLoggerFiles;
    if (!getHostLoggerFiles(hostLoggerFolderPath, hostLoggerFiles))
    {
        BMCWEB_LOG_DEBUG("Failed to get host log file path");
        return;
    }

    size_t logCount = 0;
    size_t top = 1;
    std::vector<std::string> logEntries;
    // We can get specific entry by skip and top. For example, if we
    // want to get nth entry, we can set skip = n-1 and top = 1 to
    // get that entry
    // 我们可以通过 skip 和 top 获取特定条目。例如，如果
    // 我们想获取第 n 条，可以设置 skip = n-1 且 top = 1 来
    // 获取该条目
    if (!getHostLoggerEntries(hostLoggerFiles, idInt, top, logEntries,
                              logCount))
    {
        messages::internalError(asyncResp->res);
        return;
    }

    if (!logEntries.empty())
    {
        nlohmann::json::object_t hostLogEntry;
        fillHostLoggerEntryJson(targetID, logEntries[0], hostLogEntry);
        asyncResp->res.jsonValue.update(hostLogEntry);
        return;
    }

    // Requested ID was not found
    // 未找到请求的 ID
    messages::resourceNotFound(asyncResp->res, "LogEntry", param);
}

inline void requestRoutesSystemsLogServiceHostlogger(App& app)
{
    BMCWEB_ROUTE(app, "/redfish/v1/Systems/<str>/LogServices/HostLogger/")
        .privileges(redfish::privileges::getLogService)
        .methods(boost::beast::http::verb::get)(std::bind_front(
            handleSystemsLogServicesHostloggerGet, std::ref(app)));
    BMCWEB_ROUTE(app,
                 "/redfish/v1/Systems/<str>/LogServices/HostLogger/Entries/")
        .privileges(redfish::privileges::getLogEntryCollection)
        .methods(boost::beast::http::verb::get)(std::bind_front(
            handleSystemsLogServicesHostloggerEntriesGet, std::ref(app)));

    BMCWEB_ROUTE(
        app, "/redfish/v1/Systems/<str>/LogServices/HostLogger/Entries/<str>/")
        .privileges(redfish::privileges::getLogEntry)
        .methods(boost::beast::http::verb::get)(std::bind_front(
            handleSystemsLogServicesHostloggerEntriesEntryGet, std::ref(app)));
}

} // namespace redfish
