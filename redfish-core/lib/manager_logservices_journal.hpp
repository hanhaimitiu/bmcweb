// SPDX-License-Identifier: Apache-2.0
// SPDX-FileCopyrightText: Copyright OpenBMC Authors
#pragma once

#include "bmcweb_config.h"

#include "app.hpp"
#include "async_resp.hpp"
#include "dbus_singleton.hpp"
#include "error_messages.hpp"
#include "generated/enums/log_service.hpp"
#include "http_request.hpp"
#include "logging.hpp"
#include "query.hpp"
#include "registries/privilege_registry.hpp"
#include "utility.hpp"
#include "utils/etag_utils.hpp"
#include "utils/journal_utils.hpp"
#include "utils/query_param.hpp"
#include "utils/time_utils.hpp"

#include <boost/asio/post.hpp>
#include <boost/beast/http/verb.hpp>
#include <boost/url/format.hpp>

#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <utility>

namespace redfish
{

inline void handleManagersLogServiceJournalGet(
    App& app, const crow::Request& req,
    const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
    const std::string& managerId)
{
    if (!redfish::setUpRedfishRoute(app, req, asyncResp))
    {
        return;
    }

    if (managerId != BMCWEB_REDFISH_MANAGER_URI_NAME)
    {
        messages::resourceNotFound(asyncResp->res, "Manager", managerId);
        return;
    }

    asyncResp->res.jsonValue["@odata.type"] = "#LogService.v1_2_0.LogService";
    asyncResp->res.jsonValue["@odata.id"] =
        boost::urls::format("/redfish/v1/Managers/{}/LogServices/Journal",
                            BMCWEB_REDFISH_MANAGER_URI_NAME);
    asyncResp->res.jsonValue["Name"] = "Open BMC Journal Log Service";
    asyncResp->res.jsonValue["Description"] = "BMC Journal Log Service";
    asyncResp->res.jsonValue["Id"] = "Journal";
    asyncResp->res.jsonValue["OverWritePolicy"] =
        log_service::OverWritePolicy::WrapsWhenFull;

    std::pair<std::string, std::string> redfishDateTimeOffset =
        redfish::time_utils::getDateTimeOffsetNow();
    asyncResp->res.jsonValue["DateTime"] = redfishDateTimeOffset.first;
    asyncResp->res.jsonValue["DateTimeLocalOffset"] =
        redfishDateTimeOffset.second;

    asyncResp->res.jsonValue["Entries"]["@odata.id"] = boost::urls::format(
        "/redfish/v1/Managers/{}/LogServices/Journal/Entries",
        BMCWEB_REDFISH_MANAGER_URI_NAME);

    etag_utils::setEtagOmitDateTimeHandler(asyncResp);
}

inline void readJournalEntries(
    uint64_t topEntryCount, const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
    JournalReadState&& readState)
{
    nlohmann::json& logEntry = asyncResp->res.jsonValue["Members"];
    nlohmann::json::array_t* logEntryArray =
        logEntry.get_ptr<nlohmann::json::array_t*>();
    if (logEntryArray == nullptr)
    {
        messages::internalError(asyncResp->res);
        return;
    }

    // The Journal APIs unfortunately do blocking calls to the filesystem, and
    // can be somewhat expensive.  Short of creating our own io_uring based
    // implementation of sd-journal, which would be difficult, the best thing we
    // can do is to only parse a certain number of entries at a time.  The
    // current chunk size is selected arbitrarily to ensure that we're not
    // trying to process thousands of entries at the same time.
    // The implementation will process the number of entries, then return
    // control to the io_context to let other operations continue.
    // Journal API 不幸地对文件系统执行阻塞调用，
    // 可能相当耗时。除非我们自己实现一个基于 io_uring 的
    // sd-journal（这很困难），否则我们能做的最好办法就是
    // 每次只解析一定数量的条目。
    // 当前的分块大小是任意选定的，以确保我们不会
    // 试图同时处理数千个条目。
    // 该实现将处理指定数量的条目，然后把控制权交还给
    // io_context，让其他操作得以继续。
    size_t segmentCountRemaining = 10;

    // Reset the unique ID on the first entry
    // 在第一个条目处重置唯一 ID。
    for (uint64_t entryCount = logEntryArray->size();
         entryCount < topEntryCount; entryCount++)
    {
        if (segmentCountRemaining == 0)
        {
            boost::asio::post(crow::connections::systemBus->get_io_context(),
                              [asyncResp, topEntryCount,
                               readState = std::move(readState)]() mutable {
                                  readJournalEntries(topEntryCount, asyncResp,
                                                     std::move(readState));
                              });
            return;
        }

        nlohmann::json::object_t bmcJournalLogEntry;
        if (!fillBMCJournalLogEntryJson(readState, bmcJournalLogEntry))
        {
            messages::internalError(asyncResp->res);
            return;
        }
        logEntryArray->emplace_back(std::move(bmcJournalLogEntry));

        int ret = readState.next();
        if (ret < 0)
        {
            messages::internalError(asyncResp->res);
            return;
        }
        if (ret == 0)
        {
            break;
        }
        segmentCountRemaining--;
    }
}

inline void handleManagersJournalLogEntryCollectionGet(
    App& app, const crow::Request& req,
    const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
    const std::string& managerId)
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

