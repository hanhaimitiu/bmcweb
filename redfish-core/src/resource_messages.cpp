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
#include "resource_messages.hpp"

#include "registries.hpp"
#include "registries/resource_event_message_registry.hpp"

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
    redfish::registries::ResourceEvent::Index name,
    std::span<const std::string_view> args)
{
    size_t index = static_cast<size_t>(name);
    if (index >= redfish::registries::ResourceEvent::registry.size())
    {
        return {};
    }
    return getLogFromRegistry(redfish::registries::ResourceEvent::header,
                              redfish::registries::ResourceEvent::registry,
                              index, args);
}

/**
 * @internal
 * @brief Formats ResourceCreated message into JSON
 *
 * See header file for more information
 * @endinternal
 */
/* @internal
 * @brief 将 ResourceCreated（资源已创建）消息格式化为 JSON
 * 更多信息请参见头文件
 * @endinternal */
nlohmann::json::object_t resourceCreated()
{
    return getLog(redfish::registries::ResourceEvent::Index::resourceCreated,
                  {});
}

/**
 * @internal
 * @brief Formats ResourceRemoved message into JSON
 *
 * See header file for more information
 * @endinternal
 */
/* @internal
 * @brief 将 ResourceRemoved（资源已删除）消息格式化为 JSON
 * 更多信息请参见头文件
 * @endinternal */
nlohmann::json::object_t resourceRemoved()
{
    return getLog(redfish::registries::ResourceEvent::Index::resourceRemoved,
                  {});
}

/**
 * @internal
 * @brief Formats ResourceErrorsDetected message into JSON
 *
 * See header file for more information
 * @endinternal
 */
/* @internal
 * @brief 将 ResourceErrorsDetected（检测到资源错误）消息格式化为 JSON
 * 更多信息请参见头文件
 * @endinternal */
nlohmann::json::object_t resourceErrorsDetected(std::string_view arg1,
                                                std::string_view arg2)
{
    return getLog(
        redfish::registries::ResourceEvent::Index::resourceErrorsDetected,
        std::to_array({arg1, arg2}));
}

/**
 * @internal
 * @brief Formats ResourceErrorsCorrected message into JSON
 *
 * See header file for more information
 * @endinternal
 */
/* @internal
 * @brief 将 ResourceErrorsCorrected（资源错误已纠正）消息格式化为 JSON
 * 更多信息请参见头文件
 * @endinternal */
nlohmann::json::object_t resourceErrorsCorrected(std::string_view arg1,
                                                 std::string_view arg2)
{
    return getLog(
        redfish::registries::ResourceEvent::Index::resourceErrorsCorrected,
        std::to_array({arg1, arg2}));
}

/**
 * @internal
 * @brief Formats ResourceErrorThresholdExceeded message into JSON
 *
 * See header file for more information
 * @endinternal
 */
/* @internal
 * @brief 将 ResourceErrorThresholdExceeded（超出资源错误阈值）消息格式化为 JSON
 * 更多信息请参见头文件
 * @endinternal */
nlohmann::json::object_t resourceErrorThresholdExceeded(std::string_view arg1,
                                                        uint64_t arg2)
{
    std::string arg2Str = std::to_string(arg2);
    return getLog(redfish::registries::ResourceEvent::Index::
                      resourceErrorThresholdExceeded,
                  std::to_array<std::string_view>({arg1, arg2Str}));
}

/**
 * @internal
 * @brief Formats ResourceErrorThresholdCleared message into JSON
 *
 * See header file for more information
 * @endinternal
 */
/* @internal
 * @brief 将 ResourceErrorThresholdCleared（资源错误阈值已清除）消息格式化为 JSON
 * 更多信息请参见头文件
 * @endinternal */
