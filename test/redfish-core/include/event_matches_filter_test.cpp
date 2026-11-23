#include "event_matches_filter.hpp"
#include "event_service_store.hpp"

#include <nlohmann/json.hpp>

#include <string_view>

#include <gtest/gtest.h>

namespace redfish
{

TEST(EventServiceManager, eventMatchesFilter)
{
    {
        persistent_data::UserSubscription sub;
        nlohmann::json::object_t event;

        // Default constructed should always pass
        // 默认构造的对象应始终通过
        EXPECT_TRUE(eventMatchesFilter(sub, event, "Event"));
    }
    {
        nlohmann::json::object_t event;
        // Resource types filter
        // 资源类型过滤器
        persistent_data::UserSubscription sub;
        sub.resourceTypes.emplace_back("Task");
        EXPECT_FALSE(eventMatchesFilter(sub, event, "Event"));
        EXPECT_TRUE(eventMatchesFilter(sub, event, "Task"));
    }
    {
        nlohmann::json::object_t event;
        // Resource types filter
        // 资源类型过滤器
        persistent_data::UserSubscription sub;
        sub.registryMsgIds.emplace_back("OpenBMC.PostComplete");

        // Correct message registry
        // 正确的消息注册表
        event["MessageId"] = "OpenBMC.0.1.PostComplete";
        EXPECT_TRUE(eventMatchesFilter(sub, event, "Event"));

        // Different message registry
        // 不同的消息注册表
        event["MessageId"] = "Task.0.1.PostComplete";
        EXPECT_FALSE(eventMatchesFilter(sub, event, "Event"));

        // Different MessageId
        // 不同的 MessageId
        event["MessageId"] = "OpenBMC.0.1.NoMatch";
        EXPECT_FALSE(eventMatchesFilter(sub, event, "Event"));
    }
    {
        nlohmann::json::object_t event;
        // Resource types filter
        // 资源类型过滤器
        persistent_data::UserSubscription sub;
        event["MessageId"] = "OpenBMC.0.1.PostComplete";

        // Correct message registry
        // 正确的消息注册表
        sub.registryPrefixes.emplace_back("OpenBMC");
        EXPECT_TRUE(eventMatchesFilter(sub, event, "Event"));

        // Different message registry
        // 不同的消息注册表
        event["MessageId"] = "Task.0.1.PostComplete";
        EXPECT_FALSE(eventMatchesFilter(sub, event, "Event"));
    }
    {
        nlohmann::json::object_t event;
        // Resource types filter
        // 资源类型过滤器
        {
            persistent_data::UserSubscription sub;
            event["OriginOfCondition"] = "/redfish/v1/Managers/bmc";

            // Correct origin
            // 正确的来源（origin）
            sub.originResources.emplace_back("/redfish/v1/Managers/bmc");
            EXPECT_TRUE(eventMatchesFilter(sub, event, "Event"));
        }
        {
            persistent_data::UserSubscription sub;
            // Incorrect origin
            // 错误的来源（origin）
            sub.originResources.clear();
            sub.originResources.emplace_back("/redfish/v1/Managers/bmc_not");
            EXPECT_FALSE(eventMatchesFilter(sub, event, "Event"));
        }
    }
}
} // namespace redfish
