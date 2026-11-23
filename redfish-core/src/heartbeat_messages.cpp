/****************************************************************
 *                 READ THIS WARNING FIRST
 * This is an auto-generated header which contains definitions
 * for Redfish DMTF defined messages.
 * DO NOT modify this registry outside of running the
 * parse_registries.py script.  The definitions contained within
 * this file are owned by DMTF.  Any modifications to these files
 * should be first pushed to the relevant registry in the DMTF
 * github organization.
 ***************************************************************/
/*                 请先阅读此警告
 * 这是一个自动生成的头文件，包含
 * Redfish DMTF 定义的消息定义。
 * 不要在运行 parse_registries.py 脚本之外
 * 修改此注册表。此文件中包含的定义
 * 归 DMTF 所有。对这些文件的任何修改
 * 都应先提交到 DMTF github 组织中的
 * 相应注册表。 */
#include "heartbeat_messages.hpp"

#include "registries.hpp"
#include "registries/heartbeat_event_message_registry.hpp"

#include <nlohmann/json.hpp>

#include <array>
#include <cstddef>
#include <span>
#include <string_view>

// Clang can't seem to decide whether this header needs to be included or not,
// and is inconsistent.  Include it for now
// NOLINTNEXTLINE(misc-include-cleaner)
// Clang 似乎无法确定是否需要包含这个头文件，
// 而且表现不一致。暂时先包含它
// NOLINTNEXTLINE(misc-include-cleaner)
#include <cstdint>
// NOLINTNEXTLINE(misc-include-cleaner)
#include <string>
// NOLINTNEXTLINE(misc-include-cleaner)
#include <utility>

namespace redfish
{

namespace messages
{

static nlohmann::json::object_t getLog(
    redfish::registries::HeartbeatEvent::Index name,
    std::span<const std::string_view> args)
{
    size_t index = static_cast<size_t>(name);
    if (index >= redfish::registries::HeartbeatEvent::registry.size())
    {
        return {};
    }
    return getLogFromRegistry(redfish::registries::HeartbeatEvent::header,
                              redfish::registries::HeartbeatEvent::registry,
                              index, args);
}

/**
 * @internal
 * @brief Formats RedfishServiceFunctional message into JSON
 *
 * See header file for more information
 * @endinternal
 */
/* @internal
 * @brief 将 RedfishServiceFunctional（Redfish 服务功能正常）消息格式化为 JSON
 * 更多信息请参见头文件
 * @endinternal */
nlohmann::json::object_t redfishServiceFunctional()
{
    return getLog(
        redfish::registries::HeartbeatEvent::Index::redfishServiceFunctional,
        {});
}

} // namespace messages
} // namespace redfish