nlohmann::json::object_t resourceErrorThresholdCleared(std::string_view arg1,
                                                       uint64_t arg2)
{
    std::string arg2Str = std::to_string(arg2);
    return getLog(redfish::registries::ResourceEvent::Index::
                      resourceErrorThresholdCleared,
                  std::to_array<std::string_view>({arg1, arg2Str}));
}

/**
 * @internal
 * @brief Formats ResourceWarningThresholdExceeded message into JSON
 *
 * See header file for more information
 * @endinternal
 */
/* @internal
 * @brief 将 ResourceWarningThresholdExceeded（超出资源警告阈值）消息格式化为 JSON
 * 更多信息请参见头文件
 * @endinternal */
nlohmann::json::object_t resourceWarningThresholdExceeded(std::string_view arg1,
                                                          uint64_t arg2)
{
    std::string arg2Str = std::to_string(arg2);
    return getLog(redfish::registries::ResourceEvent::Index::
                      resourceWarningThresholdExceeded,
                  std::to_array<std::string_view>({arg1, arg2Str}));
}

/**
 * @internal
 * @brief Formats ResourceWarningThresholdCleared message into JSON
 *
 * See header file for more information
 * @endinternal
 */
/* @internal
 * @brief 将 ResourceWarningThresholdCleared（资源警告阈值已清除）消息格式化为 JSON
 * 更多信息请参见头文件
 * @endinternal */
nlohmann::json::object_t resourceWarningThresholdCleared(std::string_view arg1,
                                                         uint64_t arg2)
{
    std::string arg2Str = std::to_string(arg2);
    return getLog(redfish::registries::ResourceEvent::Index::
                      resourceWarningThresholdCleared,
                  std::to_array<std::string_view>({arg1, arg2Str}));
}

/**
 * @internal
 * @brief Formats ResourceStatusChangedOK message into JSON
 *
 * See header file for more information
 * @endinternal
 */
/* @internal
 * @brief 将 ResourceStatusChangedOK（资源状态变更为 OK）消息格式化为 JSON
 * 更多信息请参见头文件
 * @endinternal */
nlohmann::json::object_t resourceStatusChangedOK(std::string_view arg1,
                                                 std::string_view arg2)
{
    return getLog(
        redfish::registries::ResourceEvent::Index::resourceStatusChangedOK,
        std::to_array({arg1, arg2}));
}

/**
 * @internal
 * @brief Formats ResourceStatusChangedWarning message into JSON
 *
 * See header file for more information
 * @endinternal
 */
/* @internal
 * @brief 将 ResourceStatusChangedWarning（资源状态变更为 Warning）消息格式化为 JSON
 * 更多信息请参见头文件
 * @endinternal */
nlohmann::json::object_t resourceStatusChangedWarning(std::string_view arg1,
                                                      std::string_view arg2)
{
    return getLog(
        redfish::registries::ResourceEvent::Index::resourceStatusChangedWarning,
        std::to_array({arg1, arg2}));
}

/**
 * @internal
 * @brief Formats ResourceStatusChangedCritical message into JSON
 *
 * See header file for more information
 * @endinternal
 */
/* @internal
 * @brief 将 ResourceStatusChangedCritical（资源状态变更为 Critical）消息格式化为 JSON
 * 更多信息请参见头文件
 * @endinternal */
nlohmann::json::object_t resourceStatusChangedCritical(std::string_view arg1,
                                                       std::string_view arg2)
{
    return getLog(redfish::registries::ResourceEvent::Index::
                      resourceStatusChangedCritical,
                  std::to_array({arg1, arg2}));
}

/**
 * @internal
 * @brief Formats ResourceStateChanged message into JSON
 *
 * See header file for more information
 * @endinternal
 */
/* @internal
 * @brief 将 ResourceStateChanged（资源状态已改变）消息格式化为 JSON
 * 更多信息请参见头文件
 * @endinternal */
nlohmann::json::object_t resourceStateChanged(std::string_view arg1,
                                              std::string_view arg2)
{
    return getLog(
        redfish::registries::ResourceEvent::Index::resourceStateChanged,
        std::to_array({arg1, arg2}));
}

