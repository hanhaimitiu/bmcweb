// SPDX-License-Identifier: Apache-2.0
// SPDX-FileCopyrightText: Copyright OpenBMC Authors
// SPDX-FileCopyrightText: Copyright 2018 Intel Corporation
#pragma once

#include "bmcweb_config.h"

#include "app.hpp"
#include "async_resp.hpp"
#include "dbus_singleton.hpp"
#include "dbus_utility.hpp"
#include "error_messages.hpp"
#include "generated/enums/action_info.hpp"
#include "generated/enums/computer_system.hpp"
#include "generated/enums/open_bmc_computer_system.hpp"
#include "generated/enums/resource.hpp"
#include "http_request.hpp"
#include "hypervisor_system.hpp"
#include "led.hpp"
#include "logging.hpp"
#include "query.hpp"
#include "redfish_util.hpp"
#include "registries/privilege_registry.hpp"
#include "utils/asset_utils.hpp"
#include "utils/dbus_utils.hpp"
#include "utils/json_utils.hpp"
#include "utils/pcie_util.hpp"
#include "utils/sw_utils.hpp"
#include "utils/systems_utils.hpp"
#include "utils/time_utils.hpp"

#include <asm-generic/errno.h>

#include <boost/asio/error.hpp>
#include <boost/beast/http/field.hpp>
#include <boost/beast/http/verb.hpp>
#include <boost/system/error_code.hpp>
#include <boost/system/linux_error.hpp>
#include <boost/url/format.hpp>
#include <nlohmann/json.hpp>
#include <sdbusplus/message/native_types.hpp>
#include <sdbusplus/unpack_properties.hpp>

#include <array>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <ratio>
#include <string>
#include <string_view>
#include <tuple>
#include <utility>
#include <vector>

