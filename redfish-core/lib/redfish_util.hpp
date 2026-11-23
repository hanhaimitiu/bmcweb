// SPDX-License-Identifier: Apache-2.0
// SPDX-FileCopyrightText: Copyright OpenBMC Authors
// SPDX-FileCopyrightText: Copyright 2019 Intel Corporation
#pragma once

#include "async_resp.hpp"
#include "boost_formatters.hpp"
#include "dbus_singleton.hpp"
#include "dbus_utility.hpp"
#include "error_messages.hpp"
#include "logging.hpp"
#include "utils/chassis_utils.hpp"

#include <boost/system/errc.hpp>
#include <boost/system/error_code.hpp>
#include <sdbusplus/message/native_types.hpp>

#include <algorithm>
#include <charconv>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <ranges>
#include <span>
#include <string>
#include <string_view>
#include <system_error>
#include <tuple>
#include <utility>
#include <vector>

namespace redfish
{

enum NetworkProtocolUnitStructFields
{
    NET_PROTO_UNIT_NAME,
    NET_PROTO_UNIT_DESC,
    NET_PROTO_UNIT_LOAD_STATE,
    NET_PROTO_UNIT_ACTIVE_STATE,
    NET_PROTO_UNIT_SUB_STATE,
    NET_PROTO_UNIT_DEVICE,
    NET_PROTO_UNIT_OBJ_PATH,
    NET_PROTO_UNIT_ALWAYS_0,
    NET_PROTO_UNIT_ALWAYS_EMPTY,
    NET_PROTO_UNIT_ALWAYS_ROOT_PATH
};

enum NetworkProtocolListenResponseElements
{
    NET_PROTO_LISTEN_TYPE,
    NET_PROTO_LISTEN_STREAM
};

/**
 * @brief D-Bus Unit structure returned in array from ListUnits Method
 */
/* @brief ListUnits 方法返回的数组中包含的 D-Bus Unit 结构体。 */
using UnitStruct =
    std::tuple<std::string, std::string, std::string, std::string, std::string,
               std::string, sdbusplus::object_path, uint32_t, std::string,
               sdbusplus::object_path>;

template <typename CallbackFunc>
void getMainChassisId(const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
                      CallbackFunc&& callback)
{
    // Find managed chassis
    // 查找受管的 chassis。
    dbus::utility::getSubTree(
        "/xyz/openbmc_project/inventory", 0, chassisInterfaces,
        // ast-grep-ignore: long-lambda
        // ast-grep-ignore: long-lambda（忽略指令：ast-grep 跳过过长 lambda 检查）
        [callback = std::forward<CallbackFunc>(callback),
         asyncResp](const boost::system::error_code& ec,
                    const dbus::utility::MapperGetSubTreeResponse& subtree) {
            if (ec)
            {
                BMCWEB_LOG_ERROR("{}", ec);
                return;
            }
            if (subtree.empty())
            {
                BMCWEB_LOG_DEBUG("Can't find chassis!");
                return;
            }

            std::size_t idPos = subtree[0].first.rfind('/');
            if (idPos == std::string::npos ||
                (idPos + 1) >= subtree[0].first.size())
            {
                messages::internalError(asyncResp->res);
                BMCWEB_LOG_DEBUG("Can't parse chassis ID!");
                return;
            }
            std::string chassisId = subtree[0].first.substr(idPos + 1);
            BMCWEB_LOG_DEBUG("chassisId = {}", chassisId);
            callback(chassisId, asyncResp);
        });
}

template <typename CallbackFunc>
void getPortStatusAndPath(
    std::span<const std::pair<std::string_view, std::string_view>>
        protocolToDBus,
    CallbackFunc&& callback)
{
    dbus::utility::async_method_call(
        // ast-grep-ignore: long-lambda
        // ast-grep-ignore: long-lambda（忽略指令：ast-grep 跳过过长 lambda 检查）
        [protocolToDBus, callback = std::forward<CallbackFunc>(callback)](
            const boost::system::error_code& ec,
            const std::vector<UnitStruct>& r) {
            std::vector<std::tuple<std::string, std::string, bool>> socketData;
            if (ec)
            {
                BMCWEB_LOG_ERROR("{}", ec);
                // return error code
                // 返回错误码。
                callback(ec, socketData);
                return;
            }

            // save all service output into vector
            // 将所有服务的输出保存到 vector。
            for (const UnitStruct& unit : r)
            {
                // Only traverse through <xyz>.socket units
                // 只遍历 <xyz>.socket 单元。
                const std::string& unitName =
                    std::get<NET_PROTO_UNIT_NAME>(unit);

                // find "." into unitsName
                // 在 unitsName 中查找 "."。
                size_t lastCharPos = unitName.rfind('.');
                if (lastCharPos == std::string::npos)
                {
                    continue;
                }

                // is unitsName end with ".socket"
                // 判断 unitsName 是否以 ".socket" 结尾。
                std::string unitNameEnd = unitName.substr(lastCharPos);
                if (unitNameEnd != ".socket")
                {
                    continue;
                }

                // find "@" into unitsName
                // 在 unitsName 中查找 "@"。
                if (size_t atCharPos = unitName.rfind('@');
                    atCharPos != std::string::npos)
                {
                    lastCharPos = atCharPos;
                }

                // unitsName without "@eth(x).socket", only <xyz>
                // unitsName without ".socket", only <xyz>
                // 去掉 "@eth(x).socket" 的 unitsName，只保留 <xyz>。
                // 去掉 ".socket" 的 unitsName，只保留 <xyz>。
                std::string unitNameStr = unitName.substr(0, lastCharPos);

                for (const auto& kv : protocolToDBus)
                {
                    // We are interested in services, which starts with
                    // mapped service name
                    // 我们关注的是以映射的服务名开头的服务。
                    if (unitNameStr != kv.second)
                    {
                        continue;
                    }

                    const std::string& socketPath =
                        std::get<NET_PROTO_UNIT_OBJ_PATH>(unit);
                    const std::string& unitState =
                        std::get<NET_PROTO_UNIT_SUB_STATE>(unit);

                    bool isProtocolEnabled = ((unitState == "running") ||
                                              (unitState == "listening"));

                    // Some protocols may have multiple services associated with
                    // them (for example IPMI). Look to see if we've already
                    // added an entry for the current protocol.
                    // 某些协议可能有多个关联的服务（例如 IPMI）。
                    // 查看是否已经为当前协议添加了条目。
                    auto find = std::ranges::find_if(
                        socketData,
                        [&kv](const std::tuple<std::string, std::string, bool>&
                                  i) { return std::get<1>(i) == kv.first; });
                    if (find != socketData.end())
                    {
                        // It only takes one enabled systemd service to consider
                        // a protocol enabled so if the current entry already
                        // has it enabled (or the new one is disabled) then just
                        // continue, otherwise remove the current one and add
                        // this new one.
                        // 只要有一个 systemd 服务启用，就认为该协议已启用。
                        // 因此，如果当前条目已启用它（或新条目被禁用），
                        // 则直接继续；否则删除当前条目并添加这个新条目。
                        if (std::get<2>(*find) || !isProtocolEnabled)
                        {
                            // Already registered as enabled or current one is
                            // not enabled, nothing to do
                            // 已注册为启用状态，或当前条目未启用，无需操作。
                            BMCWEB_LOG_DEBUG(
                                "protocolName: {}, already true or current one is false: {}",
                                kv.first, isProtocolEnabled);
                            break;
                        }
                        // Remove existing entry and replace with new one
                        // (below)
                        // 删除现有条目并替换为新条目（见下文）。
                        socketData.erase(find);
                    }

                    socketData.emplace_back(socketPath, std::string(kv.first),
                                            isProtocolEnabled);
                    // We found service, return from inner loop.
                    // 找到了服务，从内层循环返回。
                    break;
                }
            }

            callback(ec, socketData);
        },
        "org.freedesktop.systemd1", "/org/freedesktop/systemd1",
        "org.freedesktop.systemd1.Manager", "ListUnits");
}

template <typename CallbackFunc>
void getPortNumber(const std::string& socketPath, CallbackFunc&& callback)
{
    dbus::utility::getProperty<
        std::vector<std::tuple<std::string, std::string>>>(
        *crow::connections::systemBus, "org.freedesktop.systemd1", socketPath,
        "org.freedesktop.systemd1.Socket", "Listen",
        // ast-grep-ignore: long-lambda
        // ast-grep-ignore: long-lambda（忽略指令：ast-grep 跳过过长 lambda 检查）
        [callback = std::forward<CallbackFunc>(callback)](
            const boost::system::error_code& ec,
            const std::vector<std::tuple<std::string, std::string>>& resp) {
            if (ec)
            {
                BMCWEB_LOG_ERROR("{}", ec);
                callback(ec, 0);
                return;
            }
            if (resp.empty())
            {
                // Network Protocol Listen Response Elements is empty
                // Network Protocol Listen 响应元素为空。
                boost::system::error_code ec1 =
                    boost::system::errc::make_error_code(
                        boost::system::errc::no_such_file_or_directory);
                // return error code
                // 返回错误码。
                callback(ec1, 0);
                return;
            }
            const std::string& listenStream =
                std::get<NET_PROTO_LISTEN_STREAM>(resp[0]);
            const char* pa = &listenStream[listenStream.rfind(':') + 1];
            int port{0};
            if (auto [p, ec2] = std::from_chars(pa, nullptr, port);
                ec2 != std::errc())
            {
                // there is only two possibility invalid_argument and
                // result_out_of_range
                // 只有两种可能：invalid_argument 和 result_out_of_range。
                boost::system::error_code ec3 =
                    boost::system::errc::make_error_code(
                        boost::system::errc::invalid_argument);
                if (ec2 == std::errc::result_out_of_range)
                {
                    ec3 = boost::system::errc::make_error_code(
                        boost::system::errc::result_out_of_range);
                }
                // return error code
                // 返回错误码。
                callback(ec3, 0);
                BMCWEB_LOG_ERROR("{}", ec3);
            }
            callback(ec, port);
        });
}

} // namespace redfish