/**
 * @internal
 * @brief Formats ResourcePoweredOn message into JSON
 *
 * See header file for more information
 * @endinternal
 */
/* @internal
 * @brief 将 ResourcePoweredOn（资源已上电）消息格式化为 JSON
 * 更多信息请参见头文件
 * @endinternal */
nlohmann::json::object_t resourcePoweredOn(std::string_view arg1)
{
    return getLog(redfish::registries::ResourceEvent::Index::resourcePoweredOn,
                  std::to_array({arg1}));
}

/**
 * @internal
 * @brief Formats ResourcePoweringOn message into JSON
 *
 * See header file for more information
 * @endinternal
 */
/* @internal
 * @brief 将 ResourcePoweringOn（资源正在上电）消息格式化为 JSON
 * 更多信息请参见头文件
 * @endinternal */
nlohmann::json::object_t resourcePoweringOn(std::string_view arg1)
{
    return getLog(redfish::registries::ResourceEvent::Index::resourcePoweringOn,
                  std::to_array({arg1}));
}

/**
 * @internal
 * @brief Formats ResourcePoweredOff message into JSON
 *
 * See header file for more information
 * @endinternal
 */
/* @internal
 * @brief 将 ResourcePoweredOff（资源已断电）消息格式化为 JSON
 * 更多信息请参见头文件
 * @endinternal */
nlohmann::json::object_t resourcePoweredOff(std::string_view arg1)
{
    return getLog(redfish::registries::ResourceEvent::Index::resourcePoweredOff,
                  std::to_array({arg1}));
}

/**
 * @internal
 * @brief Formats ResourcePoweringOff message into JSON
 *
 * See header file for more information
 * @endinternal
 */
/* @internal
 * @brief 将 ResourcePoweringOff（资源正在断电）消息格式化为 JSON
 * 更多信息请参见头文件
 * @endinternal */
nlohmann::json::object_t resourcePoweringOff(std::string_view arg1)
{
    return getLog(
        redfish::registries::ResourceEvent::Index::resourcePoweringOff,
        std::to_array({arg1}));
}

/**
 * @internal
 * @brief Formats ResourcePaused message into JSON
 *
 * See header file for more information
 * @endinternal
 */
/* @internal
 * @brief 将 ResourcePaused（资源已暂停）消息格式化为 JSON
 * 更多信息请参见头文件
 * @endinternal */
nlohmann::json::object_t resourcePaused(std::string_view arg1)
{
    return getLog(redfish::registries::ResourceEvent::Index::resourcePaused,
                  std::to_array({arg1}));
}

/**
 * @internal
 * @brief Formats URIForResourceChanged message into JSON
 *
 * See header file for more information
 * @endinternal
 */
/* @internal
 * @brief 将 URIForResourceChanged（资源 URI 已更改）消息格式化为 JSON
 * 更多信息请参见头文件
 * @endinternal */
nlohmann::json::object_t uRIForResourceChanged()
{
    return getLog(
        redfish::registries::ResourceEvent::Index::uRIForResourceChanged, {});
}

/**
 * @internal
 * @brief Formats ResourceChanged message into JSON
 *
 * See header file for more information
 * @endinternal
 */
/* @internal
 * @brief 将 ResourceChanged（资源已更改）消息格式化为 JSON
 * 更多信息请参见头文件
 * @endinternal */
nlohmann::json::object_t resourceChanged()
{
    return getLog(redfish::registries::ResourceEvent::Index::resourceChanged,
                  {});
}

/**
 * @internal
 * @brief Formats ResourceVersionIncompatible message into JSON
 *
 * See header file for more information
 * @endinternal
 */
/* @internal
 * @brief 将 ResourceVersionIncompatible（资源版本不兼容）消息格式化为 JSON
 * 更多信息请参见头文件
 * @endinternal */