namespace redfish
{

const static std::array<std::pair<std::string_view, std::string_view>, 2>
    protocolToDBusForSystems{
        {{"SSH", "obmc-console-ssh"}, {"IPMI", "phosphor-ipmi-net"}}};

/*
 * @brief Update "ProcessorSummary" "Count" based on Cpu PresenceState
 *
 * @param[in] asyncResp Shared pointer for completing asynchronous calls
 * @param[in] cpuPresenceState CPU present or not
 *
 * @return None.
 */
/* @brief 根据 CPU PresenceState 更新 "ProcessorSummary" "Count"。
 * @param[in] asyncResp 用于完成异步调用的共享指针。
 * @param[in] cpuPresenceState CPU 是否存在。
 * @return 无。 */
inline void modifyCpuPresenceState(
    const std::shared_ptr<bmcweb::AsyncResp>& asyncResp, bool isCpuPresent)
{
    BMCWEB_LOG_DEBUG("Cpu Present: {}", isCpuPresent);

    if (isCpuPresent)
    {
        nlohmann::json& procCount =
            asyncResp->res.jsonValue["ProcessorSummary"]["Count"];
        auto* procCountPtr =
            procCount.get_ptr<nlohmann::json::number_integer_t*>();
        if (procCountPtr != nullptr)
        {
            // shouldn't be possible to be nullptr
            // 不应该是 nullptr。
            *procCountPtr += 1;
        }
    }
}

inline void getProcessorProperties(
    const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
    const std::vector<std::pair<std::string, dbus::utility::DbusVariantType>>&
        properties)
{
    BMCWEB_LOG_DEBUG("Got {} Cpu properties.", properties.size());

    // TODO: Get Model
    // TODO：获取型号（Model）

    const uint16_t* coreCount = nullptr;

    const bool success = sdbusplus::unpackPropertiesNoThrow(
        dbus_utils::UnpackErrorPrinter(), properties, "CoreCount", coreCount);

    if (!success)
    {
        messages::internalError(asyncResp->res);
        return;
    }

    if (coreCount != nullptr)
    {
        nlohmann::json& coreCountJson =
            asyncResp->res.jsonValue["ProcessorSummary"]["CoreCount"];
        uint64_t* coreCountJsonPtr = coreCountJson.get_ptr<uint64_t*>();

        if (coreCountJsonPtr == nullptr)
        {
            coreCountJson = *coreCount;
        }
        else
        {
            *coreCountJsonPtr += *coreCount;
        }
    }
}

/*
 * @brief Get ProcessorSummary fields
 *
 * @param[in] asyncResp Shared pointer for completing asynchronous calls
 * @param[in] service dbus service for Cpu Information
 * @param[in] path dbus path for Cpu
 *
 * @return None.
 */
/* @brief 获取 ProcessorSummary（处理器摘要）字段
 * @param[in] asyncResp 用于完成异步调用的共享指针
 * @param[in] service 提供 CPU 信息的 dbus 服务
 * @param[in] path CPU 的 dbus 路径
 * @return 无。 */
inline void getProcessorSummary(
    const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
    const std::string& service, const std::string& path)
{
    auto getCpuPresenceState = [asyncResp](const boost::system::error_code& ec3,
                                           const bool cpuPresenceCheck) {
        if (ec3)
        {
            BMCWEB_LOG_ERROR("DBUS response error {}", ec3);
            return;
        }
        modifyCpuPresenceState(asyncResp, cpuPresenceCheck);
    };

    // Get the Presence of CPU
    // 获取 CPU 的在位状态（Presence）
    dbus::utility::getProperty<bool>(*crow::connections::systemBus, service,
                                     path, "xyz.openbmc_project.Inventory.Item",
                                     "Present", std::move(getCpuPresenceState));

    dbus::utility::getAllProperties(
        service, path, "xyz.openbmc_project.Inventory.Item.Cpu",
        // ast-grep-ignore: long-lambda
        // ast-grep-ignore: long-lambda（ast-grep 工具指令：忽略过长的 lambda 表达式）
        [asyncResp, service,
         path](const boost::system::error_code& ec2,
               const dbus::utility::DBusPropertiesMap& properties) {
            if (ec2)
            {
                BMCWEB_LOG_ERROR("DBUS response error {}", ec2);
                messages::internalError(asyncResp->res);
                return;
            }
            getProcessorProperties(asyncResp, properties);
        });
}

/*
 * @brief processMemoryProperties fields
 *
 * @param[in] asyncResp Shared pointer for completing asynchronous calls
 * @param[in] DBUS properties for memory
 *
 * @return None.
 */
/* @brief 处理 processMemoryProperties（内存属性）字段
 * @param[in] asyncResp 用于完成异步调用的共享指针
 * @param[in] DBUS 内存的属性
 * @return 无。 */
inline void processMemoryProperties(
    const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
    const dbus::utility::DBusPropertiesMap& properties)
{
    BMCWEB_LOG_DEBUG("Got {} Dimm properties.", properties.size());

    if (properties.empty())
    {
        return;
    }

    const size_t* memorySizeInKB = nullptr;

    const bool success = sdbusplus::unpackPropertiesNoThrow(
        dbus_utils::UnpackErrorPrinter(), properties, "MemorySizeInKB",
        memorySizeInKB);

    if (!success)
    {
        messages::internalError(asyncResp->res);
        return;
    }

    if (memorySizeInKB != nullptr)
    {
        nlohmann::json& totalMemory =
            asyncResp->res.jsonValue["MemorySummary"]["TotalSystemMemoryGiB"];
        const double* preValue = totalMemory.get_ptr<const double*>();
        if (preValue == nullptr)
        {
            asyncResp->res.jsonValue["MemorySummary"]["TotalSystemMemoryGiB"] =
                static_cast<double>(*memorySizeInKB) / (1024 * 1024);
        }
        else
        {
            asyncResp->res.jsonValue["MemorySummary"]["TotalSystemMemoryGiB"] =
                static_cast<double>(*memorySizeInKB) / (1024 * 1024) +
                *preValue;
        }
    }
}

/*
 * @brief Get getMemorySummary fields
 *
 * @param[in] asyncResp Shared pointer for completing asynchronous calls
 * @param[in] service dbus service for memory Information
 * @param[in] path dbus path for memory
 *
 * @return None.
 */
/* @brief 获取 getMemorySummary（内存摘要）字段
 * @param[in] asyncResp 用于完成异步调用的共享指针
 * @param[in] service 提供内存信息的 dbus 服务
 * @param[in] path 内存的 dbus 路径
 * @return 无。 */
inline void getMemorySummary(
    const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
    const std::string& service, const std::string& path)
{
    dbus::utility::getAllProperties(
        service, path, "xyz.openbmc_project.Inventory.Item.Dimm",
        // ast-grep-ignore: long-lambda
        // ast-grep-ignore: long-lambda（ast-grep 工具指令：忽略过长的 lambda 表达式）
        [asyncResp, service,
         path](const boost::system::error_code& ec2,
               const dbus::utility::DBusPropertiesMap& properties) {
            if (ec2)
            {
                BMCWEB_LOG_ERROR("DBUS response error {}", ec2);
                messages::internalError(asyncResp->res);
                return;
            }
            processMemoryProperties(asyncResp, properties);
        });
}

inline void afterGetUUID(const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
                         const boost::system::error_code& ec,
                         const dbus::utility::DBusPropertiesMap& properties)
{
    if (ec)
    {
        BMCWEB_LOG_ERROR("DBUS response error {}", ec);
        messages::internalError(asyncResp->res);
        return;
    }
    BMCWEB_LOG_DEBUG("Got {} UUID properties.", properties.size());

    const std::string* uuid = nullptr;

    const bool success = sdbusplus::unpackPropertiesNoThrow(
        dbus_utils::UnpackErrorPrinter(), properties, "UUID", uuid);

    if (!success)
    {
        messages::internalError(asyncResp->res);
        return;
    }

    if (uuid != nullptr && !uuid->empty())
    {
        std::string valueStr = *uuid;
        if (valueStr.size() == 32)
        {
            valueStr.insert(8, 1, '-');
            valueStr.insert(13, 1, '-');
            valueStr.insert(18, 1, '-');
            valueStr.insert(23, 1, '-');
        }
        BMCWEB_LOG_DEBUG("UUID = {}", valueStr);
        asyncResp->res.jsonValue["UUID"] = valueStr;
    }
}

inline void afterGetInventory(
    const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
    const boost::system::error_code& ec,
    const dbus::utility::DBusPropertiesMap& propertiesList)
{
    if (ec)
    {
        // doesn't have to include this
        // interface
        // 不必包含此
        // 接口
        return;
    }
    BMCWEB_LOG_DEBUG("Got {} properties for system", propertiesList.size());

    asset_utils::extractAssetInfo(asyncResp, ""_json_pointer, propertiesList,
                                  false);

    const std::string* subModel = nullptr;

    const bool success = sdbusplus::unpackPropertiesNoThrow(
        dbus_utils::UnpackErrorPrinter(), propertiesList, "SubModel", subModel);

    if (!success)
    {
        messages::internalError(asyncResp->res);
        return;
    }

    if (subModel != nullptr && !subModel->empty())
    {
        asyncResp->res.jsonValue["SubModel"] = *subModel;
    }

    // Grab the bios version
    // 获取 BIOS 版本
    sw_util::populateSoftwareInformation(asyncResp, sw_util::biosPurpose,
                                         "BiosVersion", false);
}

inline void afterGetAssetTag(
    const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
    const boost::system::error_code& ec, const std::string& value)
{
    if (ec)
    {
        // doesn't have to include this
        // interface
        // 不必包含此
        // 接口
        return;
    }

    asyncResp->res.jsonValue["AssetTag"] = value;
}

inline void afterSystemGetSubTree(
    const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
    const boost::system::error_code& ec,
    const dbus::utility::MapperGetSubTreeResponse& subtree)
{
    if (ec)
    {
        BMCWEB_LOG_ERROR("DBUS response error {}", ec);
        messages::internalError(asyncResp->res);
        return;
    }
    // Iterate over all retrieved ObjectPaths.
    // 遍历所有获取到的 ObjectPath（对象路径）。
    for (const std::pair<
             std::string,
             std::vector<std::pair<std::string, std::vector<std::string>>>>&
             object : subtree)
    {
        const std::string& path = object.first;
        BMCWEB_LOG_DEBUG("Got path: {}", path);
        const std::vector<std::pair<std::string, std::vector<std::string>>>&
            connectionNames = object.second;
        if (connectionNames.empty())
        {
            continue;
        }

        // This is not system, so check if it's cpu, dimm, UUID or
        // BiosVer
        // 这不是系统对象，因此检查它是 CPU、DIMM、UUID 还是
        // BiosVer（BIOS 版本）
        for (const auto& connection : connectionNames)
        {
            for (const auto& interfaceName : connection.second)
            {
                if (interfaceName == "xyz.openbmc_project.Inventory.Item.Dimm")
                {
                    BMCWEB_LOG_DEBUG("Found Dimm, now get its properties.");

                    getMemorySummary(asyncResp, connection.first, path);
                }
                else if (interfaceName ==
                         "xyz.openbmc_project.Inventory.Item.Cpu")
                {
                    BMCWEB_LOG_DEBUG("Found Cpu, now get its properties.");

                    getProcessorSummary(asyncResp, connection.first, path);
                }
                else if (interfaceName == "xyz.openbmc_project.Common.UUID")
                {
                    BMCWEB_LOG_DEBUG("Found UUID, now get its properties.");

                    dbus::utility::getAllProperties(
                        *crow::connections::systemBus, connection.first, path,
                        "xyz.openbmc_project.Common.UUID",
                        [asyncResp](const boost::system::error_code& ec3,
                                    const dbus::utility::DBusPropertiesMap&
                                        properties) {
                            afterGetUUID(asyncResp, ec3, properties);
                        });
                }
                else if (interfaceName ==
                         "xyz.openbmc_project.Inventory.Item.System")
                {
                    dbus::utility::getAllProperties(
                        *crow::connections::systemBus, connection.first, path,
                        "xyz.openbmc_project.Inventory.Decorator.Asset",
                        [asyncResp](const boost::system::error_code& ec3,
                                    const dbus::utility::DBusPropertiesMap&
                                        properties) {
                            afterGetInventory(asyncResp, ec3, properties);
                        });

                    dbus::utility::getProperty<std::string>(
                        connection.first, path,
                        "xyz.openbmc_project.Inventory.Decorator."
                        "AssetTag",
                        "AssetTag",
                        std::bind_front(afterGetAssetTag, asyncResp));
                }
            }
        }
    }
}

/*
 * @brief Retrieves computer system properties over dbus
 *
 * @param[in] asyncResp Shared pointer for completing asynchronous calls
 *
 * @return None.
 */
/* @brief 通过 dbus 检索计算机系统属性
 * @param[in] asyncResp 用于完成异步调用的共享指针
 * @return 无。 */
inline void getComputerSystem(
    const std::shared_ptr<bmcweb::AsyncResp>& asyncResp)
{
    BMCWEB_LOG_DEBUG("Get available system components.");
    constexpr std::array<std::string_view, 5> interfaces = {
        "xyz.openbmc_project.Inventory.Decorator.Asset",
        "xyz.openbmc_project.Inventory.Item.Cpu",
        "xyz.openbmc_project.Inventory.Item.Dimm",
        "xyz.openbmc_project.Inventory.Item.System",
        "xyz.openbmc_project.Common.UUID",
    };
    dbus::utility::getSubTree(
        "/xyz/openbmc_project/inventory", 0, interfaces,
        std::bind_front(afterSystemGetSubTree, asyncResp));
}

/**
 * @brief Retrieves host state properties over dbus
 *
 * @param[in] asyncResp     Shared pointer for completing asynchronous calls.
 * @param[in] computerSystemIndex Index associated with the requested system
 *
 * @return None.
 */
/* @brief 通过 dbus 检索主机状态属性
 * @param[in] asyncResp     用于完成异步调用的共享指针。
 * @param[in] computerSystemIndex 与所请求系统关联的索引
 * @return 无。 */
inline void getHostState(const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
                         const uint64_t computerSystemIndex)
{
    BMCWEB_LOG_DEBUG("Get host information.");
    sdbusplus::object_path path =
        systems_utils::getHostStateObjectPath(computerSystemIndex);
    dbus::utility::getProperty<std::string>(
        systems_utils::getHostStateServiceName(computerSystemIndex), path,
        "xyz.openbmc_project.State.Host", "CurrentHostState",
        // ast-grep-ignore: long-lambda
        // ast-grep-ignore: long-lambda（ast-grep 工具指令：忽略过长的 lambda 表达式）
        [asyncResp](const boost::system::error_code& ec,
                    const std::string& hostState) {
            if (ec)
            {
                if (ec == boost::system::errc::host_unreachable)
                {
                    // Service not available, no error, just don't return
                    // host state info
                    // 服务不可用，并非错误，只是不返回
                    // 主机状态信息
                    BMCWEB_LOG_DEBUG("Service not available {}", ec);
                    return;
                }
                BMCWEB_LOG_ERROR("DBUS response error {}", ec);
                messages::internalError(asyncResp->res);
                return;
            }

            BMCWEB_LOG_DEBUG("Host state: {}", hostState);
            // Verify Host State
            // 校验主机状态
            if (hostState == "xyz.openbmc_project.State.Host.HostState.Running")
            {
                asyncResp->res.jsonValue["PowerState"] =
                    resource::PowerState::On;
                asyncResp->res.jsonValue["Status"]["State"] =
                    resource::State::Enabled;
            }
            else if (hostState ==
                     "xyz.openbmc_project.State.Host.HostState.Quiesced")
            {
                asyncResp->res.jsonValue["PowerState"] =
                    resource::PowerState::On;
                asyncResp->res.jsonValue["Status"]["State"] =
                    resource::State::Quiesced;
            }
            else if (hostState ==
                     "xyz.openbmc_project.State.Host.HostState.DiagnosticMode")
            {
                asyncResp->res.jsonValue["PowerState"] =
                    resource::PowerState::On;
                asyncResp->res.jsonValue["Status"]["State"] =
                    resource::State::InTest;
            }
            else if (
                hostState ==
                "xyz.openbmc_project.State.Host.HostState.TransitioningToRunning")
            {
                asyncResp->res.jsonValue["PowerState"] =
                    resource::PowerState::PoweringOn;
                asyncResp->res.jsonValue["Status"]["State"] =
                    resource::State::Starting;
            }
            else if (
                hostState ==
                "xyz.openbmc_project.State.Host.HostState.TransitioningToOff")
            {
                asyncResp->res.jsonValue["PowerState"] =
                    resource::PowerState::PoweringOff;
                asyncResp->res.jsonValue["Status"]["State"] =
                    resource::State::Disabled;
            }
            else
            {
                asyncResp->res.jsonValue["PowerState"] =
                    resource::PowerState::Off;
                asyncResp->res.jsonValue["Status"]["State"] =
                    resource::State::Disabled;
            }
        });
}

/**
 * @brief Translates boot source DBUS property value to redfish.
 *
 * @param[in] dbusSource    The boot source in DBUS speak.
 *
 * @return Returns the boot source in Redfish terms. If translation
 * cannot be done, returns computer_system::BootSource::Invalid.
 */
/* @brief 将启动源（boot source）的 DBUS 属性值转换为 Redfish 值。
 * @param[in] dbusSource    DBUS 术语下的启动源。
 * @return 返回 Redfish 术语下的启动源；如果无法
 * 转换，则返回 computer_system::BootSource::Invalid。 */
inline computer_system::BootSource dbusToRfBootSource(
    const std::string& dbusSource)
{
    if (dbusSource == "xyz.openbmc_project.Control.Boot.Source.Sources.Default")
    {
        return computer_system::BootSource::None;
    }
    if (dbusSource == "xyz.openbmc_project.Control.Boot.Source.Sources.Disk")
    {
        return computer_system::BootSource::Hdd;
    }
    if (dbusSource ==
        "xyz.openbmc_project.Control.Boot.Source.Sources.ExternalMedia")
    {
        return computer_system::BootSource::Cd;
    }
    if (dbusSource == "xyz.openbmc_project.Control.Boot.Source.Sources.Network")
    {
        return computer_system::BootSource::Pxe;
    }
    if (dbusSource ==
        "xyz.openbmc_project.Control.Boot.Source.Sources.RemovableMedia")
    {
        return computer_system::BootSource::Usb;
    }
    return computer_system::BootSource::Invalid;
}

/**
 * @brief Translates boot type DBUS property value to redfish.
 *
 * @param[in] dbusType    The boot type in DBUS speak.
 *
 * @return Returns the boot type in Redfish terms. If translation
 * cannot be done, returns computer_system::BootSourceOverrideMode::Invalid.
 */
/* @brief 将启动类型（boot type）的 DBUS 属性值转换为 Redfish 值。
 * @param[in] dbusType    DBUS 术语下的启动类型。
 * @return 返回 Redfish 术语下的启动类型；如果无法
 * 转换，则返回 computer_system::BootSourceOverrideMode::Invalid。 */
inline computer_system::BootSourceOverrideMode dbusToRfBootType(
    const std::string& dbusType)
{
    if (dbusType == "xyz.openbmc_project.Control.Boot.Type.Types.Legacy")
    {
        return computer_system::BootSourceOverrideMode::Legacy;
    }
    if (dbusType == "xyz.openbmc_project.Control.Boot.Type.Types.EFI")
    {
        return computer_system::BootSourceOverrideMode::UEFI;
    }
    return computer_system::BootSourceOverrideMode::Invalid;
}

/**
 * @brief Translates boot mode DBUS property value to redfish.
 *
 * @param[in] dbusMode    The boot mode in DBUS speak.
 *
 * @return Returns the boot mode in Redfish terms. If translation
 * cannot be done, returns computer_system::BootSource::Invalid.
 */
/* @brief 将启动模式（boot mode）的 DBUS 属性值转换为 Redfish 值。
 * @param[in] dbusMode    DBUS 术语下的启动模式。
 * @return 返回 Redfish 术语下的启动模式；如果无法
 * 转换，则返回 computer_system::BootSource::Invalid。 */
inline computer_system::BootSource dbusToRfBootMode(const std::string& dbusMode)
{
    if (dbusMode == "xyz.openbmc_project.Control.Boot.Mode.Modes.Regular")
    {
        return computer_system::BootSource::None;
    }
    if (dbusMode == "xyz.openbmc_project.Control.Boot.Mode.Modes.Safe")
    {
        return computer_system::BootSource::Diags;
    }
    if (dbusMode == "xyz.openbmc_project.Control.Boot.Mode.Modes.Setup")
    {
        return computer_system::BootSource::BiosSetup;
    }
    return computer_system::BootSource::Invalid;
}

/**
 * @brief Translates boot progress DBUS property value to redfish.
 *
 * @param[in] dbusBootProgress    The boot progress in DBUS speak.
 *
 * @return Returns the boot progress in Redfish terms. If translation
 *         cannot be done, returns computer_system::BootProgressTypes::Invalid.
 */
/* @brief 将启动进度（boot progress）的 DBUS 属性值转换为 Redfish 值。
 * @param[in] dbusBootProgress    DBUS 术语下的启动进度。
 * @return 返回 Redfish 术语下的启动进度；如果无法
 *         转换，则返回 computer_system::BootProgressTypes::Invalid。 */
inline computer_system::BootProgressTypes dbusToRfBootProgress(
    const std::string& dbusBootProgress)
{
    // Now convert the D-Bus BootProgress to the appropriate Redfish
    // enum
    // 现在将 D-Bus 的 BootProgress 转换为相应的 Redfish
    // 枚举
    if (dbusBootProgress == "xyz.openbmc_project.State.Boot.Progress."
                            "ProgressStages.Unspecified")
    {
        return computer_system::BootProgressTypes::None;
    }
    if (dbusBootProgress ==
        "xyz.openbmc_project.State.Boot.Progress.ProgressStages."
        "PrimaryProcInit")
    {
        return computer_system::BootProgressTypes::
            PrimaryProcessorInitializationStarted;
    }
    if (dbusBootProgress ==
        "xyz.openbmc_project.State.Boot.Progress.ProgressStages."
        "BusInit")
    {
        return computer_system::BootProgressTypes::BusInitializationStarted;
    }
    if (dbusBootProgress ==
        "xyz.openbmc_project.State.Boot.Progress.ProgressStages."
        "MemoryInit")
    {
        return computer_system::BootProgressTypes::MemoryInitializationStarted;
    }
    if (dbusBootProgress ==
        "xyz.openbmc_project.State.Boot.Progress.ProgressStages."
        "SecondaryProcInit")
    {
        return computer_system::BootProgressTypes::
            SecondaryProcessorInitializationStarted;
    }
    if (dbusBootProgress ==
        "xyz.openbmc_project.State.Boot.Progress.ProgressStages."
        "PCIInit")
    {
        return computer_system::BootProgressTypes::PCIResourceConfigStarted;
    }
    if (dbusBootProgress ==
        "xyz.openbmc_project.State.Boot.Progress.ProgressStages."
        "SystemSetup")
    {
        return computer_system::BootProgressTypes::SetupEntered;
    }
    if (dbusBootProgress ==
        "xyz.openbmc_project.State.Boot.Progress.ProgressStages."
        "SystemInitComplete")
    {
        return computer_system::BootProgressTypes::
            SystemHardwareInitializationComplete;
    }
    if (dbusBootProgress ==
        "xyz.openbmc_project.State.Boot.Progress.ProgressStages."
        "OSStart")
    {
        return computer_system::BootProgressTypes::OSBootStarted;
    }
    if (dbusBootProgress ==
        "xyz.openbmc_project.State.Boot.Progress.ProgressStages."
        "OSRunning")
    {
        return computer_system::BootProgressTypes::OSRunning;
    }
    return computer_system::BootProgressTypes::Invalid;
}

/**
 * @brief Translates boot source from Redfish to the DBus boot paths.
 *
 * @param[in] rfSource    The boot source in Redfish.
 * @param[out] bootSource The DBus source
 * @param[out] bootMode   the DBus boot mode
 *
 * @return Integer error code.
 */
/* @brief 将启动源从 Redfish 转换为 DBus 的启动路径。
 * @param[in] rfSource    Redfish 中的启动源。
 * @param[out] bootSource DBus 的启动源
 * @param[out] bootMode   DBus 的启动模式
 * @return 整数错误码。 */
inline int assignBootParameters(const std::string& rfSource,
                                std::string& bootSource, std::string& bootMode)
{
    bootSource = "xyz.openbmc_project.Control.Boot.Source.Sources.Default";
    bootMode = "xyz.openbmc_project.Control.Boot.Mode.Modes.Regular";

    if (rfSource == "None")
    {
        return 0;
    }
    if (rfSource == "Pxe")
    {
        bootSource = "xyz.openbmc_project.Control.Boot.Source.Sources.Network";
    }
    else if (rfSource == "Hdd")
    {
        bootSource = "xyz.openbmc_project.Control.Boot.Source.Sources.Disk";
    }
    else if (rfSource == "Diags")
    {
        bootMode = "xyz.openbmc_project.Control.Boot.Mode.Modes.Safe";
    }
    else if (rfSource == "Cd")
    {
        bootSource =
            "xyz.openbmc_project.Control.Boot.Source.Sources.ExternalMedia";
    }
    else if (rfSource == "BiosSetup")
    {
        bootMode = "xyz.openbmc_project.Control.Boot.Mode.Modes.Setup";
    }
    else if (rfSource == "Usb")
    {
        bootSource =
            "xyz.openbmc_project.Control.Boot.Source.Sources.RemovableMedia";
    }
    else
    {
        BMCWEB_LOG_DEBUG(
            "Invalid property value for BootSourceOverrideTarget: {}",
            bootSource);
        return -1;
    }
    return 0;
}

/**
 * @brief Retrieves boot progress of the system
 *
 * @param[in] asyncResp  Shared pointer for generating response message.
 * @param[in] computerSystemIndex Index associated with the requested system
 *
 * @return None.
 */
/* @brief 检索系统的启动进度
 * @param[in] asyncResp  用于生成响应消息的共享指针。
 * @param[in] computerSystemIndex 与所请求系统关联的索引
 * @return 无。 */
inline void getBootProgress(const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
                            const uint64_t computerSystemIndex)
{
    sdbusplus::object_path path =
        systems_utils::getHostStateObjectPath(computerSystemIndex);
    dbus::utility::getProperty<std::string>(
        systems_utils::getHostStateServiceName(computerSystemIndex), path,
        "xyz.openbmc_project.State.Boot.Progress", "BootProgress",
        // ast-grep-ignore: long-lambda
        // ast-grep-ignore: long-lambda（ast-grep 工具指令：忽略过长的 lambda 表达式）
        [asyncResp](const boost::system::error_code ec,
                    const std::string& bootProgressStr) {
            if (ec)
            {
                // BootProgress is an optional object so just do nothing if
                // not found
                // BootProgress 是可选对象，如果
                // 未找到则不做任何处理
                return;
            }

            BMCWEB_LOG_DEBUG("Boot Progress: {}", bootProgressStr);

            computer_system::BootProgressTypes rfBootProgress =
                dbusToRfBootProgress(bootProgressStr);
            if (rfBootProgress != computer_system::BootProgressTypes::Invalid)
            {
                asyncResp->res.jsonValue["BootProgress"]["LastState"] =
                    rfBootProgress;
            }
        });
}

/**
 * @brief Retrieves boot progress Last Update of the system
 *
 * @param[in] asyncResp  Shared pointer for generating response message.
 * @param[in] computerSystemIndex Index associated with the requested system
 *
 * @return None.
 */
/* @brief 检索系统启动进度的最后更新时间（Last Update）
 * @param[in] asyncResp  用于生成响应消息的共享指针。
 * @param[in] computerSystemIndex 与所请求系统关联的索引
 * @return 无。 */
inline void getBootProgressLastStateTime(
    const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
    const uint64_t computerSystemIndex)
{
    sdbusplus::object_path path =
        systems_utils::getHostStateObjectPath(computerSystemIndex);
    dbus::utility::getProperty<uint64_t>(
        systems_utils::getHostStateServiceName(computerSystemIndex), path,
        "xyz.openbmc_project.State.Boot.Progress", "BootProgressLastUpdate",
        // ast-grep-ignore: long-lambda
        // ast-grep-ignore: long-lambda（ast-grep 工具指令：忽略过长的 lambda 表达式）
        [asyncResp](const boost::system::error_code& ec,
                    const uint64_t lastStateTime) {
            if (ec)
            {
                BMCWEB_LOG_DEBUG("D-BUS response error {}", ec);
                return;
            }

            // BootProgressLastUpdate is the last time the BootProgress property
            // was updated. The time is the Epoch time, number of microseconds
            // since 1 Jan 1970 00::00::00 UTC."
            // https://github.com/openbmc/phosphor-dbus-interfaces/blob/master/
            // yaml/xyz/openbmc_project/State/Boot/Progress.interface.yaml#L11
            // BootProgressLastUpdate 是 BootProgress 属性最后一次
            // 被更新的时间。该时间为 Epoch 时间，即自
            // 1970 年 1 月 1 日 00:00:00 UTC 以来的微秒数。"
            // https://github.com/openbmc/phosphor-dbus-interfaces/blob/master/
            // yaml/xyz/openbmc_project/State/Boot/Progress.interface.yaml#L11

            // Convert to ISO 8601 standard
            // 转换为 ISO 8601 标准格式
            asyncResp->res.jsonValue["BootProgress"]["LastStateTime"] =
                redfish::time_utils::getDateTimeUintUs(lastStateTime);
        });
}

/**
 * @brief Retrieves boot override type over DBUS and fills out the response
 *
 * @param[in] asyncResp         Shared pointer for generating response message.
 * @param[in] computerSystemIndex Index associated with the requested system
 *
 * @return None.
 */
/* @brief 通过 DBUS 检索启动覆盖类型并填充响应
 * @param[in] asyncResp         用于生成响应消息的共享指针。
 * @param[in] computerSystemIndex 与所请求系统关联的索引
 * @return 无。 */
inline void getBootOverrideType(
    const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
    const uint64_t computerSystemIndex)
{
    sdbusplus::object_path path("/xyz/openbmc_project/control/host" +
                                std::to_string(computerSystemIndex));
    path /= "boot";

    dbus::utility::getProperty<std::string>(
        "xyz.openbmc_project.Settings", path,
        "xyz.openbmc_project.Control.Boot.Type", "BootType",
        // ast-grep-ignore: long-lambda
        // ast-grep-ignore: long-lambda（ast-grep 工具指令：忽略过长的 lambda 表达式）
        [asyncResp](const boost::system::error_code& ec,
                    const std::string& bootType) {
            if (ec)
            {
                // not an error, don't have to have the interface
                // 这不是错误，不一定非要提供该接口
                return;
            }

            BMCWEB_LOG_DEBUG("Boot type: {}", bootType);

            asyncResp->res
                .jsonValue["Boot"]
                          ["BootSourceOverrideMode@Redfish.AllowableValues"] =
                nlohmann::json::array_t({"Legacy", "UEFI"});

            computer_system::BootSourceOverrideMode rfType =
                dbusToRfBootType(bootType);
            if (rfType != computer_system::BootSourceOverrideMode::Invalid)
            {
                asyncResp->res.jsonValue["Boot"]["BootSourceOverrideMode"] =
                    rfType;
            }
        });
}

/**
 * @brief Retrieves boot override mode over DBUS and fills out the response
 *
 * @param[in] asyncResp         Shared pointer for generating response message.
 * @param[in] computerSystemIndex Index associated with the requested system
 *
 * @return None.
 */
/* @brief 通过 DBUS 检索启动覆盖模式并填充响应
 * @param[in] asyncResp         用于生成响应消息的共享指针。
 * @param[in] computerSystemIndex 与所请求系统关联的索引
 * @return 无。 */
inline void getBootOverrideMode(
    const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
    const uint64_t computerSystemIndex)
{
    sdbusplus::object_path path("/xyz/openbmc_project/control/host" +
                                std::to_string(computerSystemIndex));
    path /= "boot";
    dbus::utility::getProperty<std::string>(
        "xyz.openbmc_project.Settings", path,
        "xyz.openbmc_project.Control.Boot.Mode", "BootMode",
        // ast-grep-ignore: long-lambda
        // ast-grep-ignore: long-lambda（ast-grep 工具指令：忽略过长的 lambda 表达式）
        [asyncResp](const boost::system::error_code& ec,
                    const std::string& bootModeStr) {
            if (ec)
            {
                BMCWEB_LOG_ERROR("DBUS response error {}", ec);
                messages::internalError(asyncResp->res);
                return;
            }

            BMCWEB_LOG_DEBUG("Boot mode: {}", bootModeStr);

            nlohmann::json::array_t allowed;
            allowed.emplace_back("None");
            allowed.emplace_back("Pxe");
            allowed.emplace_back("Hdd");
            allowed.emplace_back("Cd");
            allowed.emplace_back("Diags");
            allowed.emplace_back("BiosSetup");
            allowed.emplace_back("Usb");

            asyncResp->res
                .jsonValue["Boot"]
                          ["BootSourceOverrideTarget@Redfish.AllowableValues"] =
                std::move(allowed);
            if (bootModeStr !=
                "xyz.openbmc_project.Control.Boot.Mode.Modes.Regular")
            {
                computer_system::BootSource rfMode =
                    dbusToRfBootMode(bootModeStr);
                if (rfMode != computer_system::BootSource::Invalid)
                {
                    asyncResp->res
                        .jsonValue["Boot"]["BootSourceOverrideTarget"] = rfMode;
                }
            }
        });
}

/**
 * @brief Retrieves boot override source over DBUS
 *
 * @param[in] asyncResp         Shared pointer for generating response message.
 * @param[in] computerSystemIndex Index associated with the requested system
 *
 * @return None.
 */
/* @brief 通过 DBUS 检索启动覆盖源
 * @param[in] asyncResp         用于生成响应消息的共享指针。
 * @param[in] computerSystemIndex 与所请求系统关联的索引
 * @return 无。 */
inline void getBootOverrideSource(
    const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
    const uint64_t computerSystemIndex)
{
    sdbusplus::object_path path("/xyz/openbmc_project/control/host" +
                                std::to_string(computerSystemIndex));
    path /= "boot";

    dbus::utility::getProperty<std::string>(
        "xyz.openbmc_project.Settings", path,
        "xyz.openbmc_project.Control.Boot.Source", "BootSource",
        // ast-grep-ignore: long-lambda
        // ast-grep-ignore: long-lambda（ast-grep 工具指令：忽略过长的 lambda 表达式）
        [asyncResp, computerSystemIndex](const boost::system::error_code& ec,
                                         const std::string& bootSourceStr) {
            if (ec)
            {
                // Service not available, no error, just don't return
                // Boot Override Source information
                // 服务不可用，并非错误，只是不返回
                // 启动覆盖源（Boot Override Source）信息
                if (ec.value() != EBADR &&
                    ec.value() != boost::asio::error::host_unreachable)
                {
                    BMCWEB_LOG_ERROR("D-Bus response error: {}", ec);
                    messages::internalError(asyncResp->res);
                }
                return;
            }

            BMCWEB_LOG_DEBUG("Boot source: {}", bootSourceStr);

            auto rfSource = dbusToRfBootSource(bootSourceStr);
            if (rfSource != computer_system::BootSource::Invalid)
            {
                asyncResp->res.jsonValue["Boot"]["BootSourceOverrideTarget"] =
                    rfSource;
            }

            // Get BootMode as BootSourceOverrideTarget is constructed
            // from both BootSource and BootMode
            // 获取 BootMode，因为 BootSourceOverrideTarget 是由
            // BootSource 和 BootMode 共同构造的
            getBootOverrideMode(asyncResp, computerSystemIndex);
        });
}

/**
 * @brief This functions abstracts all the logic behind getting a
 * "BootSourceOverrideEnabled" property from an overall boot override enable
 * state
 *
 * @param[in] asyncResp     Shared pointer for generating response message.
 * @param[in] computerSystemIndex Index associated with the requested system
 * @param[in] bootOverrideEnableSetting
 *
 * @return None.
 */
/* @brief 本函数抽象了从一个整体的启动覆盖使能状态中获取
 * "BootSourceOverrideEnabled" 属性的全部逻辑
 * @param[in] asyncResp     用于生成响应消息的共享指针。
 * @param[in] computerSystemIndex 与所请求系统关联的索引
 * @param[in] bootOverrideEnable
 * @return 无。 */

inline void processBootOverrideEnable(
    const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
    const uint64_t computerSystemIndex, const bool bootOverrideEnableSetting)
{
    if (!bootOverrideEnableSetting)
    {
        asyncResp->res.jsonValue["Boot"]["BootSourceOverrideEnabled"] =
            "Disabled";
        return;
    }

    sdbusplus::object_path path("/xyz/openbmc_project/control/host" +
                                std::to_string(computerSystemIndex));
    path /= "boot";
    path /= "one_time";

    // If boot source override is enabled, we need to check 'one_time'
    // property to set a correct value for the "BootSourceOverrideEnabled"
    // 如果启用了启动源覆盖，我们需要检查 'one_time'
    // 属性，以便为 "BootSourceOverrideEnabled" 设置正确的值
    dbus::utility::getProperty<bool>(
        "xyz.openbmc_project.Settings", path,
        "xyz.openbmc_project.Object.Enable", "Enabled",
        // ast-grep-ignore: long-lambda
        // ast-grep-ignore: long-lambda（ast-grep 工具指令：忽略过长的 lambda 表达式）
        [asyncResp](const boost::system::error_code& ec, bool oneTimeSetting) {
            if (ec)
            {
                BMCWEB_LOG_ERROR("DBUS response error {}", ec);
                messages::internalError(asyncResp->res);
                return;
            }

            if (oneTimeSetting)
            {
                asyncResp->res.jsonValue["Boot"]["BootSourceOverrideEnabled"] =
                    "Once";
            }
            else
            {
                asyncResp->res.jsonValue["Boot"]["BootSourceOverrideEnabled"] =
                    "Continuous";
            }
        });
}

/**
 * @brief Retrieves boot override enable over DBUS
 *
 * @param[in] asyncResp     Shared pointer for generating response message.
 * @param[in] computerSystemIndex Index associated with the requested system
 *
 * @return None.
 */
/* @brief 通过 DBUS 检索启动覆盖使能状态
 * @param[in] asyncResp     用于生成响应消息的共享指针。
 * @param[in] computerSystemIndex 与所请求系统关联的索引
 * @return 无。 */
inline void getBootOverrideEnable(
    const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
    const uint64_t computerSystemIndex)
{
    sdbusplus::object_path path("/xyz/openbmc_project/control/host" +
                                std::to_string(computerSystemIndex));
    path /= "boot";

    dbus::utility::getProperty<bool>(
        "xyz.openbmc_project.Settings", path,
        "xyz.openbmc_project.Object.Enable", "Enabled",
        // ast-grep-ignore: long-lambda
        // ast-grep-ignore: long-lambda（ast-grep 工具指令：忽略过长的 lambda 表达式）
        [asyncResp, computerSystemIndex](const boost::system::error_code& ec,
                                         const bool bootOverrideEnable) {
            if (ec)
            {
                // Service not available, no error, just don't return
                // Boot Override Enable information
                // 服务不可用，并非错误，只是不返回
                // 启动覆盖使能（Boot Override Enable）信息
                if (ec.value() != EBADR &&
                    ec.value() != boost::asio::error::host_unreachable)
                {
                    BMCWEB_LOG_ERROR("D-Bus response error: {}", ec);
                    messages::internalError(asyncResp->res);
                }
                return;
            }

            processBootOverrideEnable(asyncResp, computerSystemIndex,
                                      bootOverrideEnable);
        });
}

/**
 * @brief Retrieves boot source override properties
 *
 * @param[in] asyncResp     Shared pointer for generating response message.
 * @param[in] computerSystemIndex Index associated with the requested system
 *
 * @return None.
 */
/* @brief 检索启动源覆盖属性
 * @param[in] asyncResp     用于生成响应消息的共享指针。
 * @param[in] computerSystemIndex 与所请求系统关联的索引
 * @return 无。 */
inline void getBootProperties(
    const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
    const uint64_t computerSystemIndex)
{
    BMCWEB_LOG_DEBUG("Get boot information.");

    getBootOverrideSource(asyncResp, computerSystemIndex);
    getBootOverrideType(asyncResp, computerSystemIndex);
    getBootOverrideEnable(asyncResp, computerSystemIndex);
}

/**
 * @brief Retrieves the Last Reset Time
 *
 * "Reset" is an overloaded term in Redfish, "Reset" includes power on
 * and power off. Even though this is the "system" Redfish object look at the
 * chassis D-Bus interface for the LastStateChangeTime since this has the
 * last power operation time.
 *
 * @param[in] asyncResp     Shared pointer for generating response message.
 *
 * @return None.
 */
/* @brief 检索最后一次复位时间（Last Reset Time）
 * 在 Redfish 中 "Reset"（复位）是一个含义宽泛的术语，包含上电和
 * 断电。尽管这是 "system"（系统）Redfish 对象，但这里查看的是 chassis
 * （机箱）D-Bus 接口中的 LastStateChangeTime，因为它保存了最后一次
 * 电源操作的时间。
 * @param[in] asyncResp     用于生成响应消息的共享指针。
 * @return 无。 */
inline void getLastResetTime(
    const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
    const uint64_t computerSystemIndex)
{
    BMCWEB_LOG_DEBUG("Getting System Last Reset Time");
    sdbusplus::object_path path =
        systems_utils::getChassisStateObjectPath(computerSystemIndex);
    dbus::utility::getProperty<uint64_t>(
        systems_utils::getChassisStateServiceName(computerSystemIndex), path,
        "xyz.openbmc_project.State.Chassis", "LastStateChangeTime",
        // ast-grep-ignore: long-lambda
        // ast-grep-ignore: long-lambda（ast-grep 工具指令：忽略过长的 lambda 表达式）
        [asyncResp](const boost::system::error_code& ec,
                    uint64_t lastResetTime) {
            if (ec)
            {
                BMCWEB_LOG_DEBUG("D-BUS response error {}", ec);
                return;
            }

            // LastStateChangeTime is epoch time, in milliseconds
            // https://github.com/openbmc/phosphor-dbus-interfaces/blob/33e8e1dd64da53a66e888d33dc82001305cd0bf9/xyz/openbmc_project/State/Chassis.interface.yaml#L19
            // LastStateChangeTime 是 Epoch 时间，单位为毫秒
            // https://github.com/openbmc/phosphor-dbus-interfaces/blob/33e8e1dd64da53a66e888d33dc82001305cd0bf9/xyz/openbmc_project/State/Chassis.interface.yaml#L19
            uint64_t lastResetTimeStamp = lastResetTime / 1000;

            // Convert to ISO 8601 standard
            // 转换为 ISO 8601 标准格式
            asyncResp->res.jsonValue["LastResetTime"] =
                redfish::time_utils::getDateTimeUint(lastResetTimeStamp);
        });
}

/**
 * @brief Retrieves the number of automatic boot Retry attempts allowed/left.
 *
 * The total number of automatic reboot retries allowed "RetryAttempts" and its
 * corresponding property "AttemptsLeft" that keeps track of the amount of
 * automatic retry attempts left are hosted in phosphor-state-manager through
 * dbus.
 *
 * @param[in] asyncResp     Shared pointer for generating response message.
 * @param[in] computerSystemIndex Index associated with the requested system
 *
 * @return None.
 */
/* @brief 检索允许/剩余的自动启动重试次数。
 * 允许的总自动重启重试次数 "RetryAttempts" 及其对应的属性
 * "AttemptsLeft"（用于记录剩余的自动重试次数）由 phosphor-state-manager
 * 通过 dbus 托管。
 * @param[in] asyncResp     用于生成响应消息的共享指针。
 * @param[in] computerSystemIndex 与所请求系统关联的索引
 * @return 无。 */
inline void getAutomaticRebootAttempts(
    const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
    const uint64_t computerSystemIndex)
{
    BMCWEB_LOG_DEBUG("Get Automatic Retry policy");
    sdbusplus::object_path path =
        systems_utils::getHostStateObjectPath(computerSystemIndex);
    dbus::utility::getAllProperties(
        systems_utils::getHostStateServiceName(computerSystemIndex), path,
        "xyz.openbmc_project.Control.Boot.RebootAttempts",
        [asyncResp{asyncResp}](
            const boost::system::error_code& ec,
            const dbus::utility::DBusPropertiesMap& propertiesList) {
            if (ec)
            {
                if (ec.value() != EBADR &&
                    ec.value() != boost::asio::error::host_unreachable)
                {
                    // Service not available, no error, just don't return
                    // RebootAttempts information
                    // 服务不可用，并非错误，只是不返回
                    // RebootAttempts（重启尝试次数）信息
                    BMCWEB_LOG_ERROR("D-Bus responses error: {}", ec);
                    messages::internalError(asyncResp->res);
                }
                return;
            }

            const uint32_t* attemptsLeft = nullptr;
            const uint32_t* retryAttempts = nullptr;

            const bool success = sdbusplus::unpackPropertiesNoThrow(
                dbus_utils::UnpackErrorPrinter(), propertiesList,
                "AttemptsLeft", attemptsLeft, "RetryAttempts", retryAttempts);

            if (!success)
            {
                messages::internalError(asyncResp->res);
                return;
            }

            if (attemptsLeft != nullptr)
            {
                asyncResp->res
                    .jsonValue["Boot"]["RemainingAutomaticRetryAttempts"] =
                    *attemptsLeft;
            }

            if (retryAttempts != nullptr)
            {
                asyncResp->res.jsonValue["Boot"]["AutomaticRetryAttempts"] =
                    *retryAttempts;
            }
        });
}

/**
 * @brief Retrieves Automatic Retry properties. Known on D-Bus as AutoReboot.
 *
 * @param[in] asyncResp     Shared pointer for generating response message.
 * @param[in] computerSystemIndex Index associated with the requested system
 *
 * @return None.
 */
/* @brief 检索自动重试属性。在 D-Bus 中称为 AutoReboot。
 * @param[in] asyncResp     用于生成响应消息的共享指针。
 * @param[in] computerSystemIndex 与所请求系统关联的索引
 * @return 无。 */
inline void getAutomaticRetryPolicy(
    const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
    const uint64_t computerSystemIndex)
{
    BMCWEB_LOG_DEBUG("Get Automatic Retry policy");

    sdbusplus::object_path path("/xyz/openbmc_project/control/host" +
                                std::to_string(computerSystemIndex));
    path /= "auto_reboot";

    dbus::utility::getProperty<bool>(
        "xyz.openbmc_project.Settings", path,
        "xyz.openbmc_project.Control.Boot.RebootPolicy", "AutoReboot",
        // ast-grep-ignore: long-lambda
        // ast-grep-ignore: long-lambda（ast-grep 工具指令：忽略过长的 lambda 表达式）
        [asyncResp, computerSystemIndex](const boost::system::error_code& ec,
                                         bool autoRebootEnabled) {
            if (ec)
            {
                // Service not available, no error, just don't return
                // AutoReboot information
                // 服务不可用，并非错误，只是不返回
                // AutoReboot 信息
                if (ec.value() != EBADR &&
                    ec.value() != boost::asio::error::host_unreachable)
                {
                    BMCWEB_LOG_ERROR("D-Bus responses error: {}", ec);
                    messages::internalError(asyncResp->res);
                }
                return;
            }

            BMCWEB_LOG_DEBUG("Auto Reboot: {}", autoRebootEnabled);
            if (autoRebootEnabled)
            {
                asyncResp->res.jsonValue["Boot"]["AutomaticRetryConfig"] =
                    "RetryAttempts";
            }
            else
            {
                asyncResp->res.jsonValue["Boot"]["AutomaticRetryConfig"] =
                    "Disabled";
            }
            getAutomaticRebootAttempts(asyncResp, computerSystemIndex);

            // "AutomaticRetryConfig" can be 3 values, Disabled, RetryAlways,
            // and RetryAttempts. OpenBMC only supports Disabled and
            // RetryAttempts.
            // "AutomaticRetryConfig" 可以有 3 个值：Disabled、RetryAlways
            // 和 RetryAttempts。OpenBMC 只支持 Disabled 和
            // RetryAttempts。
            nlohmann::json::array_t allowed;
            allowed.emplace_back("Disabled");
            allowed.emplace_back("RetryAttempts");
            asyncResp->res
                .jsonValue["Boot"]
                          ["AutomaticRetryConfig@Redfish.AllowableValues"] =
                std::move(allowed);
        });
}

/**
 * @brief Sets RetryAttempts
 *
 * @param[in] asyncResp   Shared pointer for generating response message.
 * @param[in] computerSystemIndex Index associated with the requested system
 * @param[in] retryAttempts  "AutomaticRetryAttempts" from request.
 *
 *@return None.
 */
/* @brief 设置 RetryAttempts（重试次数）
 * @param[in] asyncResp   用于生成响应消息的共享指针。
 * @param[in] computerSystemIndex 与所请求系统关联的索引
 * @param[in] retryAttempts  请求中的 "AutomaticRetryAttempts"。
 * @return 无。 */

inline void setAutomaticRetryAttempts(
    const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
    const uint64_t computerSystemIndex, const uint32_t retryAttempts)
{
    BMCWEB_LOG_DEBUG("Set Automatic Retry Attempts.");

    setDbusProperty(asyncResp, "Boot/AutomaticRetryAttempts",
                    systems_utils::getHostStateServiceName(computerSystemIndex),
                    systems_utils::getHostStateObjectPath(computerSystemIndex),
                    "xyz.openbmc_project.Control.Boot.RebootAttempts",
                    "RetryAttempts", retryAttempts);
}

inline computer_system::PowerRestorePolicyTypes
    redfishPowerRestorePolicyFromDbus(std::string_view value)
{
    if (value ==
        "xyz.openbmc_project.Control.Power.RestorePolicy.Policy.AlwaysOn")
    {
        return computer_system::PowerRestorePolicyTypes::AlwaysOn;
    }
    if (value ==
        "xyz.openbmc_project.Control.Power.RestorePolicy.Policy.AlwaysOff")
    {
        return computer_system::PowerRestorePolicyTypes::AlwaysOff;
    }
    if (value ==
        "xyz.openbmc_project.Control.Power.RestorePolicy.Policy.Restore")
    {
        return computer_system::PowerRestorePolicyTypes::LastState;
    }
    if (value == "xyz.openbmc_project.Control.Power.RestorePolicy.Policy.None")
    {
        return computer_system::PowerRestorePolicyTypes::AlwaysOff;
    }
    return computer_system::PowerRestorePolicyTypes::Invalid;
}
/**
 * @brief Retrieves power restore policy over DBUS.
 *
 * @param[in] asyncResp     Shared pointer for generating response message.
 *
 * @return None.
 */
/* @brief 通过 DBUS 检索上电恢复策略。
 * @param[in] asyncResp     用于生成响应消息的共享指针。
 * @return 无。 */
inline void getPowerRestorePolicy(
    const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
    const uint64_t computerSystemIndex)
{
    BMCWEB_LOG_DEBUG("Get power restore policy");

    sdbusplus::object_path path("/xyz/openbmc_project/control/host" +
                                std::to_string(computerSystemIndex));
    path /= "power_restore_policy";

    dbus::utility::getProperty<std::string>(
        "xyz.openbmc_project.Settings", path,
        "xyz.openbmc_project.Control.Power.RestorePolicy", "PowerRestorePolicy",
        // ast-grep-ignore: long-lambda
        // ast-grep-ignore: long-lambda（ast-grep 工具指令：忽略过长的 lambda 表达式）
        [asyncResp](const boost::system::error_code& ec,
                    const std::string& policy) {
            if (ec)
            {
                BMCWEB_LOG_DEBUG("DBUS response error {}", ec);
                return;
            }
            computer_system::PowerRestorePolicyTypes restore =
                redfishPowerRestorePolicyFromDbus(policy);
            if (restore == computer_system::PowerRestorePolicyTypes::Invalid)
            {
                messages::internalError(asyncResp->res);
                return;
            }

            asyncResp->res.jsonValue["PowerRestorePolicy"] = restore;
        });
}

/**
 * @brief Stop Boot On Fault over DBUS.
 *
 * @param[in] asyncResp     Shared pointer for generating response message.
 *
 * @return None.
 */
/* @brief 通过 DBUS 获取故障停机（Stop Boot On Fault）设置。
 * @param[in] asyncResp     用于生成响应消息的共享指针。
 * @return 无。 */
inline void getStopBootOnFault(
    const std::shared_ptr<bmcweb::AsyncResp>& asyncResp)
{
    BMCWEB_LOG_DEBUG("Get Stop Boot On Fault");

    dbus::utility::getProperty<bool>(
        "xyz.openbmc_project.Settings", "/xyz/openbmc_project/logging/settings",
        "xyz.openbmc_project.Logging.Settings", "QuiesceOnHwError",
        // ast-grep-ignore: long-lambda
        // ast-grep-ignore: long-lambda（ast-grep 工具指令：忽略过长的 lambda 表达式）
        [asyncResp](const boost::system::error_code& ec, bool value) {
            if (ec)
            {
                // Service not available, no error, just don't return
                // StopBootOnFault information
                // 服务不可用，并非错误，只是不返回
                // StopBootOnFault 信息
                if (ec.value() != EBADR ||
                    ec.value() != boost::asio::error::host_unreachable)
                {
                    BMCWEB_LOG_ERROR("DBUS response error {}", ec);
                    messages::internalError(asyncResp->res);
                }
                return;
            }

            if (value)
            {
                asyncResp->res.jsonValue["Boot"]["StopBootOnFault"] =
                    computer_system::StopBootOnFault::AnyFault;
            }
            else
            {
                asyncResp->res.jsonValue["Boot"]["StopBootOnFault"] =
                    computer_system::StopBootOnFault::Never;
            }
        });
}

inline void getTrustedModuleRequiredToBootCallback(
    const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
    const uint64_t computerSystemIndex, const boost::system::error_code& ec,
    const dbus::utility::MapperGetSubTreeResponse& subtree)
{
    if (ec)
    {
        BMCWEB_LOG_DEBUG("DBUS response error on TPM.Policy GetSubTree{}", ec);
        // This is an optional D-Bus object so just return if
        // error occurs
        // 这是一个可选的 D-Bus 对象，出错时直接返回
        return;
    }
    if (subtree.empty())
    {
        // As noted above, this is an optional interface so just return
        // if there is no instance found
        // 如上所述，这是一个可选接口，如果
        // 找不到实例则直接返回
        return;
    }

    std::string path;
    std::string service;

    if constexpr (BMCWEB_EXPERIMENTAL_REDFISH_MULTI_COMPUTER_SYSTEM)
    {
        if (!systems_utils::indexMatchingSubTreeMapObjectPath(
                asyncResp, computerSystemIndex, subtree, path, service))
        {
            return;
        }
    }
    else
    {
        // Make sure the Dbus response map has a service and objectPath
        // field
        // 确保 Dbus 响应映射中包含 service 和 objectPath
        // 字段
        if (subtree[0].first.empty() || subtree[0].second.size() != 1)
        {
            BMCWEB_LOG_DEBUG("TPM.Policy mapper error!");
            messages::internalError(asyncResp->res);
            return;
        }

        path = subtree[0].first;
        service = subtree[0].second.begin()->first;
    }

    BMCWEB_LOG_DEBUG("found tpm service {}", service);
    BMCWEB_LOG_DEBUG("found tpm path {}", path);

    // Valid TPM Enable object found, now reading the current value
    // 找到有效的 TPM Enable 对象，现在读取当前值
    dbus::utility::getProperty<bool>(
        service, path, "xyz.openbmc_project.Control.TPM.Policy", "TPMEnable",
        // ast-grep-ignore: long-lambda
        // ast-grep-ignore: long-lambda（ast-grep 工具指令：忽略过长的 lambda 表达式）
        [asyncResp](const boost::system::error_code& ec2, bool tpmRequired) {
            if (ec2)
            {
                BMCWEB_LOG_ERROR("D-BUS response error on TPM.Policy Get{}",
                                 ec2);
                messages::internalError(asyncResp->res);
                return;
            }

            if (tpmRequired)
            {
                asyncResp->res
                    .jsonValue["Boot"]["TrustedModuleRequiredToBoot"] =
                    "Required";
            }
            else
            {
                asyncResp->res
                    .jsonValue["Boot"]["TrustedModuleRequiredToBoot"] =
                    "Disabled";
            }
        });
}

/**
 * @brief Get TrustedModuleRequiredToBoot property. Determines whether or not
 * TPM is required for booting the host.
 *
 * @param[in] asyncResp     Shared pointer for generating response message.
 * @param[in] computerSystemIndex Index associated with the requested system
 *
 * @return None.
 */
/* @brief 获取 TrustedModuleRequiredToBoot 属性。确定启动主机时
 * 是否需要 TPM。
 * @param[in] asyncResp     用于生成响应消息的共享指针。
 * @param[in] computerSystemIndex 与所请求系统关联的索引
 * @return 无。 */
inline void getTrustedModuleRequiredToBoot(
    const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
    const uint64_t computerSystemIndex)
{
    BMCWEB_LOG_DEBUG("Get TPM required to boot.");
    constexpr std::array<std::string_view, 1> interfaces = {
        "xyz.openbmc_project.Control.TPM.Policy"};
    dbus::utility::getSubTree(
        "/", 0, interfaces,
        std::bind_front(getTrustedModuleRequiredToBootCallback, asyncResp,
                        computerSystemIndex));
}

inline void setTrustedModuleRequiredToBootCallback(
    const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
    const uint64_t computerSystemIndex, const bool tpmRequired,
    const boost::system::error_code& ec,
    const dbus::utility::MapperGetSubTreeResponse& subtree)
{
    if (ec)
    {
        BMCWEB_LOG_ERROR("DBUS response error on TPM.Policy GetSubTree{}", ec);
        messages::internalError(asyncResp->res);
        return;
    }
    if (subtree.empty())
    {
        messages::propertyNotWritable(asyncResp->res,
                                      "TrustedModuleRequiredToBoot");
        return;
    }

    std::string path;
    std::string serv;

    if constexpr (BMCWEB_EXPERIMENTAL_REDFISH_MULTI_COMPUTER_SYSTEM)
    {
        if (!systems_utils::indexMatchingSubTreeMapObjectPath(
                asyncResp, computerSystemIndex, subtree, path, serv))
        {
            BMCWEB_LOG_DEBUG("TPM.Policy mapper error!");
            messages::internalError(asyncResp->res);
            return;
        }
    }
    else
    {
        // Make sure the Dbus response map has a service and objectPath
        // field
        // 确保 Dbus 响应映射中包含 service 和 objectPath
        // 字段
        if (subtree[0].first.empty() || subtree[0].second.size() != 1)
        {
            BMCWEB_LOG_DEBUG("TPM.Policy mapper error!");
            messages::internalError(asyncResp->res);
            return;
        }

        path = subtree[0].first;
        serv = subtree[0].second.begin()->first;
    }

    if (serv.empty())
    {
        BMCWEB_LOG_DEBUG("TPM.Policy service mapper error!");
        messages::internalError(asyncResp->res);
        return;
    }

    // Valid TPM Enable object found, now setting the value
    // 找到有效的 TPM Enable 对象，现在设置该值
    setDbusProperty(asyncResp, "Boot/TrustedModuleRequiredToBoot", serv, path,
                    "xyz.openbmc_project.Control.TPM.Policy", "TPMEnable",
                    tpmRequired);
}

/**
 * @brief Set TrustedModuleRequiredToBoot property. Determines whether or not
 * TPM is required for booting the host.
 *
 * @param[in] asyncResp     Shared pointer for generating response message.
 * @param[in] computerSystemIndex Index associated with the requested system
 * @param[in] tpmRequired   Value to set TPM Required To Boot property to.
 *
 * @return None.
 */
/* @brief 设置 TrustedModuleRequiredToBoot 属性。确定启动主机时
 * 是否需要 TPM。
 * @param[in] asyncResp     用于生成响应消息的共享指针。
 * @param[in] computerSystemIndex 与所请求系统关联的索引
 * @param[in] tpmRequired   要设置给 TPM Required To Boot 属性的值。
 * @return 无。 */
inline void setTrustedModuleRequiredToBoot(
    const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
    const uint64_t computerSystemIndex, const bool tpmRequired)
{
    BMCWEB_LOG_DEBUG("Set TrustedModuleRequiredToBoot.");
    constexpr std::array<std::string_view, 1> interfaces = {
        "xyz.openbmc_project.Control.TPM.Policy"};
    dbus::utility::getSubTree(
        "/", 0, interfaces,
        std::bind_front(setTrustedModuleRequiredToBootCallback, asyncResp,
                        computerSystemIndex, tpmRequired));
}

/**
 * @brief Sets boot properties into DBUS object(s).
 *
 * @param[in] asyncResp       Shared pointer for generating response message.
 * @param[in] computerSystemIndex Index associated with the requested system
 * @param[in] bootType        The boot type to set.
 * @return Integer error code.
 */
/* @brief 将启动属性设置到 DBUS 对象。
 * @param[in] asyncResp       用于生成响应消息的共享指针。
 * @param[in] computerSystemIndex 与所请求系统关联的索引
 * @param[in] bootType        要设置的启动类型。
 * @return 整数错误码。 */
inline void setBootType(const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
                        const uint64_t computerSystemIndex,
                        const std::optional<std::string>& bootType)
{
    std::string bootTypeStr;

    if (!bootType)
    {
        return;
    }

    // Source target specified
    // 已指定源目标（Source target）
    BMCWEB_LOG_DEBUG("Boot type: {}", *bootType);
    // Figure out which DBUS interface and property to use
    // 确定要使用哪个 DBUS 接口和属性
    if (*bootType == "Legacy")
    {
        bootTypeStr = "xyz.openbmc_project.Control.Boot.Type.Types.Legacy";
    }
    else if (*bootType == "UEFI")
    {
        bootTypeStr = "xyz.openbmc_project.Control.Boot.Type.Types.EFI";
    }
    else
    {
        BMCWEB_LOG_DEBUG("Invalid property value for "
                         "BootSourceOverrideMode: {}",
                         *bootType);
        messages::propertyValueNotInList(asyncResp->res, *bootType,
                                         "BootSourceOverrideMode");
        return;
    }

    // Act on validated parameters
    // 对经过校验的参数执行操作
    BMCWEB_LOG_DEBUG("DBUS boot type: {}", bootTypeStr);

    sdbusplus::object_path path("/xyz/openbmc_project/control/host" +
                                std::to_string(computerSystemIndex));
    path /= "boot";
    setDbusProperty(asyncResp, "Boot/BootSourceOverrideMode",
                    "xyz.openbmc_project.Settings", path,
                    "xyz.openbmc_project.Control.Boot.Type", "BootType",
                    bootTypeStr);
}

/**
 * @brief Sets boot properties into DBUS object(s).
 *
 * @param[in] asyncResp           Shared pointer for generating response
 * message.
 * @param[in] computerSystemIndex Index associated with the requested system
 * @param[in] bootType        The boot type to set.
 * @return Integer error code.
 */
/* @brief 将启动属性设置到 DBUS 对象。
 * @param[in] asyncResp           用于生成响应消息的共享指针。
 * @param[in] computerSystemIndex 与所请求系统关联的索引
 * @param[in] bootType        要设置的启动类型。
 * @return 整数错误码。 */
inline void setBootEnable(const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
                          const uint64_t computerSystemIndex,
                          const std::optional<std::string>& bootEnable)
{
    if (!bootEnable)
    {
        return;
    }
    // Source target specified
    // 已指定源目标（Source target）
    BMCWEB_LOG_DEBUG("Boot enable: {}", *bootEnable);

    bool bootOverrideEnable = false;
    bool bootOverridePersistent = false;
    // Figure out which DBUS interface and property to use
    // 确定要使用哪个 DBUS 接口和属性
    if (*bootEnable == "Disabled")
    {
        bootOverrideEnable = false;
    }
    else if (*bootEnable == "Once")
    {
        bootOverrideEnable = true;
        bootOverridePersistent = false;
    }
    else if (*bootEnable == "Continuous")
    {
        bootOverrideEnable = true;
        bootOverridePersistent = true;
    }
    else
    {
        BMCWEB_LOG_DEBUG(
            "Invalid property value for BootSourceOverrideEnabled: {}",
            *bootEnable);
        messages::propertyValueNotInList(asyncResp->res, *bootEnable,
                                         "BootSourceOverrideEnabled");
        return;
    }

    // Act on validated parameters
    // 对经过校验的参数执行操作
    BMCWEB_LOG_DEBUG("DBUS boot override enable: {}", bootOverrideEnable);

    sdbusplus::object_path path("/xyz/openbmc_project/control/host" +
                                std::to_string(computerSystemIndex));
    path /= "boot";
    setDbusProperty(asyncResp, "Boot/BootSourceOverrideEnabled",
                    "xyz.openbmc_project.Settings", path,
                    "xyz.openbmc_project.Object.Enable", "Enabled",
                    bootOverrideEnable);

    if (!bootOverrideEnable)
    {
        return;
    }

    // In case boot override is enabled we need to set correct value for the
    // 'one_time' enable DBus interface
    // 如果启用了启动覆盖，我们需要为 'one_time' 使能
    // DBus 接口设置正确的值
    BMCWEB_LOG_DEBUG("DBUS boot override persistent: {}",
                     bootOverridePersistent);

    path /= "one_time";
    setDbusProperty(asyncResp, "Boot/BootSourceOverrideEnabled",
                    "xyz.openbmc_project.Settings", path,
                    "xyz.openbmc_project.Object.Enable", "Enabled",
                    !bootOverridePersistent);
}

/**
 * @brief Sets boot properties into DBUS object(s).
 *
 * @param[in] asyncResp       Shared pointer for generating response message.
 * @param[in] computerSystemIndex Index associated with the requested system
 * @param[in] bootSource      The boot source to set.
 *
 * @return Integer error code.
 */
/* @brief 将启动属性设置到 DBUS 对象。
 * @param[in] asyncResp       用于生成响应消息的共享指针。
 * @param[in] computerSystemIndex 与所请求系统关联的索引
 * @param[in] bootSource      要设置的启动源。
 * @return 整数错误码。 */
inline void setBootModeOrSource(
    const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
    const uint64_t computerSystemIndex,
    const std::optional<std::string>& bootSource)
{
    std::string bootSourceStr;
    std::string bootModeStr;

    if (!bootSource)
    {
        return;
    }

    // Source target specified
    // 已指定源目标（Source target）
    BMCWEB_LOG_DEBUG("Boot source: {}", *bootSource);
    // Figure out which DBUS interface and property to use
    // 确定要使用哪个 DBUS 接口和属性
    if (assignBootParameters(*bootSource, bootSourceStr, bootModeStr) != 0)
    {
        BMCWEB_LOG_DEBUG(
            "Invalid property value for BootSourceOverrideTarget: {}",
            *bootSource);
        messages::propertyValueNotInList(asyncResp->res, *bootSource,
                                         "BootSourceTargetOverride");
        return;
    }

    // Act on validated parameters
    // 对经过校验的参数执行操作
    BMCWEB_LOG_DEBUG("DBUS boot source: {}", bootSourceStr);
    BMCWEB_LOG_DEBUG("DBUS boot mode: {}", bootModeStr);

    sdbusplus::object_path path("/xyz/openbmc_project/control/host" +
                                std::to_string(computerSystemIndex));
    path /= "boot";
    setDbusProperty(asyncResp, "Boot/BootSourceOverrideTarget",
                    "xyz.openbmc_project.Settings", path,
                    "xyz.openbmc_project.Control.Boot.Source", "BootSource",
                    bootSourceStr);
    setDbusProperty(asyncResp, "Boot/BootSourceOverrideTarget",
                    "xyz.openbmc_project.Settings", path,
                    "xyz.openbmc_project.Control.Boot.Mode", "BootMode",
                    bootModeStr);
}

/**
 * @brief Sets Boot source override properties.
 *
 * @param[in] asyncResp  Shared pointer for generating response message.
 * @param[in] computerSystemIndex Index associated with the requested system
 * @param[in] bootSource The boot source from incoming RF request.
 * @param[in] bootType   The boot type from incoming RF request.
 * @param[in] bootEnable The boot override enable from incoming RF request.
 *
 * @return Integer error code.
 */
/* @brief 设置启动源覆盖（Boot source override）属性。
 * @param[in] asyncResp  用于生成响应消息的共享指针。
 * @param[in] computerSystemIndex 与所请求系统关联的索引
 * @param[in] bootSource 来自传入 RF 请求的启动源。
 * @param[in] bootType   来自传入 RF 请求的启动类型。
 * @param[in] bootEnable 来自传入 RF 请求的启动覆盖使能。
 * @return 整数错误码。 */

inline void setBootProperties(
    const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
    const uint64_t computerSystemIndex,
    const std::optional<std::string>& bootSource,
    const std::optional<std::string>& bootType,
    const std::optional<std::string>& bootEnable)
{
    BMCWEB_LOG_DEBUG("Set boot information.");

    setBootModeOrSource(asyncResp, computerSystemIndex, bootSource);
    setBootType(asyncResp, computerSystemIndex, bootType);
    setBootEnable(asyncResp, computerSystemIndex, bootEnable);
}

/**
 * @brief Sets AssetTag
 *
 * @param[in] asyncResp Shared pointer for generating response message.
 * @param[in] assetTag  "AssetTag" from request.
 *
 * @return None.
 */
/* @brief 设置 AssetTag（资产标签）
 * @param[in] asyncResp 用于生成响应消息的共享指针。
 * @param[in] assetTag  请求中的 "AssetTag"。
 * @return 无。 */
inline void setAssetTag(const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
                        const std::string& assetTag)
{
    constexpr std::array<std::string_view, 1> interfaces = {
        "xyz.openbmc_project.Inventory.Item.System"};
    dbus::utility::getSubTree(
        "/xyz/openbmc_project/inventory", 0, interfaces,
        // ast-grep-ignore: long-lambda
        // ast-grep-ignore: long-lambda（ast-grep 工具指令：忽略过长的 lambda 表达式）
        [asyncResp,
         assetTag](const boost::system::error_code& ec,
                   const dbus::utility::MapperGetSubTreeResponse& subtree) {
            if (ec)
            {
                BMCWEB_LOG_DEBUG("D-Bus response error on GetSubTree {}", ec);
                messages::internalError(asyncResp->res);
                return;
            }
            if (subtree.empty())
            {
                BMCWEB_LOG_DEBUG("Can't find system D-Bus object!");
                messages::internalError(asyncResp->res);
                return;
            }
            // Assume only 1 system D-Bus object
            // Throw an error if there is more than 1
            // 假定只有一个系统 D-Bus 对象
            // 如果多于一个则抛出错误
            if (subtree.size() > 1)
            {
                BMCWEB_LOG_DEBUG("Found more than 1 system D-Bus object!");
                messages::internalError(asyncResp->res);
                return;
            }
            if (subtree[0].first.empty() || subtree[0].second.size() != 1)
            {
                BMCWEB_LOG_DEBUG("Asset Tag Set mapper error!");
                messages::internalError(asyncResp->res);
                return;
            }

            const std::string& path = subtree[0].first;
            const std::string& service = subtree[0].second.begin()->first;

            if (service.empty())
            {
                BMCWEB_LOG_DEBUG("Asset Tag Set service mapper error!");
                messages::internalError(asyncResp->res);
                return;
            }

            setDbusProperty(asyncResp, "AssetTag", service, path,
                            "xyz.openbmc_project.Inventory.Decorator.AssetTag",
                            "AssetTag", assetTag);
        });
}

/**
 * @brief Validate the specified stopBootOnFault is valid and return the
 * stopBootOnFault name associated with that string
 *
 * @param[in] stopBootOnFaultString  String representing the desired
 * stopBootOnFault
 *
 * @return stopBootOnFault value or empty  if incoming value is not valid
 */
/* @brief 校验指定的 stopBootOnFault 是否有效，并返回与该字符串
 * 关联的 stopBootOnFault 名称
 * @param[in] stopBootOnFaultString  表示所需 stopBootOnFault 的字符串
 * @return stopBootOnFault 值；如果传入值无效则返回空 */
inline std::optional<bool> validstopBootOnFault(
    const std::string& stopBootOnFaultString)
{
    if (stopBootOnFaultString == "AnyFault")
    {
        return true;
    }

    if (stopBootOnFaultString == "Never")
    {
        return false;
    }

    return std::nullopt;
}

/**
 * @brief Sets stopBootOnFault
 *
 * @param[in] asyncResp   Shared pointer for generating response message.
 * @param[in] stopBootOnFault  "StopBootOnFault" from request.
 *
 * @return None.
 */
/* @brief 设置 stopBootOnFault
 * @param[in] asyncResp   用于生成响应消息的共享指针。
 * @param[in] stopBootOnFault  请求中的 "StopBootOnFault"。
 * @return 无。 */
inline void setStopBootOnFault(
    const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
    const std::string& stopBootOnFault)
{
    BMCWEB_LOG_DEBUG("Set Stop Boot On Fault.");

    std::optional<bool> stopBootEnabled = validstopBootOnFault(stopBootOnFault);
    if (!stopBootEnabled)
    {
        BMCWEB_LOG_DEBUG("Invalid property value for StopBootOnFault: {}",
                         stopBootOnFault);
        messages::propertyValueNotInList(asyncResp->res, stopBootOnFault,
                                         "StopBootOnFault");
        return;
    }

    setDbusProperty(
        asyncResp, "Boot/StopBootOnFault", "xyz.openbmc_project.Settings",
        sdbusplus::object_path("/xyz/openbmc_project/logging/settings"),
        "xyz.openbmc_project.Logging.Settings", "QuiesceOnHwError",
        *stopBootEnabled);
}

/**
 * @brief Sets automaticRetry (Auto Reboot)
 *
 * @param[in] asyncResp   Shared pointer for generating response message.
 * @param[in] computerSystemIndex Index associated with the requested system
 * @param[in] automaticRetryConfig  "AutomaticRetryConfig" from request.
 *
 * @return None.
 */
/* @brief 设置 automaticRetry（自动重启）
 * @param[in] asyncResp   用于生成响应消息的共享指针。
 * @param[in] computerSystemIndex 与所请求系统关联的索引
 * @param[in] automaticRetryConfig  请求中的 "AutomaticRetryConfig"。
 * @return 无。 */
inline void setAutomaticRetry(
    const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
    const uint64_t computerSystemIndex, const std::string& automaticRetryConfig)
{
    BMCWEB_LOG_DEBUG("Set Automatic Retry.");

    // OpenBMC only supports "Disabled" and "RetryAttempts".
    // OpenBMC 只支持 "Disabled" 和 "RetryAttempts"。
    bool autoRebootEnabled = false;

    if (automaticRetryConfig == "Disabled")
    {
        autoRebootEnabled = false;
    }
    else if (automaticRetryConfig == "RetryAttempts")
    {
        autoRebootEnabled = true;
    }
    else
    {
        BMCWEB_LOG_DEBUG("Invalid property value for AutomaticRetryConfig: {}",
                         automaticRetryConfig);
        messages::propertyValueNotInList(asyncResp->res, automaticRetryConfig,
                                         "AutomaticRetryConfig");
        return;
    }

    sdbusplus::object_path path("/xyz/openbmc_project/control/host" +
                                std::to_string(computerSystemIndex));
    path /= "auto_reboot";
    setDbusProperty(asyncResp, "Boot/AutomaticRetryConfig",
                    "xyz.openbmc_project.Settings", path,
                    "xyz.openbmc_project.Control.Boot.RebootPolicy",
                    "AutoReboot", autoRebootEnabled);
}

inline std::string dbusPowerRestorePolicyFromRedfish(std::string_view policy)
{
    if (policy == "AlwaysOn")
    {
        return "xyz.openbmc_project.Control.Power.RestorePolicy.Policy.AlwaysOn";
    }
    if (policy == "AlwaysOff")
    {
        return "xyz.openbmc_project.Control.Power.RestorePolicy.Policy.AlwaysOff";
    }
    if (policy == "LastState")
    {
        return "xyz.openbmc_project.Control.Power.RestorePolicy.Policy.Restore";
    }
    return "";
}

/**
 * @brief Sets power restore policy properties.
 *
 * @param[in] asyncResp   Shared pointer for generating response message.
 * @param[in] computerSystemIndex Index associated with the requested system
 * @param[in] policy  power restore policy properties from request.
 *
 * @return None.
 */
/* @brief 设置上电恢复策略属性。
 * @param[in] asyncResp   用于生成响应消息的共享指针。
 * @param[in] computerSystemIndex 与所请求系统关联的索引
 * @param[in] policy  请求中的上电恢复策略属性。
 * @return 无。 */
inline void setPowerRestorePolicy(
    const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
    const uint64_t computerSystemIndex, std::string_view policy)
{
    BMCWEB_LOG_DEBUG("Set power restore policy.");

    std::string powerRestorePolicy = dbusPowerRestorePolicyFromRedfish(policy);

    if (powerRestorePolicy.empty())
    {
        messages::propertyValueNotInList(asyncResp->res, policy,
                                         "PowerRestorePolicy");
        return;
    }

    sdbusplus::object_path path("/xyz/openbmc_project/control/host" +
                                std::to_string(computerSystemIndex));
    path /= "power_restore_policy";
    setDbusProperty(asyncResp, "PowerRestorePolicy",
                    "xyz.openbmc_project.Settings", path,
                    "xyz.openbmc_project.Control.Power.RestorePolicy",
                    "PowerRestorePolicy", powerRestorePolicy);
}

/**
 * @brief Retrieves provisioning status
 *
 * @param[in] asyncResp     Shared pointer for completing asynchronous
 * calls.
 *
 * @return None.
 */
/* @brief 检索预置（provisioning）状态
 * @param[in] asyncResp     用于完成异步调用的共享指针。
 * @return 无。 */
inline void getProvisioningStatus(
    const std::shared_ptr<bmcweb::AsyncResp>& asyncResp)
{
    BMCWEB_LOG_DEBUG("Get OEM information.");
    dbus::utility::getAllProperties(
        "xyz.openbmc_project.PFR.Manager", "/xyz/openbmc_project/pfr",
        "xyz.openbmc_project.PFR.Attributes",
        // ast-grep-ignore: long-lambda
        // ast-grep-ignore: long-lambda（ast-grep 工具指令：忽略过长的 lambda 表达式）
        [asyncResp](const boost::system::error_code& ec,
                    const dbus::utility::DBusPropertiesMap& propertiesList) {
            nlohmann::json& oemPFR =
                asyncResp->res
                    .jsonValue["Oem"]["OpenBmc"]["FirmwareProvisioning"];
            asyncResp->res.jsonValue["Oem"]["OpenBmc"]["@odata.type"] =
                "#OpenBMCComputerSystem.v1_0_0.OpenBmc";
            oemPFR["@odata.type"] =
                "#OpenBMCComputerSystem.FirmwareProvisioning";

            if (ec)
            {
                BMCWEB_LOG_DEBUG("DBUS response error {}", ec);
                // not an error, don't have to have the interface
                // 这不是错误，不一定非要提供该接口
                oemPFR["ProvisioningStatus"] = open_bmc_computer_system::
                    FirmwareProvisioningStatus::NotProvisioned;
                return;
            }

            const bool* provState = nullptr;
            const bool* lockState = nullptr;

            const bool success = sdbusplus::unpackPropertiesNoThrow(
                dbus_utils::UnpackErrorPrinter(), propertiesList,
                "UfmProvisioned", provState, "UfmLocked", lockState);

            if (!success)
            {
                messages::internalError(asyncResp->res);
                return;
            }

            if ((provState == nullptr) || (lockState == nullptr))
            {
                BMCWEB_LOG_DEBUG("Unable to get PFR attributes.");
                messages::internalError(asyncResp->res);
                return;
            }

            if (*provState)
            {
                if (*lockState)
                {
                    oemPFR["ProvisioningStatus"] = open_bmc_computer_system::
                        FirmwareProvisioningStatus::ProvisionedAndLocked;
                }
                else
                {
                    oemPFR["ProvisioningStatus"] = open_bmc_computer_system::
                        FirmwareProvisioningStatus::ProvisionedButNotLocked;
                }
            }
            else
            {
                oemPFR["ProvisioningStatus"] = open_bmc_computer_system::
                    FirmwareProvisioningStatus::NotProvisioned;
            }
        });
}

/**
 * @brief Translate the PowerMode string to enum value
 *
 * @param[in]  modeString PowerMode string to be translated
 *
 * @return PowerMode enum
 */
/* @brief 将 PowerMode 字符串转换为枚举值
 * @param[in]  modeString 要转换的 PowerMode 字符串
 * @return PowerMode 枚举 */
inline computer_system::PowerMode translatePowerModeString(
    const std::string& modeString)
{
    using PowerMode = computer_system::PowerMode;

    if (modeString == "xyz.openbmc_project.Control.Power.Mode.PowerMode.Static")
    {
        return PowerMode::Static;
    }
    if (modeString ==
        "xyz.openbmc_project.Control.Power.Mode.PowerMode.MaximumPerformance")
    {
        return PowerMode::MaximumPerformance;
    }
    if (modeString ==
        "xyz.openbmc_project.Control.Power.Mode.PowerMode.PowerSaving")
    {
        return PowerMode::PowerSaving;
    }
    if (modeString ==
        "xyz.openbmc_project.Control.Power.Mode.PowerMode.BalancedPerformance")
    {
        return PowerMode::BalancedPerformance;
    }
    if (modeString ==
        "xyz.openbmc_project.Control.Power.Mode.PowerMode.EfficiencyFavorPerformance")
    {
        return PowerMode::EfficiencyFavorPerformance;
    }
    if (modeString ==
        "xyz.openbmc_project.Control.Power.Mode.PowerMode.EfficiencyFavorPower")
    {
        return PowerMode::EfficiencyFavorPower;
    }
    if (modeString == "xyz.openbmc_project.Control.Power.Mode.PowerMode.OEM")
    {
        return PowerMode::OEM;
    }
    // Any other values would be invalid
    // 任何其他值都是无效的
    BMCWEB_LOG_ERROR("PowerMode value was not valid: {}", modeString);
    return PowerMode::Invalid;
}

inline void afterGetPowerMode(
    const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
    const boost::system::error_code& ec,
    const dbus::utility::DBusPropertiesMap& properties)
{
    if (ec)
    {
        BMCWEB_LOG_ERROR("DBUS response error on PowerMode GetAll: {}", ec);
        messages::internalError(asyncResp->res);
        return;
    }

    std::string powerMode;
    const std::vector<std::string>* allowedModes = nullptr;
    const bool success = sdbusplus::unpackPropertiesNoThrow(
        dbus_utils::UnpackErrorPrinter(), properties, "PowerMode", powerMode,
        "AllowedPowerModes", allowedModes);

    if (!success)
    {
        messages::internalError(asyncResp->res);
        return;
    }

    nlohmann::json::array_t modeList;
    if (allowedModes == nullptr)
    {
        modeList.emplace_back("Static");
        modeList.emplace_back("MaximumPerformance");
        modeList.emplace_back("PowerSaving");
    }
    else
    {
        for (const auto& aMode : *allowedModes)
        {
            computer_system::PowerMode modeValue =
                translatePowerModeString(aMode);
            if (modeValue == computer_system::PowerMode::Invalid)
            {
                messages::internalError(asyncResp->res);
                continue;
            }
            modeList.emplace_back(modeValue);
        }
    }
    asyncResp->res.jsonValue["PowerMode@Redfish.AllowableValues"] = modeList;

    BMCWEB_LOG_DEBUG("Current power mode: {}", powerMode);
    const computer_system::PowerMode modeValue =
        translatePowerModeString(powerMode);
    if (modeValue == computer_system::PowerMode::Invalid)
    {
        messages::internalError(asyncResp->res);
        return;
    }
    asyncResp->res.jsonValue["PowerMode"] = modeValue;
}
/**
 * @brief Retrieves system power mode
 *
 * @param[in] asyncResp  Shared pointer for generating response message.
 *
 * @return None.
 */
/* @brief 检索系统电源模式
 * @param[in] asyncResp  用于生成响应消息的共享指针。
 * @return 无。 */
inline void getPowerMode(const std::shared_ptr<bmcweb::AsyncResp>& asyncResp)
{
    BMCWEB_LOG_DEBUG("Get power mode.");

    // Get Power Mode object path:
    // 获取 Power Mode 对象路径：
    constexpr std::array<std::string_view, 1> interfaces = {
        "xyz.openbmc_project.Control.Power.Mode"};
    dbus::utility::getSubTree(
        "/", 0, interfaces,
        // ast-grep-ignore: long-lambda
        // ast-grep-ignore: long-lambda（ast-grep 工具指令：忽略过长的 lambda 表达式）
        [asyncResp](const boost::system::error_code& ec,
                    const dbus::utility::MapperGetSubTreeResponse& subtree) {
            if (ec)
            {
                BMCWEB_LOG_DEBUG(
                    "DBUS response error on Power.Mode GetSubTree {}", ec);
                // This is an optional D-Bus object so just return if
                // error occurs
                // 这是一个可选的 D-Bus 对象，出错时直接返回
                return;
            }
            if (subtree.empty())
            {
                // As noted above, this is an optional interface so just return
                // if there is no instance found
                // 如上所述，这是一个可选接口，如果
                // 找不到实例则直接返回
                return;
            }
            if (subtree.size() > 1)
            {
                // More then one PowerMode object is not supported and is an
                // error
                // 不支持多于一个 PowerMode 对象，这是
                // 一个错误
                BMCWEB_LOG_DEBUG(
                    "Found more than 1 system D-Bus Power.Mode objects: {}",
                    subtree.size());
                messages::internalError(asyncResp->res);
                return;
            }
            if ((subtree[0].first.empty()) || (subtree[0].second.size() != 1))
            {
                BMCWEB_LOG_DEBUG("Power.Mode mapper error!");
                messages::internalError(asyncResp->res);
                return;
            }
            const std::string& path = subtree[0].first;
            const std::string& service = subtree[0].second.begin()->first;
            if (service.empty())
            {
                BMCWEB_LOG_DEBUG("Power.Mode service mapper error!");
                messages::internalError(asyncResp->res);
                return;
            }

            // Valid Power Mode object found, now read the mode properties
            // 找到有效的 Power Mode 对象，现在读取模式属性
            dbus::utility::getAllProperties(
                *crow::connections::systemBus, service, path,
                "xyz.openbmc_project.Control.Power.Mode",
                [asyncResp](
                    const boost::system::error_code& ec2,
                    const dbus::utility::DBusPropertiesMap& properties) {
                    afterGetPowerMode(asyncResp, ec2, properties);
                });
        });
}

/**
 * @brief Validate the specified mode is valid and return the PowerMode
 * name associated with that string
 *
 * @param[in] asyncResp   Shared pointer for generating response message.
 * @param[in] modeValue   String representing the desired PowerMode
 *
 * @return PowerMode value or empty string if mode is not valid
 */
/* @brief 校验指定的模式是否有效，并返回与该字符串关联的
 * PowerMode 名称
 * @param[in] asyncResp   用于生成响应消息的共享指针。
 * @param[in] modeValue   表示所需 PowerMode 的字符串
 * @return PowerMode 值；如果模式无效则返回空字符串 */
inline std::string validatePowerMode(
    const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
    const nlohmann::json& modeValue)
{
    using PowerMode = computer_system::PowerMode;
    std::string mode;

    if (modeValue == PowerMode::Static)
    {
        mode = "xyz.openbmc_project.Control.Power.Mode.PowerMode.Static";
    }
    else if (modeValue == PowerMode::MaximumPerformance)
    {
        mode =
            "xyz.openbmc_project.Control.Power.Mode.PowerMode.MaximumPerformance";
    }
    else if (modeValue == PowerMode::PowerSaving)
    {
        mode = "xyz.openbmc_project.Control.Power.Mode.PowerMode.PowerSaving";
    }
    else if (modeValue == PowerMode::BalancedPerformance)
    {
        mode =
            "xyz.openbmc_project.Control.Power.Mode.PowerMode.BalancedPerformance";
    }
    else if (modeValue == PowerMode::EfficiencyFavorPerformance)
    {
        mode =
            "xyz.openbmc_project.Control.Power.Mode.PowerMode.EfficiencyFavorPerformance";
    }
    else if (modeValue == PowerMode::EfficiencyFavorPower)
    {
        mode =
            "xyz.openbmc_project.Control.Power.Mode.PowerMode.EfficiencyFavorPower";
    }
    else
    {
        messages::propertyValueNotInList(asyncResp->res, modeValue,
                                         "PowerMode");
    }
    return mode;
}

/**
 * @brief Sets system power mode.
 *
 * @param[in] asyncResp   Shared pointer for generating response message.
 * @param[in] pmode   System power mode from request.
 *
 * @return None.
 */
/* @brief 设置系统电源模式。
 * @param[in] asyncResp   用于生成响应消息的共享指针。
 * @param[in] pmode   请求中的系统电源模式。
 * @return 无。 */
inline void setPowerMode(const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
                         const std::string& pmode)
{
    BMCWEB_LOG_DEBUG("Set power mode.");

    std::string powerMode = validatePowerMode(asyncResp, pmode);
    if (powerMode.empty())
    {
        return;
    }

    // Get Power Mode object path:
    // 获取 Power Mode 对象路径：
    constexpr std::array<std::string_view, 1> interfaces = {
        "xyz.openbmc_project.Control.Power.Mode"};
    dbus::utility::getSubTree(
        "/", 0, interfaces,
        // ast-grep-ignore: long-lambda
        // ast-grep-ignore: long-lambda（ast-grep 工具指令：忽略过长的 lambda 表达式）
        [asyncResp,
         powerMode](const boost::system::error_code& ec,
                    const dbus::utility::MapperGetSubTreeResponse& subtree) {
            if (ec)
            {
                BMCWEB_LOG_ERROR(
                    "DBUS response error on Power.Mode GetSubTree {}", ec);
                // This is an optional D-Bus object, but user attempted to patch
                // 这是一个可选的 D-Bus 对象，但用户尝试对其进行 patch
                messages::internalError(asyncResp->res);
                return;
            }
            if (subtree.empty())
            {
                // This is an optional D-Bus object, but user attempted to patch
                // 这是一个可选的 D-Bus 对象，但用户尝试对其进行 patch
                messages::resourceNotFound(asyncResp->res, "ComputerSystem",
                                           "PowerMode");
                return;
            }
            if (subtree.size() > 1)
            {
                // More then one PowerMode object is not supported and is an
                // error
                // 不支持多于一个 PowerMode 对象，这是
                // 一个错误
                BMCWEB_LOG_DEBUG(
                    "Found more than 1 system D-Bus Power.Mode objects: {}",
                    subtree.size());
                messages::internalError(asyncResp->res);
                return;
            }
            if ((subtree[0].first.empty()) || (subtree[0].second.size() != 1))
            {
                BMCWEB_LOG_DEBUG("Power.Mode mapper error!");
                messages::internalError(asyncResp->res);
                return;
            }
            const std::string& path = subtree[0].first;
            const std::string& service = subtree[0].second.begin()->first;
            if (service.empty())
            {
                BMCWEB_LOG_DEBUG("Power.Mode service mapper error!");
                messages::internalError(asyncResp->res);
                return;
            }

            BMCWEB_LOG_DEBUG("Setting power mode({}) -> {}", powerMode, path);

            // Set the Power Mode property
            // 设置 Power Mode 属性
            setDbusProperty(asyncResp, "PowerMode", service, path,
                            "xyz.openbmc_project.Control.Power.Mode",
                            "PowerMode", powerMode);
        });
}

/**
 * @brief Translates watchdog timeout action DBUS property value to redfish.
 *
 * @param[in] dbusAction    The watchdog timeout action in D-BUS.
 *
 * @return Returns a WatchdogTimeoutActions enum value representing the timeout
 * action in Redfish terms. If translation cannot be done, returns
 * WatchdogTimeoutActions::Invalid.
 */
/* @brief 将看门狗超时动作的 DBUS 属性值转换为 Redfish 值。
 * @param[in] dbusAction    D-BUS 中的看门狗超时动作。
 * @return 返回一个 WatchdogTimeoutActions 枚举值，表示 Redfish
 * 术语下的超时动作。如果无法转换，则返回
 * WatchdogTimeoutActions::Invalid。 */
inline computer_system::WatchdogTimeoutActions dbusToRfWatchdogAction(
    const std::string& dbusAction)
{
    if (dbusAction == "xyz.openbmc_project.State.Watchdog.Action.None")
    {
        return computer_system::WatchdogTimeoutActions::None;
    }
    if (dbusAction == "xyz.openbmc_project.State.Watchdog.Action.HardReset")
    {
        return computer_system::WatchdogTimeoutActions::ResetSystem;
    }
    if (dbusAction == "xyz.openbmc_project.State.Watchdog.Action.PowerOff")
    {
        return computer_system::WatchdogTimeoutActions::PowerDown;
    }
    if (dbusAction == "xyz.openbmc_project.State.Watchdog.Action.PowerCycle")
    {
        return computer_system::WatchdogTimeoutActions::PowerCycle;
    }

    return computer_system::WatchdogTimeoutActions::Invalid;
}

/**
 *@brief Translates timeout action from Redfish to DBUS property value.
 *
 *@param[in] rfAction The timeout action in Redfish.
 *
 *@return Returns as a string, the time_out action as expected by DBUS.
 *If translation cannot be done, returns an empty string.
 */
/* @brief 将超时动作从 Redfish 转换为 DBUS 属性值。
 * @param[in] rfAction Redfish 中的超时动作。
 * @return 以字符串形式返回 DBUS 所期望的 time_out 动作。
 * 如果无法转换，则返回空字符串。 */

inline std::string rfToDbusWDTTimeOutAct(const std::string& rfAction)
{
    if (rfAction == "None")
    {
        return "xyz.openbmc_project.State.Watchdog.Action.None";
    }
    if (rfAction == "PowerCycle")
    {
        return "xyz.openbmc_project.State.Watchdog.Action.PowerCycle";
    }
    if (rfAction == "PowerDown")
    {
        return "xyz.openbmc_project.State.Watchdog.Action.PowerOff";
    }
    if (rfAction == "ResetSystem")
    {
        return "xyz.openbmc_project.State.Watchdog.Action.HardReset";
    }

    return "";
}

/**
 * @brief Retrieves host watchdog timer properties over DBUS
 *
 * @param[in] asyncResp     Shared pointer for completing asynchronous calls.
 *
 * @return None.
 */
/* @brief 通过 DBUS 检索主机看门狗定时器属性
 * @param[in] asyncResp     用于完成异步调用的共享指针。
 * @return 无。 */
inline void getHostWatchdogTimer(
    const std::shared_ptr<bmcweb::AsyncResp>& asyncResp)
{
    BMCWEB_LOG_DEBUG("Get host watchodg");
    dbus::utility::getAllProperties(
        "xyz.openbmc_project.Watchdog", "/xyz/openbmc_project/watchdog/host0",
        "xyz.openbmc_project.State.Watchdog",
        // ast-grep-ignore: long-lambda
        // ast-grep-ignore: long-lambda（ast-grep 工具指令：忽略过长的 lambda 表达式）
        [asyncResp](const boost::system::error_code& ec,
                    const dbus::utility::DBusPropertiesMap& properties) {
            if (ec)
            {
                // watchdog service is stopped
                // 看门狗服务已停止
                BMCWEB_LOG_DEBUG("DBUS response error {}", ec);
                return;
            }

            BMCWEB_LOG_DEBUG("Got {} wdt prop.", properties.size());

            nlohmann::json& hostWatchdogTimer =
                asyncResp->res.jsonValue["HostWatchdogTimer"];

            // watchdog service is running/enabled
            // 看门狗服务正在运行/已使能
            hostWatchdogTimer["Status"]["State"] = resource::State::Enabled;

            const bool* enabled = nullptr;
            const std::string* expireAction = nullptr;

            const bool success = sdbusplus::unpackPropertiesNoThrow(
                dbus_utils::UnpackErrorPrinter(), properties, "Enabled",
                enabled, "ExpireAction", expireAction);

            if (!success)
            {
                messages::internalError(asyncResp->res);
                return;
            }

            if (enabled != nullptr)
            {
                hostWatchdogTimer["FunctionEnabled"] = *enabled;
            }

            if (expireAction != nullptr)
            {
                computer_system::WatchdogTimeoutActions action =
                    dbusToRfWatchdogAction(*expireAction);
                if (action == computer_system::WatchdogTimeoutActions::Invalid)
                {
                    messages::internalError(asyncResp->res);
                    return;
                }
                hostWatchdogTimer["TimeoutAction"] = action;
            }
        });
}

/**
 * @brief Sets Host WatchDog Timer properties.
 *
 * @param[in] asyncResp  Shared pointer for generating response message.
 * @param[in] wdtEnable  The WDTimer Enable value (true/false) from incoming
 *                       RF request.
 * @param[in] wdtTimeOutAction The WDT Timeout action, from incoming RF request.
 *
 * @return None.
 */
/* @brief 设置主机看门狗定时器（Host WatchDog Timer）属性。
 * @param[in] asyncResp  用于生成响应消息的共享指针。
 * @param[in] wdtEnable  来自传入 RF 请求的 WDTimer 使能值（true/false）。
 * @param[in] wdtTimeOutAction 来自传入 RF 请求的 WDT 超时动作。
 * @return 无。 */
inline void setWDTProperties(
    const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
    const std::optional<bool> wdtEnable,
    const std::optional<std::string>& wdtTimeOutAction)
{
    BMCWEB_LOG_DEBUG("Set host watchdog");

    if (wdtTimeOutAction)
    {
        std::string wdtTimeOutActStr = rfToDbusWDTTimeOutAct(*wdtTimeOutAction);
        // check if TimeOut Action is Valid
        // 检查超时动作（TimeOut Action）是否有效
        if (wdtTimeOutActStr.empty())
        {
            BMCWEB_LOG_DEBUG("Unsupported value for TimeoutAction: {}",
                             *wdtTimeOutAction);
            messages::propertyValueNotInList(asyncResp->res, *wdtTimeOutAction,
                                             "TimeoutAction");
            return;
        }

        setDbusProperty(
            asyncResp, "HostWatchdogTimer/TimeoutAction",
            "xyz.openbmc_project.Watchdog",
            sdbusplus::object_path("/xyz/openbmc_project/watchdog/host0"),
            "xyz.openbmc_project.State.Watchdog", "ExpireAction",
            wdtTimeOutActStr);
    }

    if (wdtEnable)
    {
        setDbusProperty(
            asyncResp, "HostWatchdogTimer/FunctionEnabled",
            "xyz.openbmc_project.Watchdog",
            sdbusplus::object_path("/xyz/openbmc_project/watchdog/host0"),
            "xyz.openbmc_project.State.Watchdog", "Enabled", *wdtEnable);
    }
}

/**
 * @brief Parse the Idle Power Saver properties into json
 *
 * @param[in] asyncResp   Shared pointer for completing asynchronous calls.
 * @param[in] properties  IPS property data from DBus.
 *
 * @return true if successful
 */
/* @brief 将 Idle Power Saver（空闲节能）属性解析为 json
 * @param[in] asyncResp   用于完成异步调用的共享指针。
 * @param[in] properties  来自 DBus 的 IPS 属性数据。
 * @return 成功时返回 true */
inline bool parseIpsProperties(
    const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
    const dbus::utility::DBusPropertiesMap& properties)
{
    const bool* enabled = nullptr;
    const uint8_t* enterUtilizationPercent = nullptr;
    const uint64_t* enterDwellTime = nullptr;
    const uint8_t* exitUtilizationPercent = nullptr;
    const uint64_t* exitDwellTime = nullptr;

    const bool success = sdbusplus::unpackPropertiesNoThrow(
        dbus_utils::UnpackErrorPrinter(), properties, "Enabled", enabled,
        "EnterUtilizationPercent", enterUtilizationPercent, "EnterDwellTime",
        enterDwellTime, "ExitUtilizationPercent", exitUtilizationPercent,
        "ExitDwellTime", exitDwellTime);

    if (!success)
    {
        return false;
    }

    if (enabled != nullptr)
    {
        asyncResp->res.jsonValue["IdlePowerSaver"]["Enabled"] = *enabled;
    }

    if (enterUtilizationPercent != nullptr)
    {
        asyncResp->res.jsonValue["IdlePowerSaver"]["EnterUtilizationPercent"] =
            *enterUtilizationPercent;
    }

    if (enterDwellTime != nullptr)
    {
        const std::chrono::duration<uint64_t, std::milli> ms(*enterDwellTime);
        asyncResp->res.jsonValue["IdlePowerSaver"]["EnterDwellTimeSeconds"] =
            std::chrono::duration_cast<std::chrono::duration<uint64_t>>(ms)
                .count();
    }

    if (exitUtilizationPercent != nullptr)
    {
        asyncResp->res.jsonValue["IdlePowerSaver"]["ExitUtilizationPercent"] =
            *exitUtilizationPercent;
    }

    if (exitDwellTime != nullptr)
    {
        const std::chrono::duration<uint64_t, std::milli> ms(*exitDwellTime);
        asyncResp->res.jsonValue["IdlePowerSaver"]["ExitDwellTimeSeconds"] =
            std::chrono::duration_cast<std::chrono::duration<uint64_t>>(ms)
                .count();
    }

    return true;
}

/**
 * @brief Retrieves idle power saver properties over DBUS
 *
 * @param[in] asyncResp     Shared pointer for completing asynchronous calls.
 *
 * @return None.
 */
/* @brief 通过 DBUS 检索空闲节能（idle power saver）属性
 * @param[in] asyncResp     用于完成异步调用的共享指针。
 * @return 无。 */
inline void getIdlePowerSaver(
    const std::shared_ptr<bmcweb::AsyncResp>& asyncResp)
{
    BMCWEB_LOG_DEBUG("Get idle power saver parameters");

    // Get IdlePowerSaver object path:
    // 获取 IdlePowerSaver 对象路径：
    constexpr std::array<std::string_view, 1> interfaces = {
        "xyz.openbmc_project.Control.Power.IdlePowerSaver"};
    dbus::utility::getSubTree(
        "/", 0, interfaces,
        // ast-grep-ignore: long-lambda
        // ast-grep-ignore: long-lambda（ast-grep 工具指令：忽略过长的 lambda 表达式）
        [asyncResp](const boost::system::error_code& ec,
                    const dbus::utility::MapperGetSubTreeResponse& subtree) {
            if (ec)
            {
                BMCWEB_LOG_ERROR(
                    "DBUS response error on Power.IdlePowerSaver GetSubTree {}",
                    ec);
                messages::internalError(asyncResp->res);
                return;
            }
            if (subtree.empty())
            {
                // This is an optional interface so just return
                // if there is no instance found
                // 这是一个可选接口，如果
                // 找不到实例则直接返回
                BMCWEB_LOG_DEBUG("No instances found");
                return;
            }
            if (subtree.size() > 1)
            {
                // More then one PowerIdlePowerSaver object is not supported and
                // is an error
                // 不支持多于一个 PowerIdlePowerSaver 对象，
                // 这是一个错误
                BMCWEB_LOG_DEBUG(
                    "Found more than 1 system D-Bus Power.IdlePowerSaver objects: {}",
                    subtree.size());
                messages::internalError(asyncResp->res);
                return;
            }
            if ((subtree[0].first.empty()) || (subtree[0].second.size() != 1))
            {
                BMCWEB_LOG_DEBUG("Power.IdlePowerSaver mapper error!");
                messages::internalError(asyncResp->res);
                return;
            }
            const std::string& path = subtree[0].first;
            const std::string& service = subtree[0].second.begin()->first;
            if (service.empty())
            {
                BMCWEB_LOG_DEBUG("Power.IdlePowerSaver service mapper error!");
                messages::internalError(asyncResp->res);
                return;
            }

            // Valid IdlePowerSaver object found, now read the current values
            // 找到有效的 IdlePowerSaver 对象，现在读取当前值
            dbus::utility::getAllProperties(
                *crow::connections::systemBus, service, path,
                "xyz.openbmc_project.Control.Power.IdlePowerSaver",
                // ast-grep-ignore: long-lambda
                // ast-grep-ignore: long-lambda（ast-grep 工具指令：忽略过长的 lambda 表达式）
                [asyncResp](
                    const boost::system::error_code& ec2,
                    const dbus::utility::DBusPropertiesMap& properties) {
                    if (ec2)
                    {
                        BMCWEB_LOG_ERROR(
                            "DBUS response error on IdlePowerSaver GetAll: {}",
                            ec2);
                        messages::internalError(asyncResp->res);
                        return;
                    }

                    if (!parseIpsProperties(asyncResp, properties))
                    {
                        messages::internalError(asyncResp->res);
                        return;
                    }
                });
        });

    BMCWEB_LOG_DEBUG("EXIT: Get idle power saver parameters");
}

/**
 * @brief Sets Idle Power Saver properties.
 *
 * @param[in] asyncResp  Shared pointer for generating response message.
 * @param[in] ipsEnable  The IPS Enable value (true/false) from incoming
 *                       RF request.
 * @param[in] ipsEnterUtil The utilization limit to enter idle state.
 * @param[in] ipsEnterTime The time the utilization must be below ipsEnterUtil
 * before entering idle state.
 * @param[in] ipsExitUtil The utilization limit when exiting idle state.
 * @param[in] ipsExitTime The time the utilization must be above ipsExutUtil
 * before exiting idle state
 *
 * @return None.
 */
/* @brief 设置 Idle Power Saver（空闲节能）属性。
 * @param[in] asyncResp  用于生成响应消息的共享指针。
 * @param[in] ipsEnable  来自传入 RF 请求的 IPS 使能值（true/false）。
 * @param[in] ipsEnterUtil 进入空闲状态的利用率阈值。
 * @param[in] ipsEnterTime 进入空闲状态前利用率必须低于 ipsEnterUtil
 * 的时长。
 * @param[in] ipsExitUtil 退出空闲状态时的利用率阈值。
 * @param[in] ipsExitTime 退出空闲状态前利用率必须高于 ipsExutUtil
 * 的时长
 * @return 无。 */
inline void setIdlePowerSaver(
    const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
    const std::optional<bool> ipsEnable,
    const std::optional<uint8_t> ipsEnterUtil,
    const std::optional<uint64_t> ipsEnterTime,
    const std::optional<uint8_t> ipsExitUtil,
    const std::optional<uint64_t> ipsExitTime)
{
    BMCWEB_LOG_DEBUG("Set idle power saver properties");

    // Get IdlePowerSaver object path:
    // 获取 IdlePowerSaver 对象路径：
    constexpr std::array<std::string_view, 1> interfaces = {
        "xyz.openbmc_project.Control.Power.IdlePowerSaver"};
    dbus::utility::getSubTree(
        "/", 0, interfaces,
        // ast-grep-ignore: long-lambda
        // ast-grep-ignore: long-lambda（ast-grep 工具指令：忽略过长的 lambda 表达式）
        [asyncResp, ipsEnable, ipsEnterUtil, ipsEnterTime, ipsExitUtil,
         ipsExitTime](const boost::system::error_code& ec,
                      const dbus::utility::MapperGetSubTreeResponse& subtree) {
            if (ec)
            {
                BMCWEB_LOG_ERROR(
                    "DBUS response error on Power.IdlePowerSaver GetSubTree {}",
                    ec);
                messages::internalError(asyncResp->res);
                return;
            }
            if (subtree.empty())
            {
                // This is an optional D-Bus object, but user attempted to patch
                // 这是一个可选的 D-Bus 对象，但用户尝试对其进行 patch
                messages::resourceNotFound(asyncResp->res, "ComputerSystem",
                                           "IdlePowerSaver");
                return;
            }
            if (subtree.size() > 1)
            {
                // More then one PowerIdlePowerSaver object is not supported and
                // is an error
                // 不支持多于一个 PowerIdlePowerSaver 对象，
                // 这是一个错误
                BMCWEB_LOG_DEBUG(
                    "Found more than 1 system D-Bus Power.IdlePowerSaver objects: {}",
                    subtree.size());
                messages::internalError(asyncResp->res);
                return;
            }
            if ((subtree[0].first.empty()) || (subtree[0].second.size() != 1))
            {
                BMCWEB_LOG_DEBUG("Power.IdlePowerSaver mapper error!");
                messages::internalError(asyncResp->res);
                return;
            }
            const std::string& path = subtree[0].first;
            const std::string& service = subtree[0].second.begin()->first;
            if (service.empty())
            {
                BMCWEB_LOG_DEBUG("Power.IdlePowerSaver service mapper error!");
                messages::internalError(asyncResp->res);
                return;
            }

            // Valid Power IdlePowerSaver object found, now set any values that
            // need to be updated
            // 找到有效的 Power IdlePowerSaver 对象，现在设置所有
            // 需要更新的值

            if (ipsEnable)
            {
                setDbusProperty(
                    asyncResp, "IdlePowerSaver/Enabled", service, path,
                    "xyz.openbmc_project.Control.Power.IdlePowerSaver",
                    "Enabled", *ipsEnable);
            }
            if (ipsEnterUtil)
            {
                setDbusProperty(
                    asyncResp, "IdlePowerSaver/EnterUtilizationPercent",
                    service, path,
                    "xyz.openbmc_project.Control.Power.IdlePowerSaver",
                    "EnterUtilizationPercent", *ipsEnterUtil);
            }
            if (ipsEnterTime)
            {
                // Convert from seconds into milliseconds for DBus
                // 将秒转换为毫秒以供 DBus 使用
                const uint64_t timeMilliseconds = *ipsEnterTime * 1000;
                setDbusProperty(
                    asyncResp, "IdlePowerSaver/EnterDwellTimeSeconds", service,
                    path, "xyz.openbmc_project.Control.Power.IdlePowerSaver",
                    "EnterDwellTime", timeMilliseconds);
            }
            if (ipsExitUtil)
            {
                setDbusProperty(
                    asyncResp, "IdlePowerSaver/ExitUtilizationPercent", service,
                    path, "xyz.openbmc_project.Control.Power.IdlePowerSaver",
                    "ExitUtilizationPercent", *ipsExitUtil);
            }
            if (ipsExitTime)
            {
                // Convert from seconds into milliseconds for DBus
                // 将秒转换为毫秒以供 DBus 使用
                const uint64_t timeMilliseconds = *ipsExitTime * 1000;
                setDbusProperty(
                    asyncResp, "IdlePowerSaver/ExitDwellTimeSeconds", service,
                    path, "xyz.openbmc_project.Control.Power.IdlePowerSaver",
                    "ExitDwellTime", timeMilliseconds);
            }
        });

    BMCWEB_LOG_DEBUG("EXIT: Set idle power saver parameters");
}

inline void handleComputerSystemCollectionHead(
    crow::App& app, const crow::Request& req,
    const std::shared_ptr<bmcweb::AsyncResp>& asyncResp)
{
    if (!redfish::setUpRedfishRoute(app, req, asyncResp))
    {
        return;
    }
    asyncResp->res.addHeader(
        boost::beast::http::field::link,
        "</redfish/v1/JsonSchemas/ComputerSystemCollection/ComputerSystemCollection.json>; rel=describedby");
}

inline void handleComputerSystemCollectionGet(
    crow::App& app, const crow::Request& req,
    const std::shared_ptr<bmcweb::AsyncResp>& asyncResp)
{
    if (!redfish::setUpRedfishRoute(app, req, asyncResp))
    {
        return;
    }

    asyncResp->res.addHeader(
        boost::beast::http::field::link,
        "</redfish/v1/JsonSchemas/ComputerSystemCollection.json>; rel=describedby");
    asyncResp->res.jsonValue["@odata.type"] =
        "#ComputerSystemCollection.ComputerSystemCollection";
    asyncResp->res.jsonValue["@odata.id"] = "/redfish/v1/Systems";
    asyncResp->res.jsonValue["Name"] = "Computer System Collection";

    systems_utils::getSystemCollectionMembers(asyncResp);
}

/**
 * Function transceives data with dbus directly.
 */
/* 该函数直接与 dbus 收发数据。 */
inline void doNMI(const std::shared_ptr<bmcweb::AsyncResp>& asyncResp)
{
    constexpr const char* serviceName = "xyz.openbmc_project.Control.Host.NMI";
    constexpr const char* objectPath = "/xyz/openbmc_project/control/host0/nmi";
    constexpr const char* interfaceName =
        "xyz.openbmc_project.Control.Host.NMI";
    constexpr const char* method = "NMI";

    dbus::utility::async_method_call(
        asyncResp,
        [asyncResp](const boost::system::error_code& ec) {
            if (ec)
            {
                BMCWEB_LOG_ERROR(" Bad D-Bus request error: {}", ec);
                messages::internalError(asyncResp->res);
                return;
            }
            messages::success(asyncResp->res);
        },
        serviceName, objectPath, interfaceName, method);
}

/**
 * @brief process the POST request after getting the computerSystemIndex
 *
 * @param[in] asyncResp           Shared pointer for completing asynchronous
 *                                calls
 * @param[in] resetType           The requested reset action
 * @param[in] computerSystemIndex Index associated with the requested system
 *
 * @return None
 */
/* @brief 在获取到 computerSystemIndex 后处理 POST 请求
 * @param[in] asyncResp           用于完成异步调用的共享指针
 * @param[in] resetType           请求的复位动作
 * @param[in] computerSystemIndex 与所请求系统关联的索引
 * @return 无 */
inline void processComputerSystemResetActionPost(
    const std::shared_ptr<bmcweb::AsyncResp>& asyncResp, std::string& resetType,
    const uint64_t computerSystemIndex)
{
    // Get the command and host vs. chassis
    // 获取命令以及是主机（host）还是机箱（chassis）
    std::string command;
    bool hostCommand = true;
    if ((resetType == "On") || (resetType == "ForceOn"))
    {
        command = "xyz.openbmc_project.State.Host.Transition.On";
        hostCommand = true;
    }
    else if (resetType == "ForceOff")
    {
        command = "xyz.openbmc_project.State.Chassis.Transition.Off";
        hostCommand = false;
    }
    else if (resetType == "ForceRestart")
    {
        command = "xyz.openbmc_project.State.Host.Transition.ForceWarmReboot";
        hostCommand = true;
    }
    else if (resetType == "GracefulShutdown")
    {
        command = "xyz.openbmc_project.State.Host.Transition.Off";
        hostCommand = true;
    }
    else if (resetType == "GracefulRestart")
    {
        command =
            "xyz.openbmc_project.State.Host.Transition.GracefulWarmReboot";
        hostCommand = true;
    }
    else if (resetType == "PowerCycle")
    {
        command = "xyz.openbmc_project.State.Host.Transition.Reboot";
        hostCommand = true;
    }
    else if (resetType == "Nmi")
    {
        doNMI(asyncResp);
        return;
    }
    else
    {
        messages::actionParameterValueNotInList(asyncResp->res, resetType,
                                                "ResetType", "Reset");
        return;
    }

    if (hostCommand)
    {
        setDbusProperty(
            asyncResp, "Reset",
            systems_utils::getHostStateServiceName(computerSystemIndex),
            systems_utils::getHostStateObjectPath(computerSystemIndex),
            "xyz.openbmc_project.State.Host", "RequestedHostTransition",
            command);
    }
    else
    {
        setDbusProperty(
            asyncResp, "Reset",
            systems_utils::getChassisStateServiceName(computerSystemIndex),
            systems_utils::getChassisStateObjectPath(computerSystemIndex),
            "xyz.openbmc_project.State.Chassis", "RequestedPowerTransition",
            command);
    }
}

inline void handleComputerSystemResetActionPost(
    crow::App& app, const crow::Request& req,
    const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
    const std::string& systemName)
{
    if (!redfish::setUpRedfishRoute(app, req, asyncResp))
    {
        return;
    }

    if constexpr (BMCWEB_HYPERVISOR_COMPUTER_SYSTEM)
    {
        if (systemName == "hypervisor")
        {
            handleHypervisorSystemResetPost(req, asyncResp);
            return;
        }
    }

    if (!BMCWEB_EXPERIMENTAL_REDFISH_MULTI_COMPUTER_SYSTEM)
    {
        if (systemName != BMCWEB_REDFISH_SYSTEM_URI_NAME)
        {
            messages::resourceNotFound(asyncResp->res, "ComputerSystem",
                                       systemName);
            return;
        }
    }

    std::string resetType;
    if (!json_util::readJsonAction(req, asyncResp->res, "ResetType", resetType))
    {
        return;
    }

    systems_utils::getComputerSystemIndex(
        asyncResp, systemName,
        std::bind_front(processComputerSystemResetActionPost, asyncResp,
                        resetType));
}

inline void handleComputerSystemHead(
    App& app, const crow::Request& req,
    const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
    const std::string& /*systemName*/)
{
    if (!redfish::setUpRedfishRoute(app, req, asyncResp))
    {
        return;
    }

    asyncResp->res.addHeader(
        boost::beast::http::field::link,
        "</redfish/v1/JsonSchemas/ComputerSystem/ComputerSystem.json>; rel=describedby");
}

inline void afterPortRequest(
    const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
    const boost::system::error_code& ec,
    const std::vector<std::tuple<std::string, std::string, bool>>& socketData)
{
    if (ec)
    {
        BMCWEB_LOG_ERROR("DBUS response error {}", ec);
        messages::internalError(asyncResp->res);
        return;
    }
    for (const auto& data : socketData)
    {
        const std::string& socketPath = get<0>(data);
        const std::string& protocolName = get<1>(data);
        bool isProtocolEnabled = get<2>(data);
        nlohmann::json& dataJson = asyncResp->res.jsonValue["SerialConsole"];
        dataJson[protocolName]["ServiceEnabled"] = isProtocolEnabled;
        // need to retrieve port number for
        // obmc-console-ssh service
        // 需要获取
        // obmc-console-ssh 服务的端口号
        if (protocolName == "SSH")
        {
            // ast-grep-ignore: long-lambda
            // ast-grep-ignore: long-lambda（ast-grep 工具指令：忽略过长的 lambda 表达式）
            getPortNumber(socketPath, [asyncResp, protocolName](
                                          const boost::system::error_code& ec1,
                                          int portNumber) {
                if (ec1.value() ==
                    boost::system::errc::no_such_file_or_directory)
                {
                    BMCWEB_LOG_WARNING("No ssh service found");
                    return;
                }
                if (ec1)
                {
                    BMCWEB_LOG_ERROR("DBUS response error {}", ec1);
                    messages::internalError(asyncResp->res);
                    return;
                }

                nlohmann::json& dataJson1 =
                    asyncResp->res.jsonValue["SerialConsole"];
                dataJson1[protocolName]["Port"] = portNumber;
            });
        }
    }
}

/**
 * @brief process the GET request after getting the computerSystemIndex
 *
 * @param[in] asyncResp           Shared pointer for completing asynchronous
 *                                calls
 * @param[in] systemName          Name of the requested system
 * @param[in] computerSystemIndex Index associated with the requested system
 *
 * @return None
 */
/* @brief 在获取到 computerSystemIndex 后处理 GET 请求
 * @param[in] asyncResp           用于完成异步调用的共享指针
 * @param[in] systemName          所请求系统的名称
 * @param[in] computerSystemIndex 与所请求系统关联的索引
 * @return 无 */
inline void processComputerSystemGet(
    const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
    const std::string& systemName, const uint64_t computerSystemIndex)
{
    asyncResp->res.addHeader(
        boost::beast::http::field::link,
        "</redfish/v1/JsonSchemas/ComputerSystem/ComputerSystem.json>; rel=describedby");
    asyncResp->res.jsonValue["@odata.type"] =
        "#ComputerSystem.v1_22_0.ComputerSystem";
    asyncResp->res.jsonValue["Name"] = systemName;
    asyncResp->res.jsonValue["Id"] = systemName;
    asyncResp->res.jsonValue["SystemType"] =
        computer_system::SystemType::Physical;
    asyncResp->res.jsonValue["Description"] = "Computer System";
    asyncResp->res.jsonValue["ProcessorSummary"]["Count"] = 0;
    asyncResp->res.jsonValue["MemorySummary"]["TotalSystemMemoryGiB"] =
        double(0);
    asyncResp->res.jsonValue["@odata.id"] =
        boost::urls::format("/redfish/v1/Systems/{}", systemName);

    // Currently not supported on multi-host. TBD
    // 目前多主机（multi-host）不支持。TBD
    if constexpr (!BMCWEB_EXPERIMENTAL_REDFISH_MULTI_COMPUTER_SYSTEM)
    {
        asyncResp->res.jsonValue["Bios"]["@odata.id"] =
            boost::urls::format("/redfish/v1/Systems/{}/Bios", systemName);
        asyncResp->res.jsonValue["Processors"]["@odata.id"] =
            boost::urls::format("/redfish/v1/Systems/{}/Processors",
                                systemName);
        asyncResp->res.jsonValue["Memory"]["@odata.id"] =
            boost::urls::format("/redfish/v1/Systems/{}/Memory", systemName);
        asyncResp->res.jsonValue["Storage"]["@odata.id"] =
            boost::urls::format("/redfish/v1/Systems/{}/Storage", systemName);
        asyncResp->res.jsonValue["FabricAdapters"]["@odata.id"] =
            boost::urls::format("/redfish/v1/Systems/{}/FabricAdapters",
                                systemName);
    }

    asyncResp->res.jsonValue["Actions"]["#ComputerSystem.Reset"]["target"] =
        boost::urls::format(
            "/redfish/v1/Systems/{}/Actions/ComputerSystem.Reset", systemName);
    asyncResp->res
        .jsonValue["Actions"]["#ComputerSystem.Reset"]["@Redfish.ActionInfo"] =
        boost::urls::format("/redfish/v1/Systems/{}/ResetActionInfo",
                            systemName);

    // The OpenBMC EventLog implementation currently
    // does not support per-system entries.
    // OpenBMC 的 EventLog 实现目前
    // 不支持按系统区分的条目。
    if constexpr (BMCWEB_REDFISH_EVENTLOG_LOCATION == "systems" &&
                  !BMCWEB_EXPERIMENTAL_REDFISH_MULTI_COMPUTER_SYSTEM)
    {
        asyncResp->res.jsonValue["LogServices"]["@odata.id"] =
            boost::urls::format("/redfish/v1/Systems/{}/LogServices",
                                systemName);
    }

    nlohmann::json::array_t managedBy;
    nlohmann::json& manager = managedBy.emplace_back();
    manager["@odata.id"] = boost::urls::format("/redfish/v1/Managers/{}",
                                               BMCWEB_REDFISH_MANAGER_URI_NAME);
    asyncResp->res.jsonValue["Links"]["ManagedBy"] = std::move(managedBy);
    asyncResp->res.jsonValue["Status"]["Health"] = resource::Health::OK;
    asyncResp->res.jsonValue["Status"]["State"] = resource::State::Enabled;

    // Fill in SerialConsole info
    // 填充 SerialConsole（串口控制台）信息
    asyncResp->res.jsonValue["SerialConsole"]["MaxConcurrentSessions"] = 15;
    asyncResp->res.jsonValue["SerialConsole"]["IPMI"]["ServiceEnabled"] = true;

    asyncResp->res.jsonValue["SerialConsole"]["SSH"]["ServiceEnabled"] = true;
    asyncResp->res.jsonValue["SerialConsole"]["SSH"]["Port"] = 2200;
    asyncResp->res.jsonValue["SerialConsole"]["SSH"]["HotKeySequenceDisplay"] =
        "Press ~. to exit console";
    getPortStatusAndPath(std::span{protocolToDBusForSystems},
                         std::bind_front(afterPortRequest, asyncResp));

    if constexpr (BMCWEB_KVM)
    {
        // Fill in GraphicalConsole info
        // 填充 GraphicalConsole（图形控制台）信息
        asyncResp->res.jsonValue["GraphicalConsole"]["ServiceEnabled"] = true;
        asyncResp->res.jsonValue["GraphicalConsole"]["MaxConcurrentSessions"] =
            4;
        asyncResp->res.jsonValue["GraphicalConsole"]["ConnectTypesSupported"] =
            nlohmann::json::array_t({"KVMIP"});
    }

    if constexpr (BMCWEB_REDFISH_USE_HARDCODED_SYSTEM_LOCATION_INDICATOR)
    {
        getSystemLocationIndicatorActive(asyncResp);
    }
    else
    {
        systems_utils::getValidSystemsPath(
            asyncResp, systemName,
            [asyncResp,
             systemName](const std::optional<std::string>& validSystemsPath) {
                if (validSystemsPath)
                {
                    getLocationIndicatorActive(asyncResp, *validSystemsPath);
                }
            });
    }

    // Currently not supported on multi-host.
    // 目前多主机（multi-host）不支持。
    if constexpr (!BMCWEB_EXPERIMENTAL_REDFISH_MULTI_COMPUTER_SYSTEM)
    {
        getComputerSystem(asyncResp);
        // Todo: chassis matching could be handled by patch
        // https://gerrit.openbmc.org/c/openbmc/bmcweb/+/60793
        // Todo：机箱匹配可以通过 patch 来处理
        // https://gerrit.openbmc.org/c/openbmc/bmcweb/+/60793
        getMainChassisId(
            asyncResp, [](const std::string& chassisId,
                          const std::shared_ptr<bmcweb::AsyncResp>& aRsp) {
                nlohmann::json::array_t chassisArray;
                nlohmann::json& chassis = chassisArray.emplace_back();
                chassis["@odata.id"] =
                    boost::urls::format("/redfish/v1/Chassis/{}", chassisId);
                aRsp->res.jsonValue["Links"]["Chassis"] =
                    std::move(chassisArray);
            });

        pcie_util::getPCIeDeviceList(
            asyncResp, nlohmann::json::json_pointer("/PCIeDevices"));
    }
    getHostState(asyncResp, computerSystemIndex);
    getBootProperties(asyncResp, computerSystemIndex);
    getBootProgress(asyncResp, computerSystemIndex);
    getBootProgressLastStateTime(asyncResp, computerSystemIndex);
    getHostWatchdogTimer(asyncResp);
    getPowerRestorePolicy(asyncResp, computerSystemIndex);
    getStopBootOnFault(asyncResp);
    getAutomaticRetryPolicy(asyncResp, computerSystemIndex);
    getLastResetTime(asyncResp, computerSystemIndex);
    if constexpr (BMCWEB_REDFISH_PROVISIONING_FEATURE)
    {
        getProvisioningStatus(asyncResp);
    }
    getTrustedModuleRequiredToBoot(asyncResp, computerSystemIndex);
    getPowerMode(asyncResp);
    getIdlePowerSaver(asyncResp);
}

inline void handleComputerSystemGet(
    crow::App& app, const crow::Request& req,
    const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
    const std::string& systemName)
{
    if (!redfish::setUpRedfishRoute(app, req, asyncResp))
    {
        return;
    }

    if constexpr (BMCWEB_HYPERVISOR_COMPUTER_SYSTEM)
    {
        if (systemName == "hypervisor")
        {
            handleHypervisorSystemGet(asyncResp);
            return;
        }
    }

    if constexpr (!BMCWEB_EXPERIMENTAL_REDFISH_MULTI_COMPUTER_SYSTEM)
    {
        if (systemName != BMCWEB_REDFISH_SYSTEM_URI_NAME)
        {
            messages::resourceNotFound(asyncResp->res, "ComputerSystem",
                                       systemName);
            return;
        }
    }

    BMCWEB_LOG_DEBUG("requested system = {}", systemName);
    systems_utils::getComputerSystemIndex(
        asyncResp, systemName,
        std::bind_front(processComputerSystemGet, asyncResp, systemName));
}

struct PatchParams
{
    std::optional<bool> locationIndicatorActive;
    std::optional<std::string> assetTag;
    std::optional<std::string> powerRestorePolicy;
    std::optional<std::string> powerMode;
    std::optional<bool> wdtEnable;
    std::optional<std::string> wdtTimeOutAction;
    std::optional<std::string> bootSource;
    std::optional<std::string> bootType;
    std::optional<std::string> bootEnable;
    std::optional<std::string> bootAutomaticRetry;
    std::optional<uint32_t> bootAutomaticRetryAttempts;
    std::optional<bool> bootTrustedModuleRequired;
    std::optional<std::string> stopBootOnFault;
    std::optional<bool> ipsEnable;
    std::optional<uint8_t> ipsEnterUtil;
    std::optional<uint64_t> ipsEnterTime;
    std::optional<uint8_t> ipsExitUtil;
    std::optional<uint64_t> ipsExitTime;
};

/**
 * @brief process the POST request after getting the computerSystemIndex
 *
 * @param[in] asyncResp           Shared pointer for completing asynchronous
 *                                calls
 * @param[in] patchParams         Struct containing the property we want to
 *                                patch
 * @param[in] computerSystemIndex Index associated with the requested system
 *
 * @return None
 */
/* @brief 在获取到 computerSystemIndex 后处理 POST 请求
 * @param[in] asyncResp           用于完成异步调用的共享指针
 * @param[in] patchParams         包含我们要 patch 的属性的结构体
 * @param[in] computerSystemIndex 与所请求系统关联的索引
 * @return 无 */

inline void processComputerSystemPatch(
    const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
    std::string& systemName, PatchParams& patchParams,
    const uint64_t computerSystemIndex)
{
    if (patchParams.assetTag)
    {
        setAssetTag(asyncResp, *patchParams.assetTag);
    }

    if (patchParams.wdtEnable || patchParams.wdtTimeOutAction)
    {
        setWDTProperties(asyncResp, patchParams.wdtEnable,
                         patchParams.wdtTimeOutAction);
    }

    if (patchParams.bootSource || patchParams.bootType ||
        patchParams.bootEnable)
    {
        setBootProperties(asyncResp, computerSystemIndex,
                          patchParams.bootSource, patchParams.bootType,
                          patchParams.bootEnable);
    }
    if (patchParams.bootAutomaticRetry)
    {
        setAutomaticRetry(asyncResp, computerSystemIndex,
                          *patchParams.bootAutomaticRetry);
    }

    if (patchParams.bootAutomaticRetryAttempts)
    {
        setAutomaticRetryAttempts(
            asyncResp, computerSystemIndex,
            patchParams.bootAutomaticRetryAttempts.value());
    }

    if (patchParams.bootTrustedModuleRequired)
    {
        setTrustedModuleRequiredToBoot(asyncResp, computerSystemIndex,
                                       *patchParams.bootTrustedModuleRequired);
    }

    if (patchParams.stopBootOnFault)
    {
        setStopBootOnFault(asyncResp, *patchParams.stopBootOnFault);
    }

    if (patchParams.locationIndicatorActive)
    {
        if constexpr (BMCWEB_REDFISH_USE_HARDCODED_SYSTEM_LOCATION_INDICATOR)
        {
            setSystemLocationIndicatorActive(
                asyncResp, *patchParams.locationIndicatorActive);
        }
        else
        {
            systems_utils::getValidSystemsPath(
                asyncResp, systemName,
                // ast-grep-ignore: long-lambda
                // ast-grep-ignore: long-lambda（ast-grep 工具指令：忽略过长的 lambda 表达式）
                [asyncResp, systemName,
                 locationIndicatorActive{*patchParams.locationIndicatorActive}](
                    const std::optional<std::string>& validSystemsPath) {
                    if (!validSystemsPath)
                    {
                        messages::resourceNotFound(asyncResp->res, "Systems",
                                                   systemName);
                        return;
                    }
                    setLocationIndicatorActive(asyncResp, *validSystemsPath,
                                               locationIndicatorActive);
                });
        }
    }

    if (patchParams.powerRestorePolicy)
    {
        setPowerRestorePolicy(asyncResp, computerSystemIndex,
                              *patchParams.powerRestorePolicy);
    }

    if (patchParams.powerMode)
    {
        setPowerMode(asyncResp, *patchParams.powerMode);
    }

    if (patchParams.ipsEnable || patchParams.ipsEnterUtil ||
        patchParams.ipsEnterTime || patchParams.ipsExitUtil ||
        patchParams.ipsExitTime)
    {
        setIdlePowerSaver(asyncResp, patchParams.ipsEnable,
                          patchParams.ipsEnterUtil, patchParams.ipsEnterTime,
                          patchParams.ipsExitUtil, patchParams.ipsExitTime);
    }
}

inline void handleComputerSystemPatch(
    crow::App& app, const crow::Request& req,
    const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
    const std::string& systemName)
{
    PatchParams patchParams;

    if (!redfish::setUpRedfishRoute(app, req, asyncResp))
    {
        return;
    }

    if constexpr (!BMCWEB_EXPERIMENTAL_REDFISH_MULTI_COMPUTER_SYSTEM)
    {
        if (systemName != BMCWEB_REDFISH_SYSTEM_URI_NAME)
        {
            messages::resourceNotFound(asyncResp->res, "ComputerSystem",
                                       systemName);
            return;
        }
    }

    asyncResp->res.addHeader(
        boost::beast::http::field::link,
        "</redfish/v1/JsonSchemas/ComputerSystem/ComputerSystem.json>; rel=describedby");

    if (!json_util::readJsonPatch(
            req, asyncResp->res,                                              //
            "AssetTag", patchParams.assetTag,                                 //
            "Boot/AutomaticRetryAttempts",
            patchParams.bootAutomaticRetryAttempts,                           //
            "Boot/AutomaticRetryConfig", patchParams.bootAutomaticRetry,      //
            "Boot/BootSourceOverrideEnabled", patchParams.bootEnable,         //
            "Boot/BootSourceOverrideMode", patchParams.bootType,              //
            "Boot/BootSourceOverrideTarget", patchParams.bootSource,          //
            "Boot/StopBootOnFault", patchParams.stopBootOnFault,              //
            "Boot/TrustedModuleRequiredToBoot",
            patchParams.bootTrustedModuleRequired,                            //
            "HostWatchdogTimer/FunctionEnabled", patchParams.wdtEnable,       //
            "HostWatchdogTimer/TimeoutAction", patchParams.wdtTimeOutAction,  //
            "IdlePowerSaver/Enabled", patchParams.ipsEnable,                  //
            "IdlePowerSaver/EnterDwellTimeSeconds", patchParams.ipsEnterTime, //
            "IdlePowerSaver/EnterUtilizationPercent",
            patchParams.ipsEnterUtil,                                         //
            "IdlePowerSaver/ExitDwellTimeSeconds", patchParams.ipsExitTime,   //
            "IdlePowerSaver/ExitUtilizationPercent", patchParams.ipsExitUtil, //
            "LocationIndicatorActive", patchParams.locationIndicatorActive,   //
            "PowerMode", patchParams.powerMode,                               //
            "PowerRestorePolicy", patchParams.powerRestorePolicy))
    {
        return;
    }

    systems_utils::getComputerSystemIndex(
        asyncResp, systemName,
        std::bind_front(processComputerSystemPatch, asyncResp, systemName,
                        patchParams));
}

inline void handleSystemCollectionResetActionHead(
    crow::App& app, const crow::Request& req,
    const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
    const std::string& /*systemName*/)
{
    if (!redfish::setUpRedfishRoute(app, req, asyncResp))
    {
        return;
    }
    asyncResp->res.addHeader(
        boost::beast::http::field::link,
        "</redfish/v1/JsonSchemas/ActionInfo/ActionInfo.json>; rel=describedby");
}

/**
 * @brief Translates allowed host transitions to redfish string
 *
 * @param[in]  dbusAllowedHostTran The allowed host transition on dbus
 * @param[out] allowableValues     The translated host transition(s)
 *
 * @return Emplaces corresponding Redfish translated value(s) in
 * allowableValues. If translation not possible, does nothing to
 * allowableValues.
 */
/* @brief 将允许的主机状态转换转换为 Redfish 字符串
 * @param[in]  dbusAllowedHostTran dbus 上允许的主机状态转换
 * @param[out] allowableValues     转换后的主机状态转换
 * @return 在 allowableValues 中存入对应的 Redfish 转换值。
 * 如果无法转换，则不对 allowableValues 做任何操作。 */
inline void dbusToRfAllowedHostTransitions(
    const std::string& dbusAllowedHostTran,
    nlohmann::json::array_t& allowableValues)
{
    if (dbusAllowedHostTran == "xyz.openbmc_project.State.Host.Transition.On")
    {
        allowableValues.emplace_back(resource::ResetType::On);
        allowableValues.emplace_back(resource::ResetType::ForceOn);
    }
    else if (dbusAllowedHostTran ==
             "xyz.openbmc_project.State.Host.Transition.Off")
    {
        allowableValues.emplace_back(resource::ResetType::GracefulShutdown);
    }
    else if (dbusAllowedHostTran ==
             "xyz.openbmc_project.State.Host.Transition.GracefulWarmReboot")
    {
        allowableValues.emplace_back(resource::ResetType::GracefulRestart);
    }
    else if (dbusAllowedHostTran ==
             "xyz.openbmc_project.State.Host.Transition.ForceWarmReboot")
    {
        allowableValues.emplace_back(resource::ResetType::ForceRestart);
    }
    else
    {
        BMCWEB_LOG_WARNING("Unsupported host tran {}", dbusAllowedHostTran);
    }
}

inline void afterGetAllowedHostTransitions(
    const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
    const boost::system::error_code& ec,
    const std::vector<std::string>& allowedHostTransitions)
{
    nlohmann::json::array_t allowableValues;

    // Supported on all systems currently
    // 目前所有系统都支持
    allowableValues.emplace_back(resource::ResetType::ForceOff);
    allowableValues.emplace_back(resource::ResetType::PowerCycle);
    allowableValues.emplace_back(resource::ResetType::Nmi);

    if (ec)
    {
        if ((ec.value() ==
             boost::system::linux_error::bad_request_descriptor) ||
            (ec.value() == boost::asio::error::basic_errors::host_unreachable))
        {
            // Property not implemented so just return defaults
            // 该属性未实现，因此只返回默认值
            BMCWEB_LOG_DEBUG("Property not available {}", ec);
            allowableValues.emplace_back(resource::ResetType::On);
            allowableValues.emplace_back(resource::ResetType::ForceOn);
            allowableValues.emplace_back(resource::ResetType::ForceRestart);
            allowableValues.emplace_back(resource::ResetType::GracefulRestart);
            allowableValues.emplace_back(resource::ResetType::GracefulShutdown);
        }
        else
        {
            BMCWEB_LOG_ERROR("DBUS response error {}", ec);
            messages::internalError(asyncResp->res);
            return;
        }
    }
    else
    {
        for (const std::string& transition : allowedHostTransitions)
        {
            BMCWEB_LOG_DEBUG("Found allowed host tran {}", transition);
            dbusToRfAllowedHostTransitions(transition, allowableValues);
        }
    }

    nlohmann::json::object_t parameter;
    parameter["Name"] = "ResetType";
    parameter["Required"] = true;
    parameter["DataType"] = action_info::ParameterTypes::String;
    parameter["AllowableValues"] = std::move(allowableValues);
    nlohmann::json::array_t parameters;
    parameters.emplace_back(std::move(parameter));
    asyncResp->res.jsonValue["Parameters"] = std::move(parameters);
}

inline void getAllowedHostTransitions(
    const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
    const uint64_t computerSystemIndex)
{
    sdbusplus::object_path path =
        systems_utils::getHostStateObjectPath(computerSystemIndex);
    dbus::utility::getProperty<std::vector<std::string>>(
        systems_utils::getHostStateServiceName(computerSystemIndex), path,
        "xyz.openbmc_project.State.Host", "AllowedHostTransitions",
        std::bind_front(afterGetAllowedHostTransitions, asyncResp));
}

inline void handleSystemCollectionResetActionGet(
    crow::App& app, const crow::Request& req,
    const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
    const std::string& systemName)
{
    if (!redfish::setUpRedfishRoute(app, req, asyncResp))
    {
        return;
    }

    if constexpr (BMCWEB_HYPERVISOR_COMPUTER_SYSTEM)
    {
        if (systemName == "hypervisor")
        {
            handleHypervisorResetActionGet(asyncResp);
            return;
        }
    }

    if constexpr (!BMCWEB_EXPERIMENTAL_REDFISH_MULTI_COMPUTER_SYSTEM)
    {
        if (systemName != BMCWEB_REDFISH_SYSTEM_URI_NAME)
        {
            messages::resourceNotFound(asyncResp->res, "ComputerSystem",
                                       systemName);
            return;
        }
    }

    asyncResp->res.addHeader(
        boost::beast::http::field::link,
        "</redfish/v1/JsonSchemas/ActionInfo/ActionInfo.json>; rel=describedby");

    asyncResp->res.jsonValue["@odata.id"] = boost::urls::format(
        "/redfish/v1/Systems/{}/ResetActionInfo", systemName);
    asyncResp->res.jsonValue["@odata.type"] = "#ActionInfo.v1_1_2.ActionInfo";
    asyncResp->res.jsonValue["Name"] = "Reset Action Info";
    asyncResp->res.jsonValue["Id"] = "ResetActionInfo";

    // Look to see if system defines AllowedHostTransitions
    // 查看系统是否定义了 AllowedHostTransitions
    systems_utils::getComputerSystemIndex(
        asyncResp, systemName,
        std::bind_front(getAllowedHostTransitions, asyncResp));
}

/**
 * SystemResetActionInfo derived class for delivering Computer Systems
 * ResetType AllowableValues using ResetInfo schema.
 */
/* SystemResetActionInfo 派生类，用于通过 ResetInfo schema 提供
 * 计算机系统 ResetType 的 AllowableValues。 */
inline void requestRoutesSystems(App& app)
{
    BMCWEB_ROUTE(app, "/redfish/v1/Systems/")
        .privileges(redfish::privileges::headComputerSystemCollection)
        .methods(boost::beast::http::verb::head)(
            std::bind_front(handleComputerSystemCollectionHead, std::ref(app)));

    BMCWEB_ROUTE(app, "/redfish/v1/Systems/")
        .privileges(redfish::privileges::getComputerSystemCollection)
        .methods(boost::beast::http::verb::get)(
            std::bind_front(handleComputerSystemCollectionGet, std::ref(app)));

    BMCWEB_ROUTE(app, "/redfish/v1/Systems/<str>/")
        .privileges(redfish::privileges::headComputerSystem)
        .methods(boost::beast::http::verb::head)(
            std::bind_front(handleComputerSystemHead, std::ref(app)));

    BMCWEB_ROUTE(app, "/redfish/v1/Systems/<str>/")
        .privileges(redfish::privileges::getComputerSystem)
        .methods(boost::beast::http::verb::get)(
            std::bind_front(handleComputerSystemGet, std::ref(app)));

    BMCWEB_ROUTE(app, "/redfish/v1/Systems/<str>/")
        .privileges(redfish::privileges::patchComputerSystem)
        .methods(boost::beast::http::verb::patch)(
            std::bind_front(handleComputerSystemPatch, std::ref(app)));

    BMCWEB_ROUTE(app, "/redfish/v1/Systems/<str>/Actions/ComputerSystem.Reset/")
        .privileges(redfish::privileges::postComputerSystem)
        .methods(boost::beast::http::verb::post)(std::bind_front(
            handleComputerSystemResetActionPost, std::ref(app)));

    BMCWEB_ROUTE(app, "/redfish/v1/Systems/<str>/ResetActionInfo/")
        .privileges(redfish::privileges::headActionInfo)
        .methods(boost::beast::http::verb::head)(std::bind_front(
            handleSystemCollectionResetActionHead, std::ref(app)));
    BMCWEB_ROUTE(app, "/redfish/v1/Systems/<str>/ResetActionInfo/")
        .privileges(redfish::privileges::getActionInfo)
        .methods(boost::beast::http::verb::get)(std::bind_front(
            handleSystemCollectionResetActionGet, std::ref(app)));
}
} // namespace redfish
