// SPDX-License-Identifier: Apache-2.0
// SPDX-FileCopyrightText: Copyright OpenBMC Authors
// SPDX-FileCopyrightText: Copyright 2020 Intel Corporation
#include "event_log.hpp"

#include "logging.hpp"
#include "registries.hpp"
#include "str_utility.hpp"
#include "utils/time_utils.hpp"

#include <nlohmann/json.hpp>

#include <cerrno>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <format>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace redfish
{

namespace event_log
{

bool getUniqueEntryID(UniqueEntryIDState& state, const std::string& logEntry,
                      std::string& entryID)
{
    // Get the entry timestamp
    // The redfish log format is "<Timestamp> <MessageId>,<MessageArgs>"
    // 获取条目时间戳
    // redfish 日志格式为 "<Timestamp> <MessageId>,<MessageArgs>"
    std::string_view timestamp = logEntry;
    size_t space = logEntry.find_first_of(' ');
    if (space != std::string::npos)
    {
        timestamp = timestamp.substr(0, space);
    }
    std::optional<time_utils::usSinceEpoch> curTs =
        time_utils::dateStringToEpoch(timestamp);
    if (!curTs)
    {
        return false;
    }
    using std::chrono::floor;
    // Convert to seconds.  Journal doesn't give sub-seconds resolution (yet)
    // so this should be no op.
    // 转换为秒。Journal 目前不提供亚秒级精度（尚未），
    // 因此这应该是个空操作。
    std::chrono::seconds curTsSec = floor<std::chrono::seconds>(*curTs);
    std::chrono::sys_seconds curTsSys(curTsSec);
    // If the timestamp isn't unique, increment the index
    // 如果时间戳不唯一，则递增索引
    state.index = (curTsSys == state.prevTs) ? state.index + 1 : 0;

    // Save the timestamp
    // 保存时间戳
    state.prevTs = curTsSys;

    entryID = std::to_string(curTsSys.time_since_epoch().count());
    if (state.index > 0)
    {
        entryID += "_" + std::to_string(state.index);
    }
    return true;
}

int getEventLogParams(const std::string& logEntry, std::string& timestamp,
                      std::string& messageID,
                      std::vector<std::string>& messageArgs)
{
    // The redfish log format is "<Timestamp> <MessageId>,<MessageArgs>"
    // First get the Timestamp
    // redfish 日志格式为 "<Timestamp> <MessageId>,<MessageArgs>"
    // 首先获取 Timestamp
    size_t space = logEntry.find_first_of(' ');
    if (space == std::string::npos)
    {
        BMCWEB_LOG_ERROR("EventLog Params: could not find first space: {}",
                         logEntry);
        return -EINVAL;
    }
    timestamp = logEntry.substr(0, space);
    // Then get the log contents
    // 然后获取日志内容
    size_t entryStart = logEntry.find_first_not_of(' ', space);
    if (entryStart == std::string::npos)
    {
        BMCWEB_LOG_ERROR("EventLog Params: could not find log contents: {}",
                         logEntry);
        return -EINVAL;
    }
    std::string_view entry(logEntry);
    entry.remove_prefix(entryStart);
    // Use split to separate the entry into its fields
    // 使用 split 将条目拆分为各个字段
    std::vector<std::string> logEntryFields;
    bmcweb::split(logEntryFields, entry, ',');
    // We need at least a MessageId to be valid
    // 至少需要一个 MessageId 才算有效
    if (logEntryFields.empty())
    {
        BMCWEB_LOG_ERROR("EventLog Params: could not find entry fields: {}",
                         logEntry);
        return -EINVAL;
    }
    messageID = logEntryFields[0];

    // Get the MessageArgs from the log if there are any
    // 如果日志中有 MessageArgs，则获取它们
    if (logEntryFields.size() > 1)
    {
        const std::string& messageArgsStart = logEntryFields[1];
        // If the first string is empty, assume there are no MessageArgs
        // 如果第一个字符串为空，则假定没有 MessageArgs
        if (!messageArgsStart.empty())
        {
            messageArgs.assign(logEntryFields.begin() + 1,
                               logEntryFields.end());
        }
    }

    return 0;
}

int formatEventLogEntry(uint64_t eventId, const std::string& logEntryID,
                        const std::string& messageID,
                        const std::span<std::string_view> messageArgs,
                        std::string timestamp, const std::string& customText,
                        nlohmann::json::object_t& logEntryJson)
{
    std::optional<registries::MessageId> msgComponents =
        registries::getMessageComponents(messageID);
    if (msgComponents == std::nullopt)
    {
        BMCWEB_LOG_DEBUG("Could not get Message components");
        return -1;
    }

    std::optional<registries::RegistryEntryRef> registry =
        registries::getRegistryFromPrefix(msgComponents->registryName);
    if (!registry)
    {
        BMCWEB_LOG_DEBUG("Could not get registry from prefix");
        return -1;
    }

    // Get the Message from the MessageKey and RegistryEntries
    // 从 MessageKey 和 RegistryEntries 获取 Message
    const registries::Message* message = registries::getMessageFromRegistry(
        msgComponents->messageKey, registry->get().entries);

    if (message == nullptr)
    {
        BMCWEB_LOG_DEBUG(
            "Could not find MessageKey '{}' for log entry {} in registry",
            msgComponents->messageKey, logEntryID);
        return -1;
    }

    std::string msg =
        redfish::registries::fillMessageArgs(messageArgs, message->message);
    if (msg.empty())
    {
        BMCWEB_LOG_DEBUG("Message is empty after filling fillMessageArgs");
        return -1;
    }

    // Get the Created time from the timestamp. The log timestamp is in
    // RFC3339 format which matches the Redfish format except for the
    // fractional seconds between the '.' and the '+', so just remove them.
    // 从时间戳获取 Created 时间。日志时间戳采用 RFC3339 格式，
    // 它与 Redfish 格式基本一致，只是 '.' 和 '+' 之间的
    // 小数秒部分不同，因此直接去掉即可。
    std::size_t dot = timestamp.find_first_of('.');
    std::size_t plus = timestamp.find_first_of('+', dot);
    if (dot != std::string::npos && plus != std::string::npos)
    {
        timestamp.erase(dot, plus - dot);
    }

    const unsigned int& versionMajor = registry->get().header.versionMajor;
    const unsigned int& versionMinor = registry->get().header.versionMinor;

    // Fill in the log entry with the gathered data
    // 用收集到的数据填充日志条目
    logEntryJson["EventId"] = std::to_string(eventId);

    logEntryJson["Severity"] = message->messageSeverity;
    logEntryJson["Message"] = std::move(msg);
    logEntryJson["MessageId"] =
        std::format("{}.{}.{}.{}", msgComponents->registryName, versionMajor,
                    versionMinor, msgComponents->messageKey);
    logEntryJson["MessageArgs"] = messageArgs;
    logEntryJson["EventTimestamp"] = std::move(timestamp);
    logEntryJson["Context"] = customText;
    return 0;
}

} // namespace event_log

} // namespace redfish