nlohmann::json::object_t resourceVersionIncompatible(std::string_view arg1)
{
    return getLog(
        redfish::registries::ResourceEvent::Index::resourceVersionIncompatible,
        std::to_array({arg1}));
}

/**
 * @internal
 * @brief Formats ResourceSelfTestFailed message into JSON
 *
 * See header file for more information
 * @endinternal
 */
/* @internal
 * @brief 将 ResourceSelfTestFailed（资源自检失败）消息格式化为 JSON
 * 更多信息请参见头文件
 * @endinternal */
nlohmann::json::object_t resourceSelfTestFailed(std::string_view arg1)
{
    return getLog(
        redfish::registries::ResourceEvent::Index::resourceSelfTestFailed,
        std::to_array({arg1}));
}

/**
 * @internal
 * @brief Formats ResourceSelfTestCompleted message into JSON
 *
 * See header file for more information
 * @endinternal
 */
/* @internal
 * @brief 将 ResourceSelfTestCompleted（资源自检完成）消息格式化为 JSON
 * 更多信息请参见头文件
 * @endinternal */
nlohmann::json::object_t resourceSelfTestCompleted()
{
    return getLog(
        redfish::registries::ResourceEvent::Index::resourceSelfTestCompleted,
        {});
}

/**
 * @internal
 * @brief Formats TestMessage message into JSON
 *
 * See header file for more information
 * @endinternal
 */
/* @internal
 * @brief 将 TestMessage（测试消息）消息格式化为 JSON
 * 更多信息请参见头文件
 * @endinternal */
nlohmann::json::object_t testMessage()
{
    return getLog(redfish::registries::ResourceEvent::Index::testMessage, {});
}

/**
 * @internal
 * @brief Formats AggregationSourceDiscovered message into JSON
 *
 * See header file for more information
 * @endinternal
 */
/* @internal
 * @brief 将 AggregationSourceDiscovered（发现聚合源）消息格式化为 JSON
 * 更多信息请参见头文件
 * @endinternal */
nlohmann::json::object_t aggregationSourceDiscovered(std::string_view arg1,
                                                     std::string_view arg2)
{
    return getLog(
        redfish::registries::ResourceEvent::Index::aggregationSourceDiscovered,
        std::to_array({arg1, arg2}));
}

/**
 * @internal
 * @brief Formats LicenseExpired message into JSON
 *
 * See header file for more information
 * @endinternal
 */
/* @internal
 * @brief 将 LicenseExpired（许可证已过期）消息格式化为 JSON
 * 更多信息请参见头文件
 * @endinternal */
nlohmann::json::object_t licenseExpired(std::string_view arg1,
                                        std::string_view arg2)
{
    return getLog(redfish::registries::ResourceEvent::Index::licenseExpired,
                  std::to_array({arg1, arg2}));
}

/**
 * @internal
 * @brief Formats LicenseChanged message into JSON
 *
 * See header file for more information
 * @endinternal
 */
/* @internal
 * @brief 将 LicenseChanged（许可证已更改）消息格式化为 JSON
 * 更多信息请参见头文件
 * @endinternal */
nlohmann::json::object_t licenseChanged(std::string_view arg1,
                                        std::string_view arg2)
{
    return getLog(redfish::registries::ResourceEvent::Index::licenseChanged,
                  std::to_array({arg1, arg2}));
}

/**
 * @internal
 * @brief Formats LicenseAdded message into JSON
 *
 * See header file for more information
 * @endinternal
 */
/* @internal
 * @brief 将 LicenseAdded（许可证已添加）消息格式化为 JSON
 * 更多信息请参见头文件
 * @endinternal */
nlohmann::json::object_t licenseAdded(std::string_view arg1,
                                      std::string_view arg2)
{
    return getLog(redfish::registries::ResourceEvent::Index::licenseAdded,
                  std::to_array({arg1, arg2}));
}

} // namespace messages
} // namespace redfish
