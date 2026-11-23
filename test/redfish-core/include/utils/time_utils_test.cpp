// SPDX-License-Identifier: Apache-2.0
// SPDX-FileCopyrightText: Copyright OpenBMC Authors
#include "http_response.hpp"
#include "utils/time_utils.hpp"

#include <boost/beast/http/status.hpp>

#include <chrono>
#include <cstdint>
#include <limits>
#include <optional>
#include <ratio>
#include <string>
#include <type_traits>
#include <version>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

using ::testing::MatchesRegex;
using namespace std::literals::chrono_literals;

namespace redfish::time_utils
{
namespace
{

TEST(FromDurationTest, PositiveTests)
{
    EXPECT_EQ(fromDurationString("PT12S"), std::chrono::milliseconds(12000));
    EXPECT_EQ(fromDurationString("PT0.204S"), std::chrono::milliseconds(204));
    EXPECT_EQ(fromDurationString("PT0.2S"), std::chrono::milliseconds(200));
    EXPECT_EQ(fromDurationString("PT50M"), std::chrono::milliseconds(3000000));
    EXPECT_EQ(fromDurationString("PT23H"), std::chrono::milliseconds(82800000));
    EXPECT_EQ(fromDurationString("P51D"),
              std::chrono::milliseconds(4406400000));
    EXPECT_EQ(fromDurationString("PT2H40M10.1S"),
              std::chrono::milliseconds(9610100));
    EXPECT_EQ(fromDurationString("P20DT2H40M10.1S"),
              std::chrono::milliseconds(1737610100));
    EXPECT_EQ(fromDurationString(""), std::chrono::milliseconds(0));
}

TEST(FromDurationTest, NegativeTests)
{
    EXPECT_EQ(fromDurationString("PTS"), std::nullopt);
    EXPECT_EQ(fromDurationString("P1T"), std::nullopt);
    EXPECT_EQ(fromDurationString("PT100M1000S100"), std::nullopt);
    EXPECT_EQ(fromDurationString("PDTHMS"), std::nullopt);
    EXPECT_EQ(fromDurationString("P9999999999999999999999999DT"), std::nullopt);
    EXPECT_EQ(fromDurationString("PD222T222H222M222.222S"), std::nullopt);
    EXPECT_EQ(fromDurationString("PT99999H9999999999999999999999M99999999999S"),
              std::nullopt);
    EXPECT_EQ(fromDurationString("PT-9H"), std::nullopt);
}
TEST(ToDurationTest, PositiveTests)
{
    EXPECT_EQ(toDurationString(std::chrono::milliseconds(12000)), "PT12.000S");
    EXPECT_EQ(toDurationString(std::chrono::milliseconds(204)), "PT0.204S");
    EXPECT_EQ(toDurationString(std::chrono::milliseconds(200)), "PT0.200S");
    EXPECT_EQ(toDurationString(std::chrono::milliseconds(3000000)), "PT50M");
    EXPECT_EQ(toDurationString(std::chrono::milliseconds(82800000)), "PT23H");
    EXPECT_EQ(toDurationString(std::chrono::milliseconds(4406400000)), "P51DT");
    EXPECT_EQ(toDurationString(std::chrono::milliseconds(9610100)),
              "PT2H40M10.100S");
    EXPECT_EQ(toDurationString(std::chrono::milliseconds(1737610100)),
              "P20DT2H40M10.100S");
}

TEST(ToDurationTest, NegativeTests)
{
    EXPECT_EQ(toDurationString(std::chrono::milliseconds(-250)), "");
}

TEST(ToDurationStringFromUintTest, PositiveTests)
{
    uint64_t maxAcceptedTimeMs =
        static_cast<uint64_t>(std::chrono::milliseconds::max().count());

    EXPECT_NE(toDurationStringFromUint(maxAcceptedTimeMs), std::nullopt);
    EXPECT_EQ(toDurationStringFromUint(0), "PT");
    EXPECT_EQ(toDurationStringFromUint(250), "PT0.250S");
    EXPECT_EQ(toDurationStringFromUint(5000), "PT5.000S");
}

TEST(ToDurationStringFromUintTest, NegativeTests)
{
    uint64_t minNotAcceptedTimeMs =
        static_cast<uint64_t>(std::chrono::milliseconds::max().count()) + 1;

    EXPECT_EQ(toDurationStringFromUint(minNotAcceptedTimeMs), std::nullopt);
    EXPECT_EQ(toDurationStringFromUint(static_cast<uint64_t>(-1)),
              std::nullopt);
}

TEST(GetDateTimeStdtime, UTCTests)
{
    using time_point = std::chrono::system_clock::time_point;
    EXPECT_EQ(
        getDateTimeStdtime(time_point(std::chrono::seconds(1638312095)), "UTC"),
        "2021-11-30T22:41:35+00:00");

    EXPECT_EQ(getDateTimeStdtime(time_point::max(), "UTC"),
              "2262-04-11T23:47:16+00:00");

    EXPECT_EQ(getDateTimeStdtime(time_point::min(), "UTC"),
              "1970-01-01T00:00:00+00:00");

    EXPECT_EQ(getDateTimeStdtime(time_point{std::chrono::seconds(0)}, "UTC"),
              "1970-01-01T00:00:00+00:00");
}

TEST(getDateTimeStdtime, TimezoneTests)
{
    // 2021-11-30T22:41:35 UTC (1638312095)
    // 2021-11-30T22:41:35 UTC (1638312095)
    using time_point = std::chrono::system_clock::time_point;
    constexpr time_point testTime =
        time_point{std::chrono::seconds(1638312095)};

    // UTC (offset +00:00)
    // UTC（偏移 +00:00）
    EXPECT_EQ(getDateTimeStdtime(testTime, "UTC"), "2021-11-30T22:41:35+00:00");

    // Positive offset: Europe/Berlin (+01:00 in winter)
    // 正偏移：Europe/Berlin（冬季 +01:00）
    EXPECT_EQ(getDateTimeStdtime(testTime, "Europe/Berlin"),
              "2021-11-30T23:41:35+01:00");

    // Negative offset: America/New_York (-05:00 in winter)
    // 负偏移：America/New_York（冬季 -05:00）
    EXPECT_EQ(getDateTimeStdtime(testTime, "America/New_York"),
              "2021-11-30T17:41:35-05:00");

    // Non-hour-aligned offset: Asia/Kolkata (+05:30)
    // 非整小时偏移：Asia/Kolkata (+05:30)
    EXPECT_EQ(getDateTimeStdtime(testTime, "Asia/Kolkata"),
              "2021-12-01T04:11:35+05:30");

    // Non-hour-aligned offset: Asia/Kathmandu (+05:45)
    // 非整小时偏移：Asia/Kathmandu (+05:45)
    EXPECT_EQ(getDateTimeStdtime(testTime, "Asia/Kathmandu"),
              "2021-12-01T04:26:35+05:45");

    // Extreme westerly timezone: Etc/GMT+12 (-12:00)
    // 极西时区：Etc/GMT+12 (-12:00)
    EXPECT_EQ(getDateTimeStdtime(testTime, "Etc/GMT+12"),
              "2021-11-30T10:41:35-12:00");

    // Extreme easterly timezone: Pacific/Kiritimati (+14:00)
    // 极东时区：Pacific/Kiritimati (+14:00)
    EXPECT_EQ(getDateTimeStdtime(testTime, "Pacific/Kiritimati"),
              "2021-12-01T12:41:35+14:00");

    // Most of the below limit tests assume system clock is in nanoseconds
    // 以下大多数极限测试都假设系统时钟以纳秒为单位
    static_assert(std::is_same_v<time_point::period, std::nano>);
    // If int64_max is allowed, test it
    // 如果允许 int64_max，则对其进行测试
    if (time_point::max().time_since_epoch().count() >=
        std::numeric_limits<int64_t>::max())
    {
        time_point::duration max64dur(std::numeric_limits<int64_t>::max());
        EXPECT_EQ(getDateTimeStdtime(time_point(max64dur), "UTC"),
                  "2262-04-11T23:47:16+00:00");
    }
    EXPECT_EQ(getDateTimeStdtime(time_point::min(), "UTC"),
              "1970-01-01T00:00:00+00:00");

    // Use fixed-offset CEST (+02:00) to avoid DST ambiguity at extreme values
    // 使用固定偏移的 CEST (+02:00)，以避免极值处的 DST 歧义
    EXPECT_EQ(getDateTimeStdtime(time_point::max(), "Etc/GMT-2"),
              "2262-04-12T01:47:16+02:00");

    EXPECT_EQ(getDateTimeStdtime(time_point::min(), "Etc/GMT-2"),
              "1970-01-01T00:00:00+02:00");
}

TEST(GetDateTimeUint, ConversionTests)
{
    EXPECT_EQ(getDateTimeUint(uint64_t{1638312095}, "UTC"),
              "2021-11-30T22:41:35+00:00");
    // some time in the future, beyond 2038
    // 未来的某个时间，超出 2038 年
    EXPECT_EQ(getDateTimeUint(uint64_t{41638312095}, "UTC"),
              "3289-06-18T21:48:15+00:00");
    // the maximum time we support
    // 我们支持的最大时间
    EXPECT_EQ(getDateTimeUint(uint64_t{253402300799}, "UTC"),
              "9999-12-31T23:59:59+00:00");

    // returns the maximum Redfish date
    // 返回最大的 Redfish 日期
    EXPECT_EQ(getDateTimeUint(std::numeric_limits<uint64_t>::max(), "UTC"),
              "9999-12-31T23:59:59+00:00");

    EXPECT_EQ(getDateTimeUint(std::numeric_limits<uint64_t>::min(), "UTC"),
              "1970-01-01T00:00:00+00:00");
}

TEST(GetDateTimeUintMs, ConverstionTests)
{
    EXPECT_EQ(getDateTimeUintMs(uint64_t{1638312095123}, "UTC"),
              "2021-11-30T22:41:35.123+00:00");
    // returns the maximum Redfish date
    // 返回最大的 Redfish 日期
    EXPECT_EQ(getDateTimeUintMs(std::numeric_limits<uint64_t>::max(), "UTC"),
              "9999-12-31T23:59:59.999+00:00");
    EXPECT_EQ(getDateTimeUintMs(std::numeric_limits<uint64_t>::min(), "UTC"),
              "1970-01-01T00:00:00.000+00:00");
}

TEST(Utility, GetDateTimeUintUs)
{
    EXPECT_EQ(getDateTimeUintUs(uint64_t{1638312095123456}, "UTC"),
              "2021-11-30T22:41:35.123456+00:00");
    // returns the maximum Redfish date
    // 返回最大的 Redfish 日期
    EXPECT_EQ(getDateTimeUintUs(std::numeric_limits<uint64_t>::max(), "UTC"),
              "9999-12-31T23:59:59.999999+00:00");
    EXPECT_EQ(getDateTimeUintUs(std::numeric_limits<uint64_t>::min(), "UTC"),
              "1970-01-01T00:00:00.000000+00:00");
}

TEST(Utility, DateStringToEpoch)
{
    EXPECT_EQ(dateStringToEpoch("2021-11-30T22:41:35.123456+00:00"),
              usSinceEpoch{1638312095123456});
    // no timezone
    // 无时区
    EXPECT_EQ(dateStringToEpoch("2021-11-30T22:41:35.123456"),
              usSinceEpoch{1638312095123456});
    // Milliseconds precision
    // 毫秒精度
    EXPECT_EQ(dateStringToEpoch("2021-11-30T22:41:35.123"),
              usSinceEpoch{1638312095123000});
    // Seconds precision
    // 秒精度
    EXPECT_EQ(dateStringToEpoch("2021-11-30T22:41:35"),
              usSinceEpoch{1638312095000000});

    // valid datetime format
    // 有效的日期时间格式
    EXPECT_EQ(dateStringToEpoch("20230531T000000Z"),
              usSinceEpoch{1685491200000000});

    // valid datetime format
    // 有效的日期时间格式
    EXPECT_EQ(dateStringToEpoch("20230531"), usSinceEpoch{1685491200000000});

    // Non zero timezone
    // 非零时区
    EXPECT_EQ(dateStringToEpoch("2021-11-30T22:41:35.123456+04:00"),
              usSinceEpoch{1638297695123456});

    // Epoch
    // Epoch（纪元）
    EXPECT_EQ(dateStringToEpoch("1970-01-01T00:00:00.000000+00:00"),
              usSinceEpoch{0});

    // Max time
    // 最大时间
    EXPECT_EQ(dateStringToEpoch("9999-12-31T23:59:59.999999+00:00"),
              usSinceEpoch{253402300799999999});

    // Underflow
    // 下溢
    EXPECT_EQ(dateStringToEpoch("1969-12-30T23:59:59.999999+00:00"),
              std::nullopt);
}

TEST(Utility, DateStringToEpochWithInvalidDateTimeFormats)
{
    // invalid month (13)
    // 无效月份 (13)
    EXPECT_EQ(dateStringToEpoch("2024-13-01T12:00:00Z"), std::nullopt);

    // invalid character for month
    // 月份中的无效字符
    EXPECT_EQ(dateStringToEpoch("2024-X-01T12:00:00Z"), std::nullopt);

    // invalid day (32)
    // 无效日期 (32)
    EXPECT_EQ(dateStringToEpoch("2024-07-32T12:00:00Z"), std::nullopt);

    // invalid character for day
    // 日期中的无效字符
    EXPECT_EQ(dateStringToEpoch("2024-07-XT12:00:00Z"), std::nullopt);

    // invalid hour (25)
    // 无效小时 (25)
    EXPECT_EQ(dateStringToEpoch("2024-07-01T25:00:00Z"), std::nullopt);

    // invalid character for hour
    // 小时中的无效字符
    EXPECT_EQ(dateStringToEpoch("2024-07-01TX:00:00Z"), std::nullopt);

    // invalid minute (60)
    // Date.h and std::chrono seem to disagree about whether there is a 60th
    // minute in an hour.  Not clear if this is intended or not, but really
    // isn't that important.  Let std::chrono pass with 61
    // 无效分钟 (60)
    // Date.h 和 std::chrono 似乎对一小时中是否有第 60
    // 分钟存在分歧。尚不清楚这是否有意为之，不过其实
    // 也没那么重要。就让 std::chrono 以 61 通过吧。
#if __cpp_lib_chrono >= 201907L
    EXPECT_EQ(dateStringToEpoch("2024-07-01T12:61:00Z"), std::nullopt);
#else
    EXPECT_EQ(dateStringToEpoch("2024-07-01T12:60:00Z"), std::nullopt);
#endif

    // invalid character for minute
    // 分钟中的无效字符
    EXPECT_EQ(dateStringToEpoch("2024-13-01T12:X:00Z"), std::nullopt);

    // invalid second (60)
    // 无效秒数 (60)
    EXPECT_EQ(dateStringToEpoch("2024-07-01T12:00:XZ"), std::nullopt);

    // invalid character for second
    // 秒中的无效字符
    EXPECT_EQ(dateStringToEpoch("2024-13-01T12:00:00Z"), std::nullopt);

    // invalid timezone
    // 无效时区
    EXPECT_EQ(dateStringToEpoch("2024-07-01T12:00:00X"), std::nullopt);

    // invalid datetime format
    // 无效的日期时间格式
    EXPECT_EQ(dateStringToEpoch("202305"), std::nullopt);

    // invalid month (13), day (99)
    // 无效月份 (13)、日期 (99)
    EXPECT_EQ(dateStringToEpoch("19991399"), std::nullopt);
}

TEST(Utility, GetDateTimeIso8601)
{
    EXPECT_EQ(getDateTimeIso8601("20230531"), "2023-05-31T00:00:00+00:00");

    EXPECT_EQ(getDateTimeIso8601("20230531T000000Z"),
              "2023-05-31T00:00:00+00:00");

    // invalid datetime
    // 无效的日期时间
    EXPECT_EQ(getDateTimeIso8601("202305"), std::nullopt);
}

TEST(Utility, ProductionDateReportPublishesNormalizedDate)
{
    crow::Response res;
    productionDateReport(res, "20230531T000000Z");

    EXPECT_EQ(res.jsonValue["ProductionDate"], "2023-05-31T00:00:00+00:00");
}

TEST(Utility, ProductionDateReportRejectsInvalidBuildDate)
{
    crow::Response res;
    productionDateReport(res, "202305");

    EXPECT_FALSE(res.jsonValue.contains("ProductionDate"));
    EXPECT_EQ(res.result(), boost::beast::http::status::internal_server_error);
}

TEST(Utility, ProductionDateReportEmptyStringLeavesResponseClean)
{
    crow::Response res;
    productionDateReport(res, "");

    EXPECT_FALSE(res.jsonValue.contains("ProductionDate"));
    EXPECT_EQ(res.result(), boost::beast::http::status::ok);
}

TEST(Utility, GetDateTimeOffsetNow)
{
    // Not a lot of good ways to verify "now" is correct, but we can at least
    // verify the format is correct
    // 没有太多好办法验证 "now" 是否正确，但我们至少
    // 可以验证格式是否正确
    const auto [dateTime, offset] = getDateTimeOffsetNow();
    EXPECT_THAT(
        dateTime,
        MatchesRegex(
            "[0-9]{4}-[0-9]{2}-[0-9]{2}T[0-9]{2}:[0-9]{2}:[0-9]{2}[-+][0-9]{2}:[0-9]{2}"));
    EXPECT_THAT(offset, MatchesRegex("[-+][0-9]{2}:[0-9]{2}"));
}

TEST(Utility, GetDateTimeEmptyStringDefaultsToLocal)
{
    // Test that empty string parameter defaults to local timezone
    // 测试空字符串参数是否默认使用本地时区
    uint64_t testTime = 1638312095; // 2021-11-30T22:41:35 UTC

    // Empty string should use local timezone
    // 空字符串应使用本地时区
    std::string localResult = getDateTimeUint(testTime, "");
    std::string defaultResult = getDateTimeUint(testTime);

    // Both should produce same result (default parameter)
    // 两者应产生相同的结果（默认参数）
    EXPECT_EQ(localResult, defaultResult);
    EXPECT_FALSE(localResult.empty());

    // Should be valid ISO 8601 format
    // 应为有效的 ISO 8601 格式
    EXPECT_THAT(
        localResult,
        MatchesRegex("[0-9]{4}-[0-9]{2}-[0-9]{2}T.*[+-][0-9]{2}:[0-9]{2}"));
}

TEST(Utility, GetDateTimeDefaultParameterBehavior)
{
    uint64_t testTime = 1638312095;
    uint64_t testTimeMs = testTime * 1000;
    uint64_t testTimeUs = testTime * 1000000;
    std::chrono::system_clock::time_point testTimeTP =
        std::chrono::system_clock::time_point(std::chrono::seconds(testTime));

    // All functions should treat no argument same as empty string
    // 所有函数都应将无参数视为与空字符串相同
    EXPECT_EQ(getDateTimeUint(testTime), getDateTimeUint(testTime, ""));
    EXPECT_EQ(getDateTimeUintMs(testTimeMs), getDateTimeUintMs(testTimeMs, ""));
    EXPECT_EQ(getDateTimeUintUs(testTimeUs), getDateTimeUintUs(testTimeUs, ""));
    EXPECT_EQ(getDateTimeStdtime(testTimeTP),
              getDateTimeStdtime(testTimeTP, ""));
}

TEST(Utility, GetDateTimeInvalidTimezone)
{
    // Test with invalid timezone name - should handle gracefully
    // 使用无效时区名称进行测试——应能优雅处理
    uint64_t testTime = 1638312095;
    std::string result = getDateTimeUint(testTime, "Invalid/Timezone");

    // Current implementation logs error and returns empty string
    // This is acceptable behavior for invalid input
    // 当前实现会记录错误并返回空字符串
    // 对于无效输入，这是可接受的行为
    EXPECT_TRUE(result.empty());
}

TEST(Utility, GetDateTimeEmptyTimezoneString)
{
    uint64_t testTime = 1638312095;

    // Empty string should work (defaults to local timezone)
    // 空字符串应能正常工作（默认使用本地时区）
    std::string result = getDateTimeUint(testTime, "");
    EXPECT_FALSE(result.empty());
    EXPECT_THAT(result, MatchesRegex("[0-9]{4}-[0-9]{2}-[0-9]{2}T.*"));

    // Should contain timezone offset
    // 应包含时区偏移
    EXPECT_THAT(result, MatchesRegex(".*[+-][0-9]{2}:[0-9]{2}$"));
}

TEST(Utility, GetDateTimeAllFunctionsUseSameTimezone)
{
    uint64_t seconds = 1638312095;
    uint64_t ms = seconds * 1000;
    uint64_t us = seconds * 1000000;
    std::chrono::system_clock::time_point tp =
        std::chrono::system_clock::time_point(std::chrono::seconds(seconds));

    // All functions should produce same date/time portion when using same
    // timezone
    // 使用相同时区时，所有函数应产生相同的
    // 日期/时间部分
    std::string s1 = getDateTimeUint(seconds, "UTC");
    std::string s2 = getDateTimeUintMs(ms, "UTC");
    std::string s3 = getDateTimeUintUs(us, "UTC");
    std::string s4 = getDateTimeStdtime(tp, "UTC");

    // Extract date/time portion (before fractional seconds and timezone)
    // All should start with same date and time
    // 提取日期/时间部分（小数秒和时区之前的部分）
    // 所有结果都应以相同的日期和时间开头
    EXPECT_EQ(s1, "2021-11-30T22:41:35+00:00");
    EXPECT_EQ(s2, "2021-11-30T22:41:35.000+00:00");
    EXPECT_EQ(s3, "2021-11-30T22:41:35.000000+00:00");
    EXPECT_EQ(s4, "2021-11-30T22:41:35+00:00");
}

} // namespace
} // namespace redfish::time_utils
