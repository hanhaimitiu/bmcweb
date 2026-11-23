// SPDX-License-Identifier: Apache-2.0
// SPDX-FileCopyrightText: Copyright OpenBMC Authors
#pragma once

#include "http_response.hpp"

#include <chrono>
#include <cstdint>
#include <optional>
#include <ratio>
#include <string>
#include <string_view>
#include <utility>

namespace redfish
{

namespace time_utils
{

/**
 * @brief Convert string that represents value in Duration Format to its numeric
 *        equivalent.
 */
/* @brief 将表示 Duration 格式值的字符串转换为对应的
 *        数值。 */
std::optional<std::chrono::milliseconds> fromDurationString(std::string_view v);

/**
 * @brief Convert time value into duration format that is based on ISO 8601.
 *        Example output: "P12DT1M5.5S"
 *        Ref: Redfish Specification, Section 9.4.4. Duration values
 */
/* @brief 将时间值转换为基于 ISO 8601 的 duration 格式。
 *        示例输出："P12DT1M5.5S"
 *        参考：Redfish 规范 9.4.4 节 Duration values */
std::string toDurationString(std::chrono::milliseconds ms);

std::optional<std::string> toDurationStringFromUint(uint64_t timeMs);

// Returns the formatted date time string.
// Note that the maximum supported date is 9999-12-31T23:59:59+00:00, if
// the given |secondsSinceEpoch| is too large, this call returns the maximum
// supported date.  timezoneName allows selecting a specific time zone by the
// tzdb name, for example "UTC" or "America/New_York".
// 返回格式化后的日期时间字符串。
// 注意，支持的最大日期为 9999-12-31T23:59:59+00:00，如果
// 传入的 |secondsSinceEpoch| 过大，则本调用返回所支持的
// 最大日期。timezoneName 允许通过 tzdb 名称指定特定时区，
// 例如 "UTC" 或 "America/New_York"。
std::string getDateTimeUint(uint64_t secondsSinceEpoch,
                            std::string_view timezoneName = "");

// Returns the tz-aware formatted date time string with millisecond precision
// Note that the maximum supported date is 9999-12-31T23:59:59+00:00, if
// the given |secondsSinceEpoch| is too large, we return the maximum supported
// date.
// 返回带时区信息、精度为毫秒的格式化日期时间字符串。
// 注意，支持的最大日期为 9999-12-31T23:59:59+00:00，如果
// 传入的 |secondsSinceEpoch| 过大，则返回所支持的
// 最大日期。
std::string getDateTimeUintMs(uint64_t milliSecondsSinceEpoch,
                              std::string_view timezoneName = "");

// Returns the tz-aware formatted date time string with microsecond precision
// 返回带时区信息、精度为微秒的格式化日期时间字符串
std::string getDateTimeUintUs(uint64_t microSecondsSinceEpoch,
                              std::string_view timezoneName = "");
std::string getDateTimeStdtime(std::chrono::system_clock::time_point timePoint,
                               std::string_view timezoneName = "");

/**
 * Returns the current Date, Time & the local Time Offset
 * information in a pair
 *
 * @param[in] bool - true if the time is to be returned in local time, false if
 * in UTC time
 *
 * @return std::pair<std::string, std::string>, which consist
 * of current DateTime & the TimeOffset strings respectively.
 */
/* 以 pair 形式返回当前的日期、时间和本地时间偏移量
 * 信息
 * @param[in] bool - 为 true 时以本地时间返回，为 false 时
 * 以 UTC 时间返回
 * @return std::pair<std::string, std::string>，分别包含
 * 当前的 DateTime 和 TimeOffset 字符串。 */
std::pair<std::string, std::string> getDateTimeOffsetNow();

using usSinceEpoch = std::chrono::duration<int64_t, std::micro>;
std::optional<usSinceEpoch> dateStringToEpoch(std::string_view datetime);

/**
 * @brief Returns the datetime in ISO 8601 format
 *
 * @param[in] std::string_view the date of item manufacture in ISO 8601 format,
 *            either as YYYYMMDD or YYYYMMDDThhmmssZ
 * Ref: https://github.com/openbmc/phosphor-dbus-interfaces/blob/master/yaml/
 *      xyz/openbmc_project/Inventory/Decorator/Asset.interface.yaml#L16
 *
 * @return std::string which consist the datetime
 */
/* @brief 返回 ISO 8601 格式的日期时间
 * @param[in] std::string_view 物品的制造日期，ISO 8601 格式，
 *            可以是 YYYYMMDD 或 YYYYMMDDThhmmssZ
 * 参考：https://github.com/openbmc/phosphor-dbus-interfaces/blob/master/yaml/
 *      xyz/openbmc_project/Inventory/Decorator/Asset.interface.yaml#L16
 * @return 包含该日期时间的 std::string */
std::optional<std::string> getDateTimeIso8601(std::string_view datetime);

/**
 * @brief ProductionDate report
 */
/* @brief ProductionDate（生产日期）上报 */
void productionDateReport(crow::Response& res, const std::string& buildDate);

} // namespace time_utils
} // namespace redfish