    if (managerId != BMCWEB_REDFISH_MANAGER_URI_NAME)
    {
        messages::resourceNotFound(asyncResp->res, "Manager", managerId);
        return;
    }

    size_t skip = delegatedQuery.skip.value_or(0);
    size_t top = delegatedQuery.top.value_or(query_param::Query::maxTop);

    // Collections don't include the static data added by SubRoute
    // because it has a duplicate entry for members
    // 集合不包含 SubRoute 添加的静态数据，因为其 members 存在重复条目。
    asyncResp->res.jsonValue["@odata.type"] =
        "#LogEntryCollection.LogEntryCollection";
    asyncResp->res.jsonValue["@odata.id"] = boost::urls::format(
        "/redfish/v1/Managers/{}/LogServices/Journal/Entries",
        BMCWEB_REDFISH_MANAGER_URI_NAME);
    asyncResp->res.jsonValue["Name"] = "Open BMC Journal Entries";
    asyncResp->res.jsonValue["Description"] =
        "Collection of BMC Journal Entries";
    asyncResp->res.jsonValue["Members"] = nlohmann::json::array_t();

    // Go through the journal and use the timestamp to create a
    // unique ID for each entry
    // 遍历 journal，使用时间戳为每个条目创建唯一 ID。
    std::optional<JournalReadState> journalOpt =
        JournalReadState::openDefaults();
    if (!journalOpt)
    {
        messages::internalError(asyncResp->res);
        return;
    }
    JournalReadState& journal = *journalOpt;
    // Seek to the end
    // 定位到末尾。
    if (journal.seekTail() < 0)
    {
        messages::internalError(asyncResp->res);
        return;
    }

    // Get the last entry
    // 获取最后一条条目。
    if (journal.previous() < 0)
    {
        messages::internalError(asyncResp->res);
        return;
    }

    // Get the last sequence number
    // 获取最后的序列号。
    uint64_t endSeqNum = 0;
    if (journal.getSeqnum(endSeqNum) < 0)
    {
        messages::internalError(asyncResp->res);
        return;
    }

    // Seek to the beginning
    // 定位到开头。
    if (journal.seekHead() < 0)
    {
        messages::internalError(asyncResp->res);
        return;
    }

    // Get the first entry
    // 获取第一条条目。
    if (journal.next() < 0)
    {
        messages::internalError(asyncResp->res);
        return;
    }

    // Get the first sequence number
    // 获取第一个序列号。
    uint64_t startSeqNum = 0;
    if (journal.getSeqnum(startSeqNum) < 0)
    {
        messages::internalError(asyncResp->res);
        return;
    }

