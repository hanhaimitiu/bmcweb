// SPDX-License-Identifier: Apache-2.0
// SPDX-FileCopyrightText: Copyright OpenBMC Authors
#pragma once

#include "async_resp.hpp"
#include "dbus_utility.hpp"
#include "error_messages.hpp"
#include "generated/enums/resource.hpp"
#include "logging.hpp"

#include <asm-generic/errno.h>

#include <boost/system/error_code.hpp>
#include <nlohmann/json.hpp>

#include <functional>
#include <memory>
#include <string>

namespace redfish
{
namespace resource_utils
{

/**
 * @brief Fetches resource status.health from DBus interfaces
 *
 */
/* @brief 从 DBus 接口获取资源的 status.health */
inline void determineResourceHealth(
    const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
    const nlohmann::json::json_pointer& jsonPtr, bool functional)
{
    BMCWEB_LOG_DEBUG("determineResourceHealth");

    if (!functional)
    {
        asyncResp->res.jsonValue[jsonPtr]["Status"]["Health"] =
            resource::Health::Critical;
    }
    else
    {
        asyncResp->res.jsonValue[jsonPtr]["Status"]["Health"] =
            resource::Health::OK;
    }
}

inline void determineResourceState(
    const std::shared_ptr<bmcweb::AsyncResp>& asyncResp, bool present,
    bool available, const nlohmann::json::json_pointer& jsonPtr)
{
    BMCWEB_LOG_DEBUG("determineResourceState");

    // Absent takes priority over unavailable
    // Absent 优先于 unavailable
    if (!present)
    {
        asyncResp->res.jsonValue[jsonPtr]["Status"]["State"] =
            resource::State::Absent;
    }
    else if (!available)
    {
        asyncResp->res.jsonValue[jsonPtr]["Status"]["State"] =
            resource::State::UnavailableOffline;
    }
    else
    {
        asyncResp->res.jsonValue[jsonPtr]["Status"]["State"] =
            resource::State::Enabled;
    }
}

inline void getStatusAvailableState(
    const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
    const nlohmann::json::json_pointer& jsonPtr, bool present,
    const boost::system::error_code& ec, bool available)
{
    if (ec)
    {
        if (ec.value() != EBADR)
        {
            BMCWEB_LOG_ERROR("DBUS response error for {}, ec {}", "Available",
                             ec.value());
            messages::internalError(asyncResp->res);
            return;
        }
        available = true;
    }
    determineResourceState(asyncResp, present, available, jsonPtr);
}

inline void getStatusPresentState(
    const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
    const std::string& service, const std::string& path,
    const nlohmann::json::json_pointer& jsonPtr,
    const boost::system::error_code& ec, bool present)
{
    if (ec)
    {
        if (ec.value() != EBADR)
        {
            BMCWEB_LOG_ERROR("DBUS response error for {}, ec {}", "Present",
                             ec.value());
            messages::internalError(asyncResp->res);
            return;
        }
        // Interface is missing, default to true for Enabled
        // 缺少接口时，Enabled 默认为 true
        present = true;
    }
    dbus::utility::getProperty<bool>(
        *crow::connections::systemBus, service, path,
        "xyz.openbmc_project.State.Decorator.Availability", "Available",
        std::bind_front(getStatusAvailableState, asyncResp, jsonPtr, present));
}

inline void getResourceState(
    const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
    const std::string& service, const std::string& path,
    const nlohmann::json::json_pointer& jsonPtr)
{
    BMCWEB_LOG_DEBUG("getResourceStatus");
    dbus::utility::getProperty<bool>(
        *crow::connections::systemBus, service, path,
        "xyz.openbmc_project.Inventory.Item", "Present",
        std::bind_front(getStatusPresentState, asyncResp, service, path,
                        jsonPtr));
}

/*
 * @brief Retrieves the status.state of the first service that implements
 * Inventory.Item or State.Decorator.Availability. If no service implements
 * these interfaces, it will default to Status.State Enabled
 *
 * @param[in] asyncResp AsyncResp object to update
 * @param[in] services Map of services to interfaces
 * @param[in] path D-Bus object path
 * @param[in] jsonPtr JSON pointer to where the parent JSON object is
 */
/* @brief 获取第一个实现 Inventory.Item 或
 *        State.Decorator.Availability 的服务的 status.state。若无服务
 *        实现这些接口，则默认为 Status.State Enabled
 * @param[in] asyncResp 要更新的 AsyncResp 对象
 * @param[in] services 服务到接口的映射
 * @param[in] path D-Bus 对象路径
 * @param[in] jsonPtr 指向父 JSON 对象的 JSON pointer */
inline void getResourceState(
    const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
    const dbus::utility::MapperServiceMap& services, const std::string& path,
    const nlohmann::json::json_pointer& jsonPtr)
{
    for (const auto& [serviceName, interfaces] : services)
    {
        for (const auto& interface : interfaces)
        {
            if (interface == "xyz.openbmc_project.Inventory.Item" ||
                interface == "xyz.openbmc_project.State.Decorator.Availability")
            {
                getResourceState(asyncResp, serviceName, path, jsonPtr);
                return;
            }
        }
    }
    asyncResp->res.jsonValue[jsonPtr]["Status"]["State"] =
        resource::State::Enabled;
}

inline void afterGetResourceHealth(
    const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
    const nlohmann::json::json_pointer& jsonPtr,
    const boost::system::error_code& ec, bool functional)
{
    if (ec)
    {
        if (ec.value() != EBADR)
        {
            BMCWEB_LOG_ERROR("DBUS response error for {}, ec {}", "Functional",
                             ec.value());
            messages::internalError(asyncResp->res);
            return;
        }
        // Interface missing, default to OK
        // 缺少接口时，默认为 OK
        functional = true;
    }
    determineResourceHealth(asyncResp, jsonPtr, functional);
}

/*
 * @brief Retrieves the status of the resource's status.health
 *
 * Queries interface:
 * - xyz.openbmc_project.State.Decorator.OperationalStatus::Functional
 */
/* @brief 获取资源 status.health 的状态
 * 查询接口：
 * - xyz.openbmc_project.State.Decorator.OperationalStatus::Functional */
inline void getResourceHealth(
    const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
    const std::string& service, const std::string& path,
    const nlohmann::json::json_pointer& jsonPtr)
{
    dbus::utility::getProperty<bool>(
        *crow::connections::systemBus, service, path,
        "xyz.openbmc_project.State.Decorator.OperationalStatus", "Functional",
        std::bind_front(afterGetResourceHealth, asyncResp, jsonPtr));
}

/*
 * @brief Retrieves the status.health of the first service that implements
 * State.Decorator.OperationalStatus. If no service implements the interface,
 * it will default to Status.Health OK
 *
 * @param[in] asyncResp AsyncResp object to update
 * @param[in] services Map of services to interfaces
 * @param[in] path D-Bus object path
 * @param[in] jsonPtr JSON pointer to where the parent JSON object is
 */
/* @brief 获取第一个实现 State.Decorator.OperationalStatus 的服务的
 *        status.health。若没有服务实现该接口，则默认为
 *        Status.Health OK
 * @param[in] asyncResp 要更新的 AsyncResp 对象
 * @param[in] services 服务到接口的映射
 * @param[in] path D-Bus 对象路径
 * @param[in] jsonPtr 指向父 JSON 对象的 JSON pointer */
inline void getResourceHealth(
    const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
    const dbus::utility::MapperServiceMap& services, const std::string& path,
    const nlohmann::json::json_pointer& jsonPtr)
{
    for (const auto& [serviceName, interfaces] : services)
    {
        for (const auto& interface : interfaces)
        {
            if (interface ==
                "xyz.openbmc_project.State.Decorator.OperationalStatus")
            {
                getResourceHealth(asyncResp, serviceName, path, jsonPtr);
                return;
            }
        }
    }
    asyncResp->res.jsonValue[jsonPtr]["Status"]["Health"] =
        resource::Health::OK;
}

} // namespace resource_utils
} // namespace redfish