    BMCWEB_LOG_DEBUG("journal Sequence IDs start:{} end:{}", startSeqNum,
                     endSeqNum);

    // Add 1 to account for the last entry
    // 加 1 以计入最后一条条目。
    uint64_t totalEntries = endSeqNum - startSeqNum + 1;
    asyncResp->res.jsonValue["Members@odata.count"] = totalEntries;
    if (skip + top < totalEntries)
    {
        asyncResp->res.jsonValue["Members@odata.nextLink"] =
            boost::urls::format(
                "/redfish/v1/Managers/{}/LogServices/Journal/Entries?$skip={}",
                BMCWEB_REDFISH_MANAGER_URI_NAME, std::to_string(skip + top));
    }
    uint64_t index = 0;
    if (skip > 0)
    {
        if (journal.nextSkip(skip) < 0)
        {
            messages::internalError(asyncResp->res);
            return;
        }
    }
    BMCWEB_LOG_DEBUG("Index was {}", index);
    readJournalEntries(top, asyncResp, {std::move(journal)});
}

inline void handleManagersJournalEntriesLogEntryGet(
    App& app, const crow::Request& req,
    const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
    const std::string& managerId, const std::string& entryID)
{
    if (!redfish::setUpRedfishRoute(app, req, asyncResp))
    {
        return;
    }

    if (managerId != BMCWEB_REDFISH_MANAGER_URI_NAME)
    {
        messages::resourceNotFound(asyncResp->res, "Manager", managerId);
        return;
    }

    std::optional<JournalReadState> journalOpt =
        JournalReadState::openDefaults();
    if (!journalOpt)
    {
        messages::internalError(asyncResp->res);
        return;
    }
    JournalReadState& journal = *journalOpt;

    std::string cursor;
    if (!crow::utility::base64Decode(entryID, cursor))
    {
        messages::resourceNotFound(asyncResp->res, "LogEntry", entryID);
        return;
    }

    // Go to the cursor in the log
    // 定位到日志中的游标位置。
    int ret = journal.seekCursor(cursor);
    if (ret < 0)
    {
        messages::resourceNotFound(asyncResp->res, "LogEntry", entryID);
        return;
    }

    if (journal.next() < 0)
    {
        messages::internalError(asyncResp->res);
        return;
    }

    ret = journal.testCursor(cursor);
    if (ret == 0)
    {
        messages::resourceNotFound(asyncResp->res, "LogEntry", entryID);
        return;
    }
    if (ret < 0)
    {
        messages::internalError(asyncResp->res);
        return;
    }

    nlohmann::json::object_t bmcJournalLogEntry;
    if (!fillBMCJournalLogEntryJson(journal, bmcJournalLogEntry))
    {
        messages::internalError(asyncResp->res);
        return;
    }
    asyncResp->res.jsonValue.update(bmcJournalLogEntry);
}

inline void requestRoutesBMCJournalLogService(App& app)
{
    BMCWEB_ROUTE(app, "/redfish/v1/Managers/<str>/LogServices/Journal/")
        .privileges(redfish::privileges::getLogService)
        .methods(boost::beast::http::verb::get)(
            std::bind_front(handleManagersLogServiceJournalGet, std::ref(app)));

    BMCWEB_ROUTE(app, "/redfish/v1/Managers/<str>/LogServices/Journal/Entries/")
        .privileges(redfish::privileges::getLogEntryCollection)
        .methods(boost::beast::http::verb::get)(std::bind_front(
            handleManagersJournalLogEntryCollectionGet, std::ref(app)));

    BMCWEB_ROUTE(
        app, "/redfish/v1/Managers/<str>/LogServices/Journal/Entries/<str>/")
        .privileges(redfish::privileges::getLogEntry)
        .methods(boost::beast::http::verb::get)(std::bind_front(
            handleManagersJournalEntriesLogEntryGet, std::ref(app)));
}
} // namespace redfish
