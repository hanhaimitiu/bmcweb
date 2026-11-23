// SPDX-License-Identifier: Apache-2.0
// SPDX-FileCopyrightText: Copyright OpenBMC Authors
// SPDX-FileCopyrightText: Copyright 2018 Intel Corporation
#pragma once

#include "bmcweb_config.h"

#include "app.hpp"
#include "async_resp.hpp"
#include "dbus_utility.hpp"
#include "error_messages.hpp"
#include "generated/enums/ethernet_interface.hpp"
#include "generated/enums/resource.hpp"
#include "http_request.hpp"
#include "http_response.hpp"
#include "human_sort.hpp"
#include "logging.hpp"
#include "query.hpp"
#include "registries/privilege_registry.hpp"
#include "utility.hpp"
#include "utils/dbus_utils.hpp"
#include "utils/ip_utils.hpp"
#include "utils/json_utils.hpp"

#include <systemd/sd-bus.h>

#include <boost/beast/http/verb.hpp>
#include <boost/system/error_code.hpp>
#include <boost/system/result.hpp>
#include <boost/url/format.hpp>
#include <boost/url/parse.hpp>
#include <boost/url/url.hpp>
#include <boost/url/url_view.hpp>
#include <sdbusplus/message.hpp>
#include <sdbusplus/message/native_types.hpp>
#include <sdbusplus/unpack_properties.hpp>

#include <algorithm>
#include <cctype>
#include <cstddef>
#include <cstdint>
#include <format>
#include <functional>
#include <memory>
#include <optional>
#include <ranges>
#include <regex>
#include <string>
#include <string_view>
#include <utility>
#include <variant>
#include <vector>

namespace redfish
{

enum class LinkType
{
    Local,
    Global
};

enum class IpVersion
{
    IpV4,
    IpV6
};

/**
 * Structure for keeping IPv4 data required by Redfish
 */
/* 用于保存 Redfish 所需 IPv4 数据的结构体。 */
struct IPv4AddressData
{
    std::string id;
    std::string address;
    std::string domain;
    std::string gateway;
    std::string netmask;
    std::string origin;
    LinkType linktype{};
    bool isActive{};
};

/**
 * Structure for keeping IPv6 data required by Redfish
 */
/* 用于保存 Redfish 所需 IPv6 数据的结构体。 */
struct IPv6AddressData
{
    std::string id;
    std::string address;
    std::string origin;
    uint8_t prefixLength = 0;
};

/**
 * Structure for keeping static route data required by Redfish
 */
/* 用于保存 Redfish 所需静态路由数据的结构体。 */
struct StaticGatewayData
{
    std::string id;
    std::string gateway;
    size_t prefixLength = 0;
    std::string protocol;
};

/**
 * Structure for keeping basic single Ethernet Interface information
 * available from DBus
 */
/* 用于保存从 DBus 获取的基本单个以太网接口信息的结构体。 */
struct EthernetInterfaceData
{
    uint32_t speed;
    size_t mtuSize;
    bool autoNeg;
    bool fullDuplex;
    bool dnsv4Enabled;
    bool dnsv6Enabled;
    bool domainv4Enabled;
    bool domainv6Enabled;
    bool ntpv4Enabled;
    bool ntpv6Enabled;
    bool hostNamev4Enabled;
    bool hostNamev6Enabled;
    bool linkUp;
    bool nicEnabled;
    bool ipv6AcceptRa;
    std::string dhcpEnabled;
    std::string operatingMode;
    std::string hostName;
    std::string defaultGateway;
    std::string ipv6DefaultGateway;
    std::string ipv6StaticDefaultGateway;
    std::optional<std::string> macAddress;
    std::optional<uint32_t> vlanId;
    std::vector<std::string> nameServers;
    std::vector<std::string> staticNameServers;
    std::vector<std::string> domainnames;
};

struct DHCPParameters
{
    std::optional<bool> dhcpv4Enabled;
    std::optional<bool> useDnsServers;
    std::optional<bool> useNtpServers;
    std::optional<bool> useDomainName;
    std::optional<std::string> dhcpv6OperatingMode;
};

// Helper function that changes bits netmask notation (i.e. /24)
// into full dot notation
// 辅助函数，将位掩码表示法（如 /24）转换为完整的点分表示法。
inline std::string getNetmask(unsigned int bits)
{
    uint32_t value = 0xffffffff << (32 - bits);
    std::string netmask = std::to_string((value >> 24) & 0xff) + "." +
                          std::to_string((value >> 16) & 0xff) + "." +
                          std::to_string((value >> 8) & 0xff) + "." +
                          std::to_string(value & 0xff);
    return netmask;
}

inline bool translateDhcpEnabledToBool(const std::string& inputDHCP,
                                       bool isIPv4)
{
    if (isIPv4)
    {
        return (
            (inputDHCP ==
             "xyz.openbmc_project.Network.EthernetInterface.DHCPConf.v4") ||
            (inputDHCP ==
             "xyz.openbmc_project.Network.EthernetInterface.DHCPConf.both") ||
            (inputDHCP ==
             "xyz.openbmc_project.Network.EthernetInterface.DHCPConf.v4v6stateless"));
    }
    return ((inputDHCP ==
             "xyz.openbmc_project.Network.EthernetInterface.DHCPConf.v6") ||
            (inputDHCP ==
             "xyz.openbmc_project.Network.EthernetInterface.DHCPConf.both"));
}

inline std::string getDhcpEnabledEnumeration(bool isIPv4, bool isIPv6)
{
    if (isIPv4 && isIPv6)
    {
        return "xyz.openbmc_project.Network.EthernetInterface.DHCPConf.both";
    }
    if (isIPv4)
    {
        return "xyz.openbmc_project.Network.EthernetInterface.DHCPConf.v4";
    }
    if (isIPv6)
    {
        return "xyz.openbmc_project.Network.EthernetInterface.DHCPConf.v6";
    }
    return "xyz.openbmc_project.Network.EthernetInterface.DHCPConf.none";
}

inline std::string translateAddressOriginDbusToRedfish(
    const std::string& inputOrigin, bool isIPv4)
{
    if (inputOrigin == "xyz.openbmc_project.Network.IP.AddressOrigin.Static")
    {
        return "Static";
    }
    if (inputOrigin == "xyz.openbmc_project.Network.IP.AddressOrigin.LinkLocal")
    {
        if (isIPv4)
        {
            return "IPv4LinkLocal";
        }
        return "LinkLocal";
    }
    if (inputOrigin == "xyz.openbmc_project.Network.IP.AddressOrigin.DHCP")
    {
        if (isIPv4)
        {
            return "DHCP";
        }
        return "DHCPv6";
    }
    if (inputOrigin == "xyz.openbmc_project.Network.IP.AddressOrigin.SLAAC")
    {
        return "SLAAC";
    }
    return "";
}

inline bool extractEthernetInterfaceData(
    const std::string& ethifaceId,
    const dbus::utility::ManagedObjectType& dbusData,
    EthernetInterfaceData& ethData)
{
    bool idFound = false;
    for (const auto& objpath : dbusData)
    {
        for (const auto& ifacePair : objpath.second)
        {
            if (objpath.first == "/xyz/openbmc_project/network/" + ethifaceId)
            {
                idFound = true;
                if (ifacePair.first == "xyz.openbmc_project.Network.MACAddress")
                {
                    for (const auto& propertyPair : ifacePair.second)
                    {
                        if (propertyPair.first == "MACAddress")
                        {
                            const std::string* mac =
                                std::get_if<std::string>(&propertyPair.second);
                            if (mac != nullptr)
                            {
                                ethData.macAddress = *mac;
                            }
                        }
                    }
                }
                else if (ifacePair.first == "xyz.openbmc_project.Network.VLAN")
                {
                    for (const auto& propertyPair : ifacePair.second)
                    {
                        if (propertyPair.first == "Id")
                        {
                            const uint32_t* id =
                                std::get_if<uint32_t>(&propertyPair.second);
                            if (id != nullptr)
                            {
                                ethData.vlanId = *id;
                            }
                        }
                    }
                }
                else if (ifacePair.first ==
                         "xyz.openbmc_project.Network.EthernetInterface")
                {
                    for (const auto& propertyPair : ifacePair.second)
                    {
                        if (propertyPair.first == "AutoNeg")
                        {
                            const bool* autoNeg =
                                std::get_if<bool>(&propertyPair.second);
                            if (autoNeg != nullptr)
                            {
                                ethData.autoNeg = *autoNeg;
                            }
                        }
                        else if (propertyPair.first == "FullDuplex")
                        {
                            const bool* fullDuplex =
                                std::get_if<bool>(&propertyPair.second);
                            if (fullDuplex != nullptr)
                            {
                                ethData.fullDuplex = *fullDuplex;
                            }
                        }
                        else if (propertyPair.first == "Speed")
                        {
                            const uint32_t* speed =
                                std::get_if<uint32_t>(&propertyPair.second);
                            if (speed != nullptr)
                            {
                                ethData.speed = *speed;
                            }
                        }
                        else if (propertyPair.first == "MTU")
                        {
                            const size_t* mtuSize =
                                std::get_if<size_t>(&propertyPair.second);
                            if (mtuSize != nullptr)
                            {
                                ethData.mtuSize = *mtuSize;
                            }
                        }
                        else if (propertyPair.first == "LinkUp")
                        {
                            const bool* linkUp =
                                std::get_if<bool>(&propertyPair.second);
                            if (linkUp != nullptr)
                            {
                                ethData.linkUp = *linkUp;
                            }
                        }
                        else if (propertyPair.first == "NICEnabled")
                        {
                            const bool* nicEnabled =
                                std::get_if<bool>(&propertyPair.second);
                            if (nicEnabled != nullptr)
                            {
                                ethData.nicEnabled = *nicEnabled;
                            }
                        }
                        else if (propertyPair.first == "IPv6AcceptRA")
                        {
                            const bool* ipv6AcceptRa =
                                std::get_if<bool>(&propertyPair.second);
                            if (ipv6AcceptRa != nullptr)
                            {
                                ethData.ipv6AcceptRa = *ipv6AcceptRa;
                            }
                        }
                        else if (propertyPair.first == "Nameservers")
                        {
                            const std::vector<std::string>* nameservers =
                                std::get_if<std::vector<std::string>>(
                                    &propertyPair.second);
                            if (nameservers != nullptr)
                            {
                                ethData.nameServers = *nameservers;
                            }
                        }
                        else if (propertyPair.first == "StaticNameServers")
                        {
                            const std::vector<std::string>* staticNameServers =
                                std::get_if<std::vector<std::string>>(
                                    &propertyPair.second);
                            if (staticNameServers != nullptr)
                            {
                                ethData.staticNameServers = *staticNameServers;
                            }
                        }
                        else if (propertyPair.first == "DHCPEnabled")
                        {
                            const std::string* dhcpEnabled =
                                std::get_if<std::string>(&propertyPair.second);
                            if (dhcpEnabled != nullptr)
                            {
                                ethData.dhcpEnabled = *dhcpEnabled;
                            }
                        }
                        else if (propertyPair.first == "DomainName")
                        {
                            const std::vector<std::string>* domainNames =
                                std::get_if<std::vector<std::string>>(
                                    &propertyPair.second);
                            if (domainNames != nullptr)
                            {
                                ethData.domainnames = *domainNames;
                            }
                        }
                        else if (propertyPair.first == "DefaultGateway")
                        {
                            const std::string* defaultGateway =
                                std::get_if<std::string>(&propertyPair.second);
                            if (defaultGateway != nullptr)
                            {
                                std::string defaultGatewayStr = *defaultGateway;
                                if (defaultGatewayStr.empty())
                                {
                                    ethData.defaultGateway = "0.0.0.0";
                                }
                                else
                                {
                                    ethData.defaultGateway = defaultGatewayStr;
                                }
                            }
                        }
                        else if (propertyPair.first == "DefaultGateway6")
                        {
                            const std::string* defaultGateway6 =
                                std::get_if<std::string>(&propertyPair.second);
                            if (defaultGateway6 != nullptr)
                            {
                                std::string defaultGateway6Str =
                                    *defaultGateway6;
                                if (defaultGateway6Str.empty())
                                {
                                    ethData.ipv6DefaultGateway =
                                        "0:0:0:0:0:0:0:0";
                                }
                                else
                                {
                                    ethData.ipv6DefaultGateway =
                                        defaultGateway6Str;
                                }
                            }
                        }
                    }
                }
            }

            sdbusplus::object_path path("/xyz/openbmc_project/network");
            sdbusplus::object_path dhcp4Path = path / ethifaceId / "dhcp4";

            if (sdbusplus::object_path(objpath.first) == dhcp4Path)
            {
                if (ifacePair.first ==
                    "xyz.openbmc_project.Network.DHCPConfiguration")
                {
                    for (const auto& propertyPair : ifacePair.second)
                    {
                        if (propertyPair.first == "DNSEnabled")
                        {
                            const bool* dnsEnabled =
                                std::get_if<bool>(&propertyPair.second);
                            if (dnsEnabled != nullptr)
                            {
                                ethData.dnsv4Enabled = *dnsEnabled;
                            }
                        }
                        else if (propertyPair.first == "DomainEnabled")
                        {
                            const bool* domainEnabled =
                                std::get_if<bool>(&propertyPair.second);
                            if (domainEnabled != nullptr)
                            {
                                ethData.domainv4Enabled = *domainEnabled;
                            }
                        }
                        else if (propertyPair.first == "NTPEnabled")
                        {
                            const bool* ntpEnabled =
                                std::get_if<bool>(&propertyPair.second);
                            if (ntpEnabled != nullptr)
                            {
                                ethData.ntpv4Enabled = *ntpEnabled;
                            }
                        }
                        else if (propertyPair.first == "HostNameEnabled")
                        {
                            const bool* hostNameEnabled =
                                std::get_if<bool>(&propertyPair.second);
                            if (hostNameEnabled != nullptr)
                            {
                                ethData.hostNamev4Enabled = *hostNameEnabled;
                            }
                        }
                    }
                }
            }

            sdbusplus::object_path dhcp6Path = path / ethifaceId / "dhcp6";

            if (sdbusplus::object_path(objpath.first) == dhcp6Path)
            {
                if (ifacePair.first ==
                    "xyz.openbmc_project.Network.DHCPConfiguration")
                {
                    for (const auto& propertyPair : ifacePair.second)
                    {
                        if (propertyPair.first == "DNSEnabled")
                        {
                            const bool* dnsEnabled =
                                std::get_if<bool>(&propertyPair.second);
                            if (dnsEnabled != nullptr)
                            {
                                ethData.dnsv6Enabled = *dnsEnabled;
                            }
                        }
                        if (propertyPair.first == "DomainEnabled")
                        {
                            const bool* domainEnabled =
                                std::get_if<bool>(&propertyPair.second);
                            if (domainEnabled != nullptr)
                            {
                                ethData.domainv6Enabled = *domainEnabled;
                            }
                        }
                        else if (propertyPair.first == "NTPEnabled")
                        {
                            const bool* ntpEnabled =
                                std::get_if<bool>(&propertyPair.second);
                            if (ntpEnabled != nullptr)
                            {
                                ethData.ntpv6Enabled = *ntpEnabled;
                            }
                        }
                        else if (propertyPair.first == "HostNameEnabled")
                        {
                            const bool* hostNameEnabled =
                                std::get_if<bool>(&propertyPair.second);
                            if (hostNameEnabled != nullptr)
                            {
                                ethData.hostNamev6Enabled = *hostNameEnabled;
                            }
                        }
                    }
                }
            }
            // System configuration shows up in the global namespace, so no need
            // to check eth number
            // 系统配置显示在全局命名空间中，因此无需检查 eth 编号。
            if (ifacePair.first ==
                "xyz.openbmc_project.Network.SystemConfiguration")
            {
                for (const auto& propertyPair : ifacePair.second)
                {
                    if (propertyPair.first == "HostName")
                    {
                        const std::string* hostname =
                            std::get_if<std::string>(&propertyPair.second);
                        if (hostname != nullptr)
                        {
                            ethData.hostName = *hostname;
                        }
                    }
                }
            }
        }
    }
    return idFound;
}

// Helper function that extracts data for single ethernet ipv6 address
// 辅助函数，用于提取单个以太网 IPv6 地址的数据。
inline void extractIPV6Data(const std::string& ethifaceId,
                            const dbus::utility::ManagedObjectType& dbusData,
                            std::vector<IPv6AddressData>& ipv6Config)
{
    const std::string ipPathStart =
        "/xyz/openbmc_project/network/" + ethifaceId;

    // Since there might be several IPv6 configurations aligned with
    // single ethernet interface, loop over all of them
    // 由于单个以太网接口可能对应多个 IPv6 配置，
    // 因此遍历所有配置。
    for (const auto& objpath : dbusData)
    {
        // Check if proper pattern for object path appears
        // 检查对象路径是否出现了正确的模式。
        if (objpath.first.str.starts_with(ipPathStart + "/"))
        {
            for (const auto& interface : objpath.second)
            {
                if (interface.first == "xyz.openbmc_project.Network.IP")
                {
                    auto type = std::ranges::find_if(
                        interface.second, [](const auto& property) {
                            return property.first == "Type";
                        });
                    if (type == interface.second.end())
                    {
                        continue;
                    }

                    const std::string* typeStr =
                        std::get_if<std::string>(&type->second);

                    if (typeStr == nullptr ||
                        (*typeStr !=
                         "xyz.openbmc_project.Network.IP.Protocol.IPv6"))
                    {
                        continue;
                    }

                    // Instance IPv6AddressData structure, and set as
                    // appropriate
                    // 实例化 IPv6AddressData 结构体，并进行相应设置。
                    IPv6AddressData& ipv6Address = ipv6Config.emplace_back();
                    ipv6Address.id =
                        objpath.first.str.substr(ipPathStart.size());
                    for (const auto& property : interface.second)
                    {
                        if (property.first == "Address")
                        {
                            const std::string* address =
                                std::get_if<std::string>(&property.second);
                            if (address != nullptr)
                            {
                                ipv6Address.address = *address;
                            }
                        }
                        else if (property.first == "Origin")
                        {
                            const std::string* origin =
                                std::get_if<std::string>(&property.second);
                            if (origin != nullptr)
                            {
                                ipv6Address.origin =
                                    translateAddressOriginDbusToRedfish(*origin,
                                                                        false);
                            }
                        }
                        else if (property.first == "PrefixLength")
                        {
                            const uint8_t* prefix =
                                std::get_if<uint8_t>(&property.second);
                            if (prefix != nullptr)
                            {
                                ipv6Address.prefixLength = *prefix;
                            }
                        }
                        else if (property.first == "Type" ||
                                 property.first == "Gateway")
                        {
                            // Type & Gateway is not used
                            // Type 和 Gateway 未使用。
                        }
                        else
                        {
                            BMCWEB_LOG_ERROR(
                                "Got extra property: {} on the {} object",
                                property.first, objpath.first.str);
                        }
                    }
                }
            }
        }
    }
}

// Helper function that extracts data for single ethernet ipv4 address
// 辅助函数，用于提取单个以太网 IPv4 地址的数据。
inline void extractIPData(const std::string& ethifaceId,
                          const dbus::utility::ManagedObjectType& dbusData,
                          std::vector<IPv4AddressData>& ipv4Config)
{
    const std::string ipPathStart =
        "/xyz/openbmc_project/network/" + ethifaceId;

    // Since there might be several IPv4 configurations aligned with
    // single ethernet interface, loop over all of them
    // 由于单个以太网接口可能对应多个 IPv4 配置，
    // 因此遍历所有配置。
    for (const auto& objpath : dbusData)
    {
        // Check if proper pattern for object path appears
        // 检查对象路径是否出现了正确的模式。
        if (objpath.first.str.starts_with(ipPathStart + "/"))
        {
            for (const auto& interface : objpath.second)
            {
                if (interface.first == "xyz.openbmc_project.Network.IP")
                {
                    auto type = std::ranges::find_if(
                        interface.second, [](const auto& property) {
                            return property.first == "Type";
                        });
                    if (type == interface.second.end())
                    {
                        continue;
                    }

                    const std::string* typeStr =
                        std::get_if<std::string>(&type->second);

                    if (typeStr == nullptr ||
                        (*typeStr !=
                         "xyz.openbmc_project.Network.IP.Protocol.IPv4"))
                    {
                        continue;
                    }

                    // Instance IPv4AddressData structure, and set as
                    // appropriate
                    // 实例化 IPv4AddressData 结构体，并进行相应设置。
                    IPv4AddressData& ipv4Address = ipv4Config.emplace_back();
                    ipv4Address.id =
                        objpath.first.str.substr(ipPathStart.size());
                    for (const auto& property : interface.second)
                    {
                        if (property.first == "Address")
                        {
                            const std::string* address =
                                std::get_if<std::string>(&property.second);
                            if (address != nullptr)
                            {
                                ipv4Address.address = *address;
                            }
                        }
                        else if (property.first == "Origin")
                        {
                            const std::string* origin =
                                std::get_if<std::string>(&property.second);
                            if (origin != nullptr)
                            {
                                ipv4Address.origin =
                                    translateAddressOriginDbusToRedfish(*origin,
                                                                        true);
                            }
                        }
                        else if (property.first == "PrefixLength")
                        {
                            const uint8_t* mask =
                                std::get_if<uint8_t>(&property.second);
                            if (mask != nullptr)
                            {
                                // convert it to the string
                                // 将其转换为字符串。
                                ipv4Address.netmask = getNetmask(*mask);
                            }
                        }
                        else if (property.first == "Type" ||
                                 property.first == "Gateway")
                        {
                            // Type & Gateway is not used
                            // Type 和 Gateway 未使用。
                        }
                        else
                        {
                            BMCWEB_LOG_ERROR(
                                "Got extra property: {} on the {} object",
                                property.first, objpath.first.str);
                        }
                    }
                    // Check if given address is local, or global
                    // 检查给定地址是本地地址还是全局地址。
                    ipv4Address.linktype =
                        ipv4Address.address.starts_with("169.254.")
                            ? LinkType::Local
                            : LinkType::Global;
                }
            }
        }
    }
}

/**
 * @brief Modifies the default gateway assigned to the NIC
 *
 * @param[in] ifaceId     Id of network interface whose default gateway is to be
 *                        changed
 * @param[in] gateway     The new gateway value. Assigning an empty string
 *                        causes the gateway to be deleted
 * @param[io] asyncResp   Response object that will be returned to client
 *
 * @return None
 */
/* @brief 修改分配给 NIC 的默认网关。
 * @param[in] ifaceId 要修改默认网关的网络接口 Id。
 * @param[in] gateway 新的网关值。赋空字符串表示删除网关。
 * @param[io] asyncResp 将返回给客户端的响应对象。
 * @return 无。 */
inline void updateIPv4DefaultGateway(
    const std::string& ifaceId, const std::string& gateway,
    const std::shared_ptr<bmcweb::AsyncResp>& asyncResp)
{
    setDbusProperty(
        asyncResp, "Gateway", "xyz.openbmc_project.Network",
        sdbusplus::object_path("/xyz/openbmc_project/network") / ifaceId,
        "xyz.openbmc_project.Network.EthernetInterface", "DefaultGateway",
        gateway);
}

/**
 * @brief Deletes given static IP address for the interface
 *
 * @param[in] ifaceId     Id of interface whose IP should be deleted
 * @param[in] ipHash      DBus Hash id of IP that should be deleted
 * @param[io] asyncResp   Response object that will be returned to client
 *
 * @return None
 */
/* @brief 删除该接口指定的静态 IP 地址。
 * @param[in] ifaceId 要删除 IP 的接口 Id。
 * @param[in] ipHash 要删除 IP 的 DBus Hash id。
 * @param[io] asyncResp 将返回给客户端的响应对象。
 * @return 无。 */
inline void deleteIPAddress(const std::string& ifaceId,
                            const std::string& ipHash,
                            const std::shared_ptr<bmcweb::AsyncResp>& asyncResp)
{
    dbus::utility::async_method_call(
        asyncResp,
        [asyncResp](const boost::system::error_code& ec) {
            if (ec)
            {
                messages::internalError(asyncResp->res);
            }
        },
        "xyz.openbmc_project.Network",
        "/xyz/openbmc_project/network/" + ifaceId + ipHash,
        "xyz.openbmc_project.Object.Delete", "Delete");
}

/**
 * @brief Creates a static IPv4 entry
 *
 * @param[in] ifaceId      Id of interface upon which to create the IPv4 entry
 * @param[in] prefixLength IPv4 prefix syntax for the subnet mask
 * @param[in] gateway      IPv4 address of this interfaces gateway
 * @param[in] address      IPv4 address to assign to this interface
 * @param[io] asyncResp    Response object that will be returned to client
 *
 * @return None
 */
/* @brief 创建一个静态 IPv4 条目。
 * @param[in] ifaceId 要在其上创建 IPv4 条目的接口 Id。
 * @param[in] prefixLength 子网掩码的 IPv4 前缀表示法。
 * @param[in] gateway 该接口网关的 IPv4 地址。
 * @param[in] address 分配给该接口的 IPv4 地址。
 * @param[io] asyncResp 将返回给客户端的响应对象。
 * @return 无。 */
inline void createIPv4(const std::string& ifaceId, uint8_t prefixLength,
                       const std::string& gateway, const std::string& address,
                       const std::shared_ptr<bmcweb::AsyncResp>& asyncResp)
{
    auto createIpHandler =
        [asyncResp, ifaceId, gateway](const boost::system::error_code& ec) {
            if (ec)
            {
                messages::internalError(asyncResp->res);
                return;
            }
        };

    dbus::utility::async_method_call(
        asyncResp, std::move(createIpHandler), "xyz.openbmc_project.Network",
        "/xyz/openbmc_project/network/" + ifaceId,
        "xyz.openbmc_project.Network.IP.Create", "IP",
        "xyz.openbmc_project.Network.IP.Protocol.IPv4", address, prefixLength,
        gateway);
}

/**
 * @brief Deletes the IP entry for this interface and creates a replacement
 * static entry
 *
 * @param[in] ifaceId        Id of interface upon which to create the IPv6 entry
 * @param[in] id             The unique hash entry identifying the DBus entry
 * @param[in] prefixLength   Prefix syntax for the subnet mask
 * @param[in] address        Address to assign to this interface
 * @param[in] numStaticAddrs Count of IPv4 static addresses
 * @param[io] asyncResp      Response object that will be returned to client
 *
 * @return None
 */
/* @brief 删除该接口的 IP 条目并创建替代的静态条目。
 * @param[in] ifaceId 要在其上创建 IPv6 条目的接口 Id。
 * @param[in] id 标识 DBus 条目的唯一 hash 条目。
 * @param[in] prefixLength 子网掩码的前缀表示法。
 * @param[in] address 分配给该接口的地址。
 * @param[in] numStaticAddrs IPv4 静态地址的数量。
 * @param[io] asyncResp 将返回给客户端的响应对象。
 * @return 无。 */

inline void deleteAndCreateIPAddress(
    IpVersion version, const std::string& ifaceId, const std::string& id,
    uint8_t prefixLength, const std::string& address,
    const std::string& gateway,
    const std::shared_ptr<bmcweb::AsyncResp>& asyncResp)
{
    dbus::utility::async_method_call(
        asyncResp,
        // ast-grep-ignore: long-lambda
        // ast-grep-ignore: long-lambda（忽略指令：ast-grep 跳过过长 lambda 检查）
        [asyncResp, version, ifaceId, address, prefixLength,
         gateway](const boost::system::error_code& ec) {
            if (ec)
            {
                messages::internalError(asyncResp->res);
            }
            std::string protocol = "xyz.openbmc_project.Network.IP.Protocol.";
            protocol += version == IpVersion::IpV4 ? "IPv4" : "IPv6";
            dbus::utility::async_method_call(
                asyncResp,
                [asyncResp](const boost::system::error_code& ec2) {
                    if (ec2)
                    {
                        messages::internalError(asyncResp->res);
                    }
                },
                "xyz.openbmc_project.Network",
                "/xyz/openbmc_project/network/" + ifaceId,
                "xyz.openbmc_project.Network.IP.Create", "IP", protocol,
                address, prefixLength, gateway);
        },
        "xyz.openbmc_project.Network",
        "/xyz/openbmc_project/network/" + ifaceId + id,
        "xyz.openbmc_project.Object.Delete", "Delete");
}

inline bool extractIPv6DefaultGatewayData(
    const std::string& ethifaceId,
    const dbus::utility::ManagedObjectType& dbusData,
    std::vector<StaticGatewayData>& staticGatewayConfig)
{
    std::string staticGatewayPathStart("/xyz/openbmc_project/network/");
    staticGatewayPathStart += ethifaceId;

    for (const auto& objpath : dbusData)
    {
        if (!std::string_view(objpath.first.str)
                 .starts_with(staticGatewayPathStart))
        {
            continue;
        }
        for (const auto& interface : objpath.second)
        {
            if (interface.first != "xyz.openbmc_project.Network.StaticGateway")
            {
                continue;
            }
            StaticGatewayData& staticGateway =
                staticGatewayConfig.emplace_back();
            staticGateway.id = objpath.first.filename();

            bool success = sdbusplus::unpackPropertiesNoThrow(
                redfish::dbus_utils::UnpackErrorPrinter(), interface.second,
                "Gateway", staticGateway.gateway, "ProtocolType",
                staticGateway.protocol);
            if (!success)
            {
                return false;
            }
        }
    }
    return true;
}

/**
 * @brief Creates IPv6 with given data
 *
 * @param[in] ifaceId      Id of interface whose IP should be added
 * @param[in] prefixLength Prefix length that needs to be added
 * @param[in] address      IP address that needs to be added
 * @param[io] asyncResp    Response object that will be returned to client
 *
 * @return None
 */
/* @brief 用给定数据创建 IPv6。
 * @param[in] ifaceId 要添加 IP 的接口 Id。
 * @param[in] prefixLength 要添加的前缀长度。
 * @param[in] address 要添加的 IP 地址。
 * @param[io] asyncResp 将返回给客户端的响应对象。
 * @return 无。 */
inline void createIPv6(const std::string& ifaceId, uint8_t prefixLength,
                       const std::string& address,
                       const std::shared_ptr<bmcweb::AsyncResp>& asyncResp)
{
    sdbusplus::object_path path("/xyz/openbmc_project/network");
    path /= ifaceId;

    auto createIpHandler =
        // ast-grep-ignore: long-lambda
        // ast-grep-ignore: long-lambda（忽略指令：ast-grep 跳过过长 lambda 检查）
        [asyncResp, address](const boost::system::error_code& ec) {
            if (ec)
            {
                if (ec == boost::system::errc::io_error)
                {
                    messages::propertyValueFormatError(asyncResp->res, address,
                                                       "Address");
                }
                else
                {
                    messages::internalError(asyncResp->res);
                }
            }
        };
    // Passing null for gateway, as per redfish spec IPv6StaticAddresses
    // object does not have associated gateway property
    // 按照 redfish 规范，IPv6StaticAddresses 对象没有关联的网关属性，
    // 因此这里为 gateway 传入 null。
    dbus::utility::async_method_call(
        asyncResp, std::move(createIpHandler), "xyz.openbmc_project.Network",
        path, "xyz.openbmc_project.Network.IP.Create", "IP",
        "xyz.openbmc_project.Network.IP.Protocol.IPv6", address, prefixLength,
        "");
}

/**
 * @brief Deletes given IPv6 Static Gateway
 *
 * @param[in] ifaceId     Id of interface whose IP should be deleted
 * @param[in] ipHash      DBus Hash id of IP that should be deleted
 * @param[io] asyncResp   Response object that will be returned to client
 *
 * @return None
 */
/* @brief 删除给定的 IPv6 静态网关。
 * @param[in] ifaceId 要删除 IP 的接口 Id。
 * @param[in] ipHash 要删除 IP 的 DBus Hash id。
 * @param[io] asyncResp 将返回给客户端的响应对象。
 * @return 无。 */
inline void deleteIPv6Gateway(
    std::string_view ifaceId, std::string_view gatewayId,
    const std::shared_ptr<bmcweb::AsyncResp>& asyncResp)
{
    sdbusplus::object_path path("/xyz/openbmc_project/network");
    path /= ifaceId;
    path /= gatewayId;
    dbus::utility::async_method_call(
        asyncResp,
        [asyncResp](const boost::system::error_code& ec) {
            if (ec)
            {
                messages::internalError(asyncResp->res);
            }
        },
        "xyz.openbmc_project.Network", path,
        "xyz.openbmc_project.Object.Delete", "Delete");
}

/**
 * @brief Creates IPv6 static default gateway with given data
 *
 * @param[in] ifaceId      Id of interface whose IP should be added
 * @param[in] gateway      Gateway address that needs to be added
 * @param[io] asyncResp    Response object that will be returned to client
 *
 * @return None
 */
/* @brief 用给定数据创建 IPv6 静态默认网关。
 * @param[in] ifaceId 要添加 IP 的接口 Id。
 * @param[in] gateway 需要添加的网关地址。
 * @param[io] asyncResp 将返回给客户端的响应对象。
 * @return 无。 */
inline void createIPv6DefaultGateway(
    std::string_view ifaceId, const std::string& gateway,
    const std::shared_ptr<bmcweb::AsyncResp>& asyncResp)
{
    sdbusplus::object_path path("/xyz/openbmc_project/network");
    path /= ifaceId;
    auto createIpHandler = [asyncResp](const boost::system::error_code& ec) {
        if (ec)
        {
            messages::internalError(asyncResp->res);
        }
    };
    dbus::utility::async_method_call(
        asyncResp, std::move(createIpHandler), "xyz.openbmc_project.Network",
        path, "xyz.openbmc_project.Network.StaticGateway.Create",
        "StaticGateway", gateway,
        "xyz.openbmc_project.Network.IP.Protocol.IPv6");
}

/**
 * @brief Deletes the IPv6 default gateway entry for this interface and
 * creates a replacement IPv6 default gateway entry
 *
 * @param[in] ifaceId      Id of interface upon which to create the IPv6
 * entry
 * @param[in] gateway      IPv6 gateway to assign to this interface
 * @param[io] asyncResp    Response object that will be returned to client
 *
 * @return None
 */
/* @brief 删除该接口的 IPv6 默认网关条目，并创建替代的 IPv6 默认网关条目。
 * @param[in] ifaceId 要在其上创建 IPv6 条目的接口 Id。
 * @param[in] gateway 分配给该接口的 IPv6 网关。
 * @param[io] asyncResp 将返回给客户端的响应对象。
 * @return 无。 */
inline void deleteAndCreateIPv6DefaultGateway(
    std::string_view ifaceId, std::string_view gatewayId,
    const std::string& gateway,
    const std::shared_ptr<bmcweb::AsyncResp>& asyncResp)
{
    sdbusplus::object_path path("/xyz/openbmc_project/network");
    path /= ifaceId;
    path /= gatewayId;
    dbus::utility::async_method_call(
        asyncResp,
        [asyncResp, ifaceId, gateway](const boost::system::error_code& ec) {
            if (ec)
            {
                messages::internalError(asyncResp->res);
                return;
            }
            createIPv6DefaultGateway(ifaceId, gateway, asyncResp);
        },
        "xyz.openbmc_project.Network", path,
        "xyz.openbmc_project.Object.Delete", "Delete");
}

/**
 * @brief Sets IPv6 default gateway with given data
 *
 * @param[in] ifaceId      Id of interface whose gateway should be added
 * @param[in] input        Contains address that needs to be added
 * @param[in] staticGatewayData  Current static gateways in the system
 * @param[io] asyncResp    Response object that will be returned to client
 *
 * @return None
 */
/* @brief 用给定数据设置 IPv6 默认网关。
 * @param[in] ifaceId 要添加网关的接口 Id。
 * @param[in] input 包含需要添加的地址。
 * @param[in] staticGatewayData 系统中当前的静态网关。
 * @param[io] asyncResp 将返回给客户端的响应对象。
 * @return 无。 */

inline void handleIPv6DefaultGateway(
    const std::string& ifaceId,
    std::vector<std::variant<nlohmann::json::object_t, std::nullptr_t>>& input,
    const std::vector<StaticGatewayData>& staticGatewayData,
    const std::shared_ptr<bmcweb::AsyncResp>& asyncResp)
{
    size_t entryIdx = 1;
    std::vector<StaticGatewayData>::const_iterator staticGatewayEntry =
        staticGatewayData.begin();

    for (std::variant<nlohmann::json::object_t, std::nullptr_t>& thisJson :
         input)
    {
        // find the next gateway entry
        // 查找下一个网关条目。
        while (staticGatewayEntry != staticGatewayData.end())
        {
            if (staticGatewayEntry->protocol ==
                "xyz.openbmc_project.Network.IP.Protocol.IPv6")
            {
                break;
            }
            staticGatewayEntry++;
        }
        std::string pathString =
            "IPv6StaticDefaultGateways/" + std::to_string(entryIdx);
        nlohmann::json::object_t* obj =
            std::get_if<nlohmann::json::object_t>(&thisJson);
        if (obj == nullptr)
        {
            if (staticGatewayEntry == staticGatewayData.end())
            {
                messages::resourceCannotBeDeleted(asyncResp->res);
                return;
            }
            deleteIPv6Gateway(ifaceId, staticGatewayEntry->id, asyncResp);
            staticGatewayEntry++;
            entryIdx++;
            continue;
        }
        if (obj->empty())
        {
            // Do nothing, but make sure the entry exists.
            // 什么都不做，但要确保该条目存在。
            if (staticGatewayEntry == staticGatewayData.end())
            {
                messages::propertyValueFormatError(asyncResp->res, *obj,
                                                   pathString);
                return;
            }
        }
        std::optional<std::string> address;

        if (!json_util::readJsonObject(*obj, asyncResp->res, "Address",
                                       address))
        {
            return;
        }
        const std::string* addr = nullptr;
        if (address)
        {
            addr = &(*address);
        }
        else if (staticGatewayEntry != staticGatewayData.end())
        {
            addr = &(staticGatewayEntry->gateway);
        }
        else
        {
            messages::propertyMissing(asyncResp->res, pathString + "/Address");
            return;
        }
        if (staticGatewayEntry != staticGatewayData.end())
        {
            deleteAndCreateIPv6DefaultGateway(ifaceId, staticGatewayEntry->id,
                                              *addr, asyncResp);
            staticGatewayEntry++;
        }
        else
        {
            createIPv6DefaultGateway(ifaceId, *addr, asyncResp);
        }
        entryIdx++;
    }
}

/**
 * Function that retrieves all properties for given Ethernet Interface
 * Object
 * from EntityManager Network Manager
 * @param ethiface_id a eth interface id to query on DBus
 * @param callback a function that shall be called to convert Dbus output
 * into JSON
 */
/* 从 EntityManager Network Manager 检索给定 Ethernet Interface 对象的所有属性。
 * @param ethiface_id 要在 DBus 上查询的以太网接口 id。
 * @param callback 一个函数，用于将 Dbus 输出转换为 JSON。 */
template <typename CallbackFunc>
void getEthernetIfaceData(const std::string& ethifaceId,
                          CallbackFunc&& callback)
{
    sdbusplus::object_path path("/xyz/openbmc_project/network");
    dbus::utility::getManagedObjects(
        "xyz.openbmc_project.Network", path,
        [ethifaceId{std::string{ethifaceId}},
         callback = std::forward<CallbackFunc>(callback)](
            const boost::system::error_code& ec,
            const dbus::utility::ManagedObjectType& resp) mutable {
            EthernetInterfaceData ethData{};
            std::vector<IPv4AddressData> ipv4Data;
            std::vector<IPv6AddressData> ipv6Data;
            std::vector<StaticGatewayData> ipv6GatewayData;

            if (ec)
            {
                callback(false, ethData, ipv4Data, ipv6Data, ipv6GatewayData);
                return;
            }

            bool found =
                extractEthernetInterfaceData(ethifaceId, resp, ethData);
            if (!found)
            {
                callback(false, ethData, ipv4Data, ipv6Data, ipv6GatewayData);
                return;
            }

            extractIPData(ethifaceId, resp, ipv4Data);
            // Fix global GW
            // 修正全局 GW（网关）。
            for (IPv4AddressData& ipv4 : ipv4Data)
            {
                if (((ipv4.linktype == LinkType::Global) &&
                     (ipv4.gateway == "0.0.0.0")) ||
                    (ipv4.origin == "DHCP") || (ipv4.origin == "Static"))
                {
                    ipv4.gateway = ethData.defaultGateway;
                }
            }

            extractIPV6Data(ethifaceId, resp, ipv6Data);
            if (!extractIPv6DefaultGatewayData(ethifaceId, resp,
                                               ipv6GatewayData))
            {
                callback(false, ethData, ipv4Data, ipv6Data, ipv6GatewayData);
            }
            // Finally make a callback with useful data
            // 最后用有用的数据执行回调。
            callback(true, ethData, ipv4Data, ipv6Data, ipv6GatewayData);
        });
}

/**
 * Function that retrieves all Ethernet Interfaces available through Network
 * Manager
 * @param callback a function that shall be called to convert Dbus output
 * into JSON.
 */
/* 检索 Network Manager 中所有可用的 Ethernet Interface。
 * @param callback 一个函数，用于将 Dbus 输出转换为 JSON。 */
template <typename CallbackFunc>
void getEthernetIfaceList(CallbackFunc&& callback)
{
    sdbusplus::object_path path("/xyz/openbmc_project/network");
    dbus::utility::getManagedObjects(
        "xyz.openbmc_project.Network", path,
        // ast-grep-ignore: long-lambda
        // ast-grep-ignore: long-lambda（忽略指令：ast-grep 跳过过长 lambda 检查）
        [callback = std::forward<CallbackFunc>(callback)](
            const boost::system::error_code& ec,
            const dbus::utility::ManagedObjectType& resp) {
            // Callback requires vector<string> to retrieve all available
            // ethernet interfaces
            // 回调需要 vector<string> 来检索所有可用的以太网接口。
            std::vector<std::string> ifaceList;
            ifaceList.reserve(resp.size());
            if (ec)
            {
                callback(false, ifaceList);
                return;
            }

            // Iterate over all retrieved ObjectPaths.
            // 遍历所有检索到的 ObjectPath。
            for (const auto& objpath : resp)
            {
                // And all interfaces available for certain ObjectPath.
                // 以及特定 ObjectPath 上所有可用的接口。
                for (const auto& interface : objpath.second)
                {
                    // If interface is
                    // xyz.openbmc_project.Network.EthernetInterface, this is
                    // what we're looking for.
                    // 如果接口是 xyz.openbmc_project.Network.EthernetInterface，那就是我们要找的。
                    if (interface.first ==
                        "xyz.openbmc_project.Network.EthernetInterface")
                    {
                        std::string ifaceId = objpath.first.filename();
                        if (ifaceId.empty())
                        {
                            continue;
                        }
                        // and put it into output vector.
                        // 并将其放入输出 vector。
                        ifaceList.emplace_back(ifaceId);
                    }
                }
            }

            std::ranges::sort(ifaceList, AlphanumLess<std::string>());

            // Finally make a callback with useful data
            // 最后用有用的数据执行回调。
            callback(true, ifaceList);
        });
}

inline bool isHostnameValid(const std::string& hostname)
{
    // A valid host name can never have the dotted-decimal form (RFC 1123)
    // 有效的主机名永远不能是点分十进制形式（RFC 1123）。
    if (std::ranges::all_of(hostname, ::isdigit))
    {
        return false;
    }
    // Each label(hostname/subdomains) within a valid FQDN
    // MUST handle host names of up to 63 characters (RFC 1123)
    // labels cannot start or end with hyphens (RFC 952)
    // labels can start with numbers (RFC 1123)
    // 有效 FQDN 中的每个标签（主机名/子域）
    // 必须能处理最长 63 个字符的主机名（RFC 1123）。
    // 标签不能以连字符开头或结尾（RFC 952）。
    // 标签可以以数字开头（RFC 1123）。
    const static std::regex pattern(
        "^[a-zA-Z0-9]|[a-zA-Z0-9][a-zA-Z0-9\\-]{0,61}[a-zA-Z0-9]$");

    return std::regex_match(hostname, pattern);
}

inline bool isDomainnameValid(const std::string& domainname)
{
    // Can have multiple subdomains
    // Top Level Domain's min length is 2 character
    // 可以有多个子域。
    // 顶级域名的最小长度为 2 个字符。
    const static std::regex pattern(
        "^([A-Za-z0-9][a-zA-Z0-9\\-]{1,61}|[a-zA-Z0-9]{1,30}\\.)*[a-zA-Z]{2,}$");

    return std::regex_match(domainname, pattern);
}

inline void handleHostnamePatch(
    const std::string& hostname,
    const std::shared_ptr<bmcweb::AsyncResp>& asyncResp)
{
    // SHOULD handle host names of up to 255 characters(RFC 1123)
    // 应能处理最长 255 个字符的主机名（RFC 1123）。
    if (hostname.length() > 255)
    {
        messages::propertyValueFormatError(asyncResp->res, hostname,
                                           "HostName");
        return;
    }
    setDbusProperty(
        asyncResp, "HostName", "xyz.openbmc_project.Network",
        sdbusplus::object_path("/xyz/openbmc_project/network/config"),
        "xyz.openbmc_project.Network.SystemConfiguration", "HostName",
        hostname);
}

inline void handleMTUSizePatch(
    const std::string& ifaceId, const size_t mtuSize,
    const std::shared_ptr<bmcweb::AsyncResp>& asyncResp)
{
    sdbusplus::object_path objPath("/xyz/openbmc_project/network");
    objPath /= ifaceId;
    setDbusProperty(asyncResp, "MTUSize", "xyz.openbmc_project.Network",
                    objPath, "xyz.openbmc_project.Network.EthernetInterface",
                    "MTU", mtuSize);
}

inline void handleDomainnamePatch(
    const std::string& ifaceId, const std::string& domainname,
    const std::shared_ptr<bmcweb::AsyncResp>& asyncResp)
{
    std::vector<std::string> vectorDomainname = {domainname};
    setDbusProperty(
        asyncResp, "FQDN", "xyz.openbmc_project.Network",
        sdbusplus::object_path("/xyz/openbmc_project/network") / ifaceId,
        "xyz.openbmc_project.Network.EthernetInterface", "DomainName",
        vectorDomainname);
}

inline void handleFqdnPatch(const std::string& ifaceId, const std::string& fqdn,
                            const std::shared_ptr<bmcweb::AsyncResp>& asyncResp)
{
    // Total length of FQDN must not exceed 255 characters(RFC 1035)
    // FQDN 的总长度不得超过 255 个字符（RFC 1035）。
    if (fqdn.length() > 255)
    {
        messages::propertyValueFormatError(asyncResp->res, fqdn, "FQDN");
        return;
    }

    size_t pos = fqdn.find('.');
    if (pos == std::string::npos)
    {
        messages::propertyValueFormatError(asyncResp->res, fqdn, "FQDN");
        return;
    }

    std::string hostname;
    std::string domainname;
    domainname = (fqdn).substr(pos + 1);
    hostname = (fqdn).substr(0, pos);

    if (!isHostnameValid(hostname) || !isDomainnameValid(domainname))
    {
        messages::propertyValueFormatError(asyncResp->res, fqdn, "FQDN");
        return;
    }

    handleHostnamePatch(hostname, asyncResp);
    handleDomainnamePatch(ifaceId, domainname, asyncResp);
}

inline void handleMACAddressPatch(
    const std::string& ifaceId, const std::string& macAddress,
    const std::shared_ptr<bmcweb::AsyncResp>& asyncResp)
{
    setDbusProperty(
        asyncResp, "MACAddress", "xyz.openbmc_project.Network",
        sdbusplus::object_path("/xyz/openbmc_project/network") / ifaceId,
        "xyz.openbmc_project.Network.MACAddress", "MACAddress", macAddress);
}

inline void setDHCPEnabled(const std::string& ifaceId,
                           const std::string& propertyName, const bool v4Value,
                           const bool v6Value,
                           const std::shared_ptr<bmcweb::AsyncResp>& asyncResp)
{
    const std::string dhcp = getDhcpEnabledEnumeration(v4Value, v6Value);
    setDbusProperty(
        asyncResp, "DHCPv4", "xyz.openbmc_project.Network",
        sdbusplus::object_path("/xyz/openbmc_project/network") / ifaceId,
        "xyz.openbmc_project.Network.EthernetInterface", propertyName, dhcp);
}

enum class NetworkType
{
    dhcp4,
    dhcp6
};

inline void setDHCPConfig(const std::string& propertyName, const bool& value,
                          const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
                          const std::string& ethifaceId, NetworkType type)
{
    BMCWEB_LOG_DEBUG("{} = {}", propertyName, value);
    std::string redfishPropertyName;
    sdbusplus::object_path path("/xyz/openbmc_project/network/");
    path /= ethifaceId;

    if (type == NetworkType::dhcp4)
    {
        path /= "dhcp4";
        redfishPropertyName = "DHCPv4";
    }
    else
    {
        path /= "dhcp6";
        redfishPropertyName = "DHCPv6";
    }

    setDbusProperty(
        asyncResp, redfishPropertyName, "xyz.openbmc_project.Network", path,
        "xyz.openbmc_project.Network.DHCPConfiguration", propertyName, value);
}

inline void handleSLAACAutoConfigPatch(
    const std::string& ifaceId, bool ipv6AutoConfigEnabled,
    const std::shared_ptr<bmcweb::AsyncResp>& asyncResp)
{
    sdbusplus::object_path path("/xyz/openbmc_project/network");
    path /= ifaceId;
    setDbusProperty(asyncResp,
                    "StatelessAddressAutoConfig/IPv6AutoConfigEnabled",
                    "xyz.openbmc_project.Network", path,
                    "xyz.openbmc_project.Network.EthernetInterface",
                    "IPv6AcceptRA", ipv6AutoConfigEnabled);
}

inline void handleDHCPPatch(
    const std::string& ifaceId, const EthernetInterfaceData& ethData,
    const DHCPParameters& v4dhcpParms, const DHCPParameters& v6dhcpParms,
    const std::shared_ptr<bmcweb::AsyncResp>& asyncResp)
{
    bool ipv4Active = translateDhcpEnabledToBool(ethData.dhcpEnabled, true);
    bool ipv6Active = translateDhcpEnabledToBool(ethData.dhcpEnabled, false);

    if (ipv4Active)
    {
        updateIPv4DefaultGateway(ifaceId, "", asyncResp);
    }
    bool nextv4DHCPState =
        v4dhcpParms.dhcpv4Enabled ? *v4dhcpParms.dhcpv4Enabled : ipv4Active;

    bool nextv6DHCPState{};
    if (v6dhcpParms.dhcpv6OperatingMode)
    {
        if ((*v6dhcpParms.dhcpv6OperatingMode != "Enabled") &&
            (*v6dhcpParms.dhcpv6OperatingMode != "Disabled"))
        {
            messages::propertyValueFormatError(asyncResp->res,
                                               *v6dhcpParms.dhcpv6OperatingMode,
                                               "OperatingMode");
            return;
        }
        nextv6DHCPState = (*v6dhcpParms.dhcpv6OperatingMode == "Enabled");
    }
    else
    {
        nextv6DHCPState = ipv6Active;
    }

    bool nextDNSv4 = ethData.dnsv4Enabled;
    bool nextDNSv6 = ethData.dnsv6Enabled;
    if (v4dhcpParms.useDnsServers)
    {
        nextDNSv4 = *v4dhcpParms.useDnsServers;
    }
    if (v6dhcpParms.useDnsServers)
    {
        nextDNSv6 = *v6dhcpParms.useDnsServers;
    }

    bool nextNTPv4 = ethData.ntpv4Enabled;
    bool nextNTPv6 = ethData.ntpv6Enabled;
    if (v4dhcpParms.useNtpServers)
    {
        nextNTPv4 = *v4dhcpParms.useNtpServers;
    }
    if (v6dhcpParms.useNtpServers)
    {
        nextNTPv6 = *v6dhcpParms.useNtpServers;
    }

    bool nextUsev4Domain = ethData.domainv4Enabled;
    bool nextUsev6Domain = ethData.domainv6Enabled;
    if (v4dhcpParms.useDomainName)
    {
        nextUsev4Domain = *v4dhcpParms.useDomainName;
    }
    if (v6dhcpParms.useDomainName)
    {
        nextUsev6Domain = *v6dhcpParms.useDomainName;
    }

    BMCWEB_LOG_DEBUG("set DHCPEnabled...");
    setDHCPEnabled(ifaceId, "DHCPEnabled", nextv4DHCPState, nextv6DHCPState,
                   asyncResp);
    BMCWEB_LOG_DEBUG("set DNSEnabled...");
    setDHCPConfig("DNSEnabled", nextDNSv4, asyncResp, ifaceId,
                  NetworkType::dhcp4);
    BMCWEB_LOG_DEBUG("set NTPEnabled...");
    setDHCPConfig("NTPEnabled", nextNTPv4, asyncResp, ifaceId,
                  NetworkType::dhcp4);
    BMCWEB_LOG_DEBUG("set DomainEnabled...");
    setDHCPConfig("DomainEnabled", nextUsev4Domain, asyncResp, ifaceId,
                  NetworkType::dhcp4);
    BMCWEB_LOG_DEBUG("set DNSEnabled for dhcp6...");
    setDHCPConfig("DNSEnabled", nextDNSv6, asyncResp, ifaceId,
                  NetworkType::dhcp6);
    BMCWEB_LOG_DEBUG("set NTPEnabled for dhcp6...");
    setDHCPConfig("NTPEnabled", nextNTPv6, asyncResp, ifaceId,
                  NetworkType::dhcp6);
    BMCWEB_LOG_DEBUG("set DomainEnabled for dhcp6...");
    setDHCPConfig("DomainEnabled", nextUsev6Domain, asyncResp, ifaceId,
                  NetworkType::dhcp6);
}

inline std::vector<IPv4AddressData>::const_iterator getNextStaticIpEntry(
    const std::vector<IPv4AddressData>::const_iterator& head,
    const std::vector<IPv4AddressData>::const_iterator& end)
{
    return std::find_if(head, end, [](const IPv4AddressData& value) {
        return value.origin == "Static";
    });
}

inline std::vector<IPv6AddressData>::const_iterator getNextStaticIpEntry(
    const std::vector<IPv6AddressData>::const_iterator& head,
    const std::vector<IPv6AddressData>::const_iterator& end)
{
    return std::find_if(head, end, [](const IPv6AddressData& value) {
        return value.origin == "Static";
    });
}

enum class AddrChange
{
    Noop,
    Delete,
    Update,
};

// Struct representing a dbus change
// 表示 dbus 变更的结构体。
struct AddressPatch
{
    std::string address;
    std::string gateway;
    uint8_t prefixLength = 0;
    std::string existingDbusId;
    AddrChange operation = AddrChange::Noop;
};

inline bool parseAddresses(
    std::vector<std::variant<nlohmann::json::object_t, std::nullptr_t>>& input,
    const std::vector<IPv4AddressData>& ipv4Data, crow::Response& res,
    std::vector<AddressPatch>& addressesOut, std::string& gatewayOut)
{
    std::vector<IPv4AddressData>::const_iterator nicIpEntry =
        getNextStaticIpEntry(ipv4Data.cbegin(), ipv4Data.cend());

    std::string lastGatewayPath;
    size_t entryIdx = 0;
    for (std::variant<nlohmann::json::object_t, std::nullptr_t>& thisJson :
         input)
    {
        std::string pathString =
            std::format("IPv4StaticAddresses/{}", entryIdx);
        AddressPatch& thisAddress = addressesOut.emplace_back();
        nlohmann::json::object_t* obj =
            std::get_if<nlohmann::json::object_t>(&thisJson);
        if (nicIpEntry != ipv4Data.cend())
        {
            thisAddress.existingDbusId = nicIpEntry->id;
        }

        if (obj == nullptr)
        {
            if (thisAddress.existingDbusId.empty())
            {
                // Received a DELETE action on an entry not assigned to the NIC
                // 收到了针对未分配给该 NIC 的条目的 DELETE 操作。
                messages::resourceCannotBeDeleted(res);
                return false;
            }
            thisAddress.operation = AddrChange::Delete;
        }
        else
        {
            std::optional<std::string> address;
            std::optional<std::string> gateway;
            std::optional<std::string> subnetMask;
            if (!obj->empty())
            {
                if (!json_util::readJsonObject(  //
                        *obj, res,               //
                        "Address", address,      //
                        "Gateway", gateway,      //
                        "SubnetMask", subnetMask //
                        ))
                {
                    return false;
                }
            }
            // Find the address/subnet/gateway values. Any values that are
            // not explicitly provided are assumed to be unmodified from the
            // current state of the interface. Merge existing state into the
            // current request.
            // 查找地址/子网/网关的值。任何未显式提供的值，
            // 均假定与接口的当前状态一致，未做修改。
            // 将现有状态合并到当前请求中。
            if (address)
            {
                if (!ip_util::ipv4VerifyIpAndGetBitcount(*address))
                {
                    messages::propertyValueFormatError(res, *address,
                                                       pathString + "/Address");
                    return false;
                }
                thisAddress.operation = AddrChange::Update;
                thisAddress.address = *address;
            }
            else if (thisAddress.existingDbusId.empty())
            {
                messages::propertyMissing(res, pathString + "/Address");
                return false;
            }
            else
            {
                thisAddress.address = nicIpEntry->address;
            }

            if (subnetMask)
            {
                uint8_t prefixLength = 0;
                if (!ip_util::ipv4VerifyIpAndGetBitcount(*subnetMask,
                                                         &prefixLength))
                {
                    messages::propertyValueFormatError(
                        res, *subnetMask, pathString + "/SubnetMask");
                    return false;
                }
                thisAddress.prefixLength = prefixLength;
                thisAddress.operation = AddrChange::Update;
            }
            else if (thisAddress.existingDbusId.empty())
            {
                messages::propertyMissing(res, pathString + "/SubnetMask");
                return false;
            }
            else
            {
                uint8_t prefixLength = 0;
                // Ignore return code.  It came from internal, it's it's invalid
                // nothing we can do
                // 忽略返回码。它来自内部，如果无效我们也无能为力。
                ip_util::ipv4VerifyIpAndGetBitcount(nicIpEntry->netmask,
                                                    &prefixLength);

                thisAddress.prefixLength = prefixLength;
            }
            if (gateway)
            {
                if (!ip_util::ipv4VerifyIpAndGetBitcount(*gateway))
                {
                    messages::propertyValueFormatError(res, *gateway,
                                                       pathString + "/Gateway");
                    return false;
                }
                thisAddress.operation = AddrChange::Update;
                thisAddress.gateway = *gateway;
            }
            else if (thisAddress.existingDbusId.empty())
            {
                // Default to null gateway
                // 默认网关为 null。
                gateway = "";
            }
            else
            {
                thisAddress.gateway = nicIpEntry->gateway;
            }

            // Changing gateway from existing
            // 正在更改网关（与现有值不同）。
            if (!thisAddress.gateway.empty() &&
                thisAddress.gateway != "0.0.0.0")
            {
                if (!gatewayOut.empty() && gatewayOut != thisAddress.gateway)
                {
                    // A NIC can only have a single active gateway value.
                    // If any gateway in the array of static addresses
                    // mismatch the PATCH is in error.
                    // 一个 NIC 只能有一个生效的网关值。
                    // 如果静态地址数组中有任何网关与之不匹配，则 PATCH 出错。
                    std::string arg1 = pathString + "/Gateway";
                    std::string arg2 = lastGatewayPath + "/Gateway";
                    messages::propertyValueConflict(res, arg1, arg2);
                    return false;
                }
                gatewayOut = thisAddress.gateway;
                lastGatewayPath = pathString;
            }
        }
        if (nicIpEntry != ipv4Data.end())
        {
            nicIpEntry++;
            nicIpEntry = getNextStaticIpEntry(nicIpEntry, ipv4Data.cend());
        }
        entryIdx++;
    }

    // Delete the remaining IPs
    // 删除剩余的 IP。
    while (nicIpEntry != ipv4Data.cend())
    {
        AddressPatch& thisAddress = addressesOut.emplace_back();
        thisAddress.operation = AddrChange::Delete;
        thisAddress.existingDbusId = nicIpEntry->id;
        nicIpEntry++;
        nicIpEntry = getNextStaticIpEntry(nicIpEntry, ipv4Data.cend());
    }

    return true;
}

inline void handleIPv4StaticPatch(
    const std::string& ifaceId,
    std::vector<std::variant<nlohmann::json::object_t, std::nullptr_t>>& input,
    const EthernetInterfaceData& ethData,
    const std::vector<IPv4AddressData>& ipv4Data,
    const std::shared_ptr<bmcweb::AsyncResp>& asyncResp)
{
    std::vector<AddressPatch> addresses;
    std::string gatewayOut;
    if (!parseAddresses(input, ipv4Data, asyncResp->res, addresses, gatewayOut))
    {
        return;
    }

    // If we're setting the gateway to something new, delete the
    // existing so we won't conflict
    // 如果要把网关设置为新值，先删除现有网关以免冲突。
    if (!ethData.defaultGateway.empty() && ethData.defaultGateway != gatewayOut)
    {
        updateIPv4DefaultGateway(ifaceId, "", asyncResp);
    }

    for (const AddressPatch& address : addresses)
    {
        switch (address.operation)
        {
            case AddrChange::Delete:
            {
                BMCWEB_LOG_ERROR("Deleting id {} on interface {}",
                                 address.existingDbusId, ifaceId);
                deleteIPAddress(ifaceId, address.existingDbusId, asyncResp);
            }
            break;
            case AddrChange::Update:
            {
                // Update is a delete then a recreate
                // Only need to update if there is an existing ip at this index
                // Update 是先删除再重建。
                // 只有该索引处已有 IP 时才需要更新。
                if (!address.existingDbusId.empty())
                {
                    BMCWEB_LOG_ERROR("Deleting id {} on interface {}",
                                     address.existingDbusId, ifaceId);
                    deleteAndCreateIPAddress(
                        IpVersion::IpV4, ifaceId, address.existingDbusId,
                        address.prefixLength, address.address, address.gateway,
                        asyncResp);
                }
                else
                {
                    // Otherwise, just create a new one
                    // 否则，直接新建一个。
                    BMCWEB_LOG_ERROR(
                        "creating ip {} prefix {} gateway {} on interface {}",
                        address.address, address.prefixLength, address.gateway,
                        ifaceId);
                    createIPv4(ifaceId, address.prefixLength, address.gateway,
                               address.address, asyncResp);
                }
            }
            break;
            default:
            {
                // Leave alone
                // 不处理。
            }
            break;
        }
    }

    // now update to the new gateway.
    // Default gateway is already empty, so no need to update if we're clearing
    // 现在更新为新网关。
    // 默认网关已经为空，因此如果要清空则无需更新。
    if (!gatewayOut.empty() && ethData.defaultGateway != gatewayOut)
    {
        updateIPv4DefaultGateway(ifaceId, gatewayOut, asyncResp);
    }
}

inline void handleStaticNameServersPatch(
    const std::string& ifaceId,
    const std::vector<std::string>& updatedStaticNameServers,
    const std::shared_ptr<bmcweb::AsyncResp>& asyncResp)
{
    setDbusProperty(
        asyncResp, "StaticNameServers", "xyz.openbmc_project.Network",
        sdbusplus::object_path("/xyz/openbmc_project/network") / ifaceId,
        "xyz.openbmc_project.Network.EthernetInterface", "StaticNameServers",
        updatedStaticNameServers);
}

inline void handleIPv6StaticAddressesPatch(
    const std::string& ifaceId,
    std::vector<std::variant<nlohmann::json::object_t, std::nullptr_t>>& input,
    const std::vector<IPv6AddressData>& ipv6Data,
    const std::shared_ptr<bmcweb::AsyncResp>& asyncResp)
{
    size_t entryIdx = 1;
    std::vector<IPv6AddressData>::const_iterator nicIpEntry =
        getNextStaticIpEntry(ipv6Data.cbegin(), ipv6Data.cend());
    for (std::variant<nlohmann::json::object_t, std::nullptr_t>& thisJson :
         input)
    {
        std::string pathString =
            "IPv6StaticAddresses/" + std::to_string(entryIdx);
        nlohmann::json::object_t* obj =
            std::get_if<nlohmann::json::object_t>(&thisJson);
        if (obj != nullptr && !obj->empty())
        {
            std::optional<std::string> address;
            std::optional<uint8_t> prefixLength;
            nlohmann::json::object_t thisJsonCopy = *obj;
            if (!json_util::readJsonObject(       //
                    thisJsonCopy, asyncResp->res, //
                    "Address", address,           //
                    "PrefixLength", prefixLength  //
                    ))
            {
                return;
            }

            // Find the address and prefixLength values. Any values that are
            // not explicitly provided are assumed to be unmodified from the
            // current state of the interface. Merge existing state into the
            // current request.
            // 查找 address 和 prefixLength 的值。任何未显式提供的值，
            // 均假定与接口的当前状态一致，未做修改。
            // 将现有状态合并到当前请求中。
            if (!address)
            {
                if (nicIpEntry == ipv6Data.end())
                {
                    messages::propertyMissing(asyncResp->res,
                                              pathString + "/Address");
                    return;
                }
                address = nicIpEntry->address;
            }

            if (!prefixLength)
            {
                if (nicIpEntry == ipv6Data.end())
                {
                    messages::propertyMissing(asyncResp->res,
                                              pathString + "/PrefixLength");
                    return;
                }
                prefixLength = nicIpEntry->prefixLength;
            }

            if (nicIpEntry != ipv6Data.end())
            {
                deleteAndCreateIPAddress(IpVersion::IpV6, ifaceId,
                                         nicIpEntry->id, *prefixLength,
                                         *address, "", asyncResp);
                nicIpEntry =
                    getNextStaticIpEntry(++nicIpEntry, ipv6Data.cend());
            }
            else
            {
                createIPv6(ifaceId, *prefixLength, *address, asyncResp);
            }
            entryIdx++;
        }
        else
        {
            if (nicIpEntry == ipv6Data.end())
            {
                // Requesting a DELETE/DO NOT MODIFY action for an item
                // that isn't present on the eth(n) interface. Input JSON is
                // in error, so bail out.
                // 请求对 eth(n) 接口上不存在的条目执行 DELETE/DO NOT MODIFY 操作。
                // 输入的 JSON 有误，
                // 因此退出。
                if (obj == nullptr)
                {
                    messages::resourceCannotBeDeleted(asyncResp->res);
                    return;
                }
                messages::propertyValueFormatError(asyncResp->res, *obj,
                                                   pathString);
                return;
            }

            if (obj == nullptr)
            {
                deleteIPAddress(ifaceId, nicIpEntry->id, asyncResp);
            }
            if (nicIpEntry != ipv6Data.cend())
            {
                nicIpEntry =
                    getNextStaticIpEntry(++nicIpEntry, ipv6Data.cend());
            }
            entryIdx++;
        }
    }
}

inline std::string extractParentInterfaceName(const std::string& ifaceId)
{
    std::size_t pos = ifaceId.find('_');
    return ifaceId.substr(0, pos);
}

inline void parseInterfaceData(
    const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
    const std::string& ifaceId, const EthernetInterfaceData& ethData,
    const std::vector<IPv4AddressData>& ipv4Data,
    const std::vector<IPv6AddressData>& ipv6Data,
    const std::vector<StaticGatewayData>& ipv6GatewayData)
{
    nlohmann::json& jsonResponse = asyncResp->res.jsonValue;
    jsonResponse["Id"] = ifaceId;
    jsonResponse["@odata.id"] =
        boost::urls::format("/redfish/v1/Managers/{}/EthernetInterfaces/{}",
                            BMCWEB_REDFISH_MANAGER_URI_NAME, ifaceId);
    jsonResponse["InterfaceEnabled"] = ethData.nicEnabled;

    if (ethData.nicEnabled)
    {
        jsonResponse["LinkStatus"] =
            ethData.linkUp ? ethernet_interface::LinkStatus::LinkUp
                           : ethernet_interface::LinkStatus::LinkDown;
        jsonResponse["Status"]["State"] = resource::State::Enabled;
    }
    else
    {
        jsonResponse["LinkStatus"] = ethernet_interface::LinkStatus::NoLink;
        jsonResponse["Status"]["State"] = resource::State::Disabled;
    }

    jsonResponse["SpeedMbps"] = ethData.speed;
    jsonResponse["FullDuplex"] = ethData.fullDuplex;
    jsonResponse["MTUSize"] = ethData.mtuSize;
    if (ethData.macAddress)
    {
        jsonResponse["MACAddress"] = *ethData.macAddress;
    }
    jsonResponse["DHCPv4"]["DHCPEnabled"] =
        translateDhcpEnabledToBool(ethData.dhcpEnabled, true);
    jsonResponse["DHCPv4"]["UseNTPServers"] = ethData.ntpv4Enabled;
    jsonResponse["DHCPv4"]["UseDNSServers"] = ethData.dnsv4Enabled;
    jsonResponse["DHCPv4"]["UseDomainName"] = ethData.domainv4Enabled;
    jsonResponse["DHCPv6"]["OperatingMode"] =
        translateDhcpEnabledToBool(ethData.dhcpEnabled, false)
            ? "Enabled"
            : "Disabled";
    jsonResponse["DHCPv6"]["UseNTPServers"] = ethData.ntpv6Enabled;
    jsonResponse["DHCPv6"]["UseDNSServers"] = ethData.dnsv6Enabled;
    jsonResponse["DHCPv6"]["UseDomainName"] = ethData.domainv6Enabled;
    jsonResponse["StatelessAddressAutoConfig"]["IPv6AutoConfigEnabled"] =
        ethData.ipv6AcceptRa;

    if (!ethData.hostName.empty())
    {
        jsonResponse["HostName"] = ethData.hostName;

        // When domain name is empty then it means, that it is a network
        // without domain names, and the host name itself must be treated as
        // FQDN
        // 当域名为空时，表示这是一个没有域名的网络，
        // 此时主机名本身必须作为 FQDN 处理。
        std::string fqdn = ethData.hostName;
        if (!ethData.domainnames.empty())
        {
            fqdn += "." + ethData.domainnames[0];
        }
        jsonResponse["FQDN"] = fqdn;
    }

    if (ethData.vlanId)
    {
        jsonResponse["EthernetInterfaceType"] =
            ethernet_interface::EthernetDeviceType::Virtual;
        jsonResponse["VLAN"]["VLANEnable"] = true;
        jsonResponse["VLAN"]["VLANId"] = *ethData.vlanId;
        jsonResponse["VLAN"]["Tagged"] = true;

        nlohmann::json::array_t relatedInterfaces;
        nlohmann::json& parentInterface = relatedInterfaces.emplace_back();
        parentInterface["@odata.id"] =
            boost::urls::format("/redfish/v1/Managers/{}/EthernetInterfaces/{}",
                                BMCWEB_REDFISH_MANAGER_URI_NAME,
                                extractParentInterfaceName(ifaceId));
        jsonResponse["Links"]["RelatedInterfaces"] =
            std::move(relatedInterfaces);
    }
    else
    {
        jsonResponse["EthernetInterfaceType"] =
            ethernet_interface::EthernetDeviceType::Physical;
    }

    jsonResponse["NameServers"] = ethData.nameServers;
    jsonResponse["StaticNameServers"] = ethData.staticNameServers;

    nlohmann::json& ipv4Array = jsonResponse["IPv4Addresses"];
    nlohmann::json& ipv4StaticArray = jsonResponse["IPv4StaticAddresses"];
    ipv4Array = nlohmann::json::array();
    ipv4StaticArray = nlohmann::json::array();
    for (const auto& ipv4Config : ipv4Data)
    {
        std::string gatewayStr = ipv4Config.gateway;
        if (gatewayStr.empty())
        {
            gatewayStr = "0.0.0.0";
        }
        nlohmann::json::object_t ipv4;
        ipv4["AddressOrigin"] = ipv4Config.origin;
        ipv4["SubnetMask"] = ipv4Config.netmask;
        ipv4["Address"] = ipv4Config.address;
        ipv4["Gateway"] = gatewayStr;

        if (ipv4Config.origin == "Static")
        {
            ipv4StaticArray.push_back(ipv4);
        }

        ipv4Array.emplace_back(std::move(ipv4));
    }

    std::string ipv6GatewayStr = ethData.ipv6DefaultGateway;
    if (ipv6GatewayStr.empty())
    {
        ipv6GatewayStr = "0:0:0:0:0:0:0:0";
    }

    jsonResponse["IPv6DefaultGateway"] = ipv6GatewayStr;

    nlohmann::json::array_t ipv6StaticGatewayArray;
    for (const auto& ipv6GatewayConfig : ipv6GatewayData)
    {
        nlohmann::json::object_t ipv6Gateway;
        ipv6Gateway["Address"] = ipv6GatewayConfig.gateway;
        ipv6StaticGatewayArray.emplace_back(std::move(ipv6Gateway));
    }
    jsonResponse["IPv6StaticDefaultGateways"] =
        std::move(ipv6StaticGatewayArray);

    nlohmann::json& ipv6Array = jsonResponse["IPv6Addresses"];
    nlohmann::json& ipv6StaticArray = jsonResponse["IPv6StaticAddresses"];
    ipv6Array = nlohmann::json::array();
    ipv6StaticArray = nlohmann::json::array();
    nlohmann::json& ipv6AddrPolicyTable =
        jsonResponse["IPv6AddressPolicyTable"];
    ipv6AddrPolicyTable = nlohmann::json::array();
    for (const auto& ipv6Config : ipv6Data)
    {
        nlohmann::json::object_t ipv6;
        ipv6["Address"] = ipv6Config.address;
        ipv6["PrefixLength"] = ipv6Config.prefixLength;
        ipv6["AddressOrigin"] = ipv6Config.origin;

        ipv6Array.emplace_back(std::move(ipv6));
        if (ipv6Config.origin == "Static")
        {
            nlohmann::json::object_t ipv6Static;
            ipv6Static["Address"] = ipv6Config.address;
            ipv6Static["PrefixLength"] = ipv6Config.prefixLength;
            ipv6StaticArray.emplace_back(std::move(ipv6Static));
        }
    }
}

inline void afterDelete(const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
                        const std::string& ifaceId,
                        const boost::system::error_code& ec,
                        const sdbusplus::message_t& m)
{
    if (!ec)
    {
        return;
    }
    const sd_bus_error* dbusError = m.get_error();
    if (dbusError == nullptr)
    {
        messages::internalError(asyncResp->res);
        return;
    }
    BMCWEB_LOG_DEBUG("DBus error: {}", dbusError->name);

    if (std::string_view("org.freedesktop.DBus.Error.UnknownObject") ==
        dbusError->name)
    {
        messages::resourceNotFound(asyncResp->res, "EthernetInterface",
                                   ifaceId);
        return;
    }
    if (std::string_view("org.freedesktop.DBus.Error.UnknownMethod") ==
        dbusError->name)
    {
        messages::resourceCannotBeDeleted(asyncResp->res);
        return;
    }
    messages::internalError(asyncResp->res);
}

inline void afterVlanCreate(
    const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
    const std::string& parentInterfaceUri, const std::string& vlanInterface,
    const boost::system::error_code& ec, const sdbusplus::message_t& m

)
{
    if (ec)
    {
        const sd_bus_error* dbusError = m.get_error();
        if (dbusError == nullptr)
        {
            messages::internalError(asyncResp->res);
            return;
        }
        BMCWEB_LOG_DEBUG("DBus error: {}", dbusError->name);

        if (std::string_view(
                "xyz.openbmc_project.Common.Error.ResourceNotFound") ==
            dbusError->name)
        {
            messages::propertyValueNotInList(
                asyncResp->res, parentInterfaceUri,
                "Links/RelatedInterfaces/0/@odata.id");
            return;
        }
        if (std::string_view(
                "xyz.openbmc_project.Common.Error.InvalidArgument") ==
            dbusError->name)
        {
            messages::resourceAlreadyExists(asyncResp->res, "EthernetInterface",
                                            "Id", vlanInterface);
            return;
        }
        messages::internalError(asyncResp->res);
        return;
    }

    const boost::urls::url vlanInterfaceUri =
        boost::urls::format("/redfish/v1/Managers/{}/EthernetInterfaces/{}",
                            BMCWEB_REDFISH_MANAGER_URI_NAME, vlanInterface);
    asyncResp->res.addHeader("Location", vlanInterfaceUri.buffer());
}

inline void requestEthernetInterfacesRoutes(App& app)
{
    BMCWEB_ROUTE(app, "/redfish/v1/Managers/<str>/EthernetInterfaces/")
        .privileges(redfish::privileges::getEthernetInterfaceCollection)
        .methods(boost::beast::http::verb::get)(
            // ast-grep-ignore: long-lambda
            // ast-grep-ignore: long-lambda（忽略指令：ast-grep 跳过过长 lambda 检查）
            [&app](const crow::Request& req,
                   const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
                   const std::string& managerId) {
                if (!redfish::setUpRedfishRoute(app, req, asyncResp))
                {
                    return;
                }

                if (managerId != BMCWEB_REDFISH_MANAGER_URI_NAME)
                {
                    messages::resourceNotFound(asyncResp->res, "Manager",
                                               managerId);
                    return;
                }

                asyncResp->res.jsonValue["@odata.type"] =
                    "#EthernetInterfaceCollection.EthernetInterfaceCollection";
                asyncResp->res.jsonValue["@odata.id"] = boost::urls::format(
                    "/redfish/v1/Managers/{}/EthernetInterfaces",
                    BMCWEB_REDFISH_MANAGER_URI_NAME);
                asyncResp->res.jsonValue["Name"] =
                    "Ethernet Network Interface Collection";
                asyncResp->res.jsonValue["Description"] =
                    "Collection of EthernetInterfaces for this Manager";

                // Get eth interface list, and call the below callback for JSON
                // preparation
                // 获取 eth 接口列表，并调用下面的回调准备 JSON。
                getEthernetIfaceList(
                    // ast-grep-ignore: long-lambda
                    // ast-grep-ignore: long-lambda（忽略指令：ast-grep 跳过过长 lambda 检查）
                    [asyncResp](const bool& success,
                                const std::vector<std::string>& ifaceList) {
                        if (!success)
                        {
                            messages::internalError(asyncResp->res);
                            return;
                        }

                        nlohmann::json& ifaceArray =
                            asyncResp->res.jsonValue["Members"];
                        ifaceArray = nlohmann::json::array();
                        for (const std::string& ifaceItem : ifaceList)
                        {
                            nlohmann::json::object_t iface;
                            iface["@odata.id"] = boost::urls::format(
                                "/redfish/v1/Managers/{}/EthernetInterfaces/{}",
                                BMCWEB_REDFISH_MANAGER_URI_NAME, ifaceItem);
                            ifaceArray.push_back(std::move(iface));
                        }

                        asyncResp->res.jsonValue["Members@odata.count"] =
                            ifaceArray.size();
                        asyncResp->res.jsonValue["@odata.id"] =
                            boost::urls::format(
                                "/redfish/v1/Managers/{}/EthernetInterfaces",
                                BMCWEB_REDFISH_MANAGER_URI_NAME);
                    });
            });

    BMCWEB_ROUTE(app, "/redfish/v1/Managers/<str>/EthernetInterfaces/")
        .privileges(
            redfish::privileges::
                postEthernetInterfaceSubOverManagerEthernetInterfaceCollection)
        .methods(boost::beast::http::verb::post)(
            // ast-grep-ignore: long-lambda
            // ast-grep-ignore: long-lambda（忽略指令：ast-grep 跳过过长 lambda 检查）
            [&app](const crow::Request& req,
                   const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
                   const std::string& managerId) {
                if (!redfish::setUpRedfishRoute(app, req, asyncResp))
                {
                    return;
                }

                if (managerId != BMCWEB_REDFISH_MANAGER_URI_NAME)
                {
                    messages::resourceNotFound(asyncResp->res, "Manager",
                                               managerId);
                    return;
                }

                bool vlanEnable = false;
                uint32_t vlanId = 0;
                std::vector<nlohmann::json::object_t> relatedInterfaces;

                if (!json_util::readJsonPatch(                        //
                        req, asyncResp->res,                          //
                        "Links/RelatedInterfaces", relatedInterfaces, //
                        "VLAN/VLANEnable", vlanEnable,                //
                        "VLAN/VLANId", vlanId                         //
                        ))
                {
                    return;
                }

                if (relatedInterfaces.size() != 1)
                {
                    messages::arraySizeTooLong(asyncResp->res,
                                               "Links/RelatedInterfaces",
                                               relatedInterfaces.size());
                    return;
                }

                std::string parentInterfaceUri;
                if (!json_util::readJsonObject(relatedInterfaces[0],
                                               asyncResp->res, "@odata.id",
                                               parentInterfaceUri))
                {
                    messages::propertyMissing(
                        asyncResp->res, "Links/RelatedInterfaces/0/@odata.id");
                    return;
                }
                BMCWEB_LOG_INFO("Parent Interface URI: {}", parentInterfaceUri);

                boost::system::result<boost::urls::url_view> parsedUri =
                    boost::urls::parse_relative_ref(parentInterfaceUri);
                if (!parsedUri)
                {
                    messages::propertyValueFormatError(
                        asyncResp->res, parentInterfaceUri,
                        "Links/RelatedInterfaces/0/@odata.id");
                    return;
                }

                std::string parentInterface;
                if (!crow::utility::readUrlSegments(
                        *parsedUri, "redfish", "v1", "Managers", "bmc",
                        "EthernetInterfaces", std::ref(parentInterface)))
                {
                    messages::propertyValueNotInList(
                        asyncResp->res, parentInterfaceUri,
                        "Links/RelatedInterfaces/0/@odata.id");
                    return;
                }

                if (!vlanEnable)
                {
                    // In OpenBMC implementation, VLANEnable cannot be false on
                    // create
                    // 在 OpenBMC 实现中，创建时 VLANEnable 不能为 false。
                    messages::propertyValueIncorrect(asyncResp->res,
                                                     "VLAN/VLANEnable", false);
                    return;
                }

                std::string vlanInterface =
                    parentInterface + "_" + std::to_string(vlanId);
                dbus::utility::async_method_call(
                    asyncResp,
                    [asyncResp, parentInterfaceUri,
                     vlanInterface](const boost::system::error_code& ec,
                                    const sdbusplus::message_t& m) {
                        afterVlanCreate(asyncResp, parentInterfaceUri,
                                        vlanInterface, ec, m);
                    },
                    "xyz.openbmc_project.Network",
                    "/xyz/openbmc_project/network",
                    "xyz.openbmc_project.Network.VLAN.Create", "VLAN",
                    parentInterface, vlanId);
            });

    BMCWEB_ROUTE(app, "/redfish/v1/Managers/<str>/EthernetInterfaces/<str>/")
        .privileges(redfish::privileges::getEthernetInterface)
        .methods(boost::beast::http::verb::get)(
            // ast-grep-ignore: long-lambda
            // ast-grep-ignore: long-lambda（忽略指令：ast-grep 跳过过长 lambda 检查）
            [&app](const crow::Request& req,
                   const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
                   const std::string& managerId, const std::string& ifaceId) {
                if (!redfish::setUpRedfishRoute(app, req, asyncResp))
                {
                    return;
                }

                if (managerId != BMCWEB_REDFISH_MANAGER_URI_NAME)
                {
                    messages::resourceNotFound(asyncResp->res, "Manager",
                                               managerId);
                    return;
                }

                getEthernetIfaceData(
                    ifaceId,
                    // ast-grep-ignore: long-lambda
                    // ast-grep-ignore: long-lambda（忽略指令：ast-grep 跳过过长 lambda 检查）
                    [asyncResp, ifaceId](
                        const bool& success,
                        const EthernetInterfaceData& ethData,
                        const std::vector<IPv4AddressData>& ipv4Data,
                        const std::vector<IPv6AddressData>& ipv6Data,
                        const std::vector<StaticGatewayData>& ipv6GatewayData) {
                        if (!success)
                        {
                            // TODO(Pawel)consider distinguish between non
                            // existing object, and other errors
                            // TODO(Pawel)：考虑区分不存在的对象和其他错误。
                            messages::resourceNotFound(
                                asyncResp->res, "EthernetInterface", ifaceId);
                            return;
                        }

                        asyncResp->res.jsonValue["@odata.type"] =
                            "#EthernetInterface.v1_9_0.EthernetInterface";
                        asyncResp->res.jsonValue["Name"] =
                            "Manager Ethernet Interface";
                        asyncResp->res.jsonValue["Description"] =
                            "Management Network Interface";

                        parseInterfaceData(asyncResp, ifaceId, ethData,
                                           ipv4Data, ipv6Data, ipv6GatewayData);
                    });
            });

    BMCWEB_ROUTE(app, "/redfish/v1/Managers/<str>/EthernetInterfaces/<str>/")
        .privileges(
            redfish::privileges::
                patchEthernetInterfaceSubOverManagerEthernetInterfaceCollection)
        .methods(boost::beast::http::verb::patch)(
            // ast-grep-ignore: long-lambda
            // ast-grep-ignore: long-lambda（忽略指令：ast-grep 跳过过长 lambda 检查）
            [&app](const crow::Request& req,
                   const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
                   const std::string& managerId, const std::string& ifaceId) {
                if (!redfish::setUpRedfishRoute(app, req, asyncResp))
                {
                    return;
                }

                if (managerId != BMCWEB_REDFISH_MANAGER_URI_NAME)
                {
                    messages::resourceNotFound(asyncResp->res, "Manager",
                                               managerId);
                    return;
                }

                std::optional<std::string> hostname;
                std::optional<std::string> fqdn;
                std::optional<std::string> macAddress;
                std::optional<std::string> ipv6DefaultGateway;
                std::optional<std::vector<
                    std::variant<nlohmann::json::object_t, std::nullptr_t>>>
                    ipv4StaticAddresses;
                std::optional<std::vector<
                    std::variant<nlohmann::json::object_t, std::nullptr_t>>>
                    ipv6StaticAddresses;
                std::optional<std::vector<
                    std::variant<nlohmann::json::object_t, std::nullptr_t>>>
                    ipv6StaticDefaultGateways;
                std::optional<std::vector<std::string>> staticNameServers;
                std::optional<bool> ipv6AutoConfigEnabled;
                std::optional<bool> interfaceEnabled;
                std::optional<size_t> mtuSize;
                DHCPParameters v4dhcpParms;
                DHCPParameters v6dhcpParms;

                if (!json_util::readJsonPatch(                             //
                        req, asyncResp->res,                               //
                        "DHCPv4/DHCPEnabled", v4dhcpParms.dhcpv4Enabled,   //
                        "DHCPv4/UseDNSServers", v4dhcpParms.useDnsServers, //
                        "DHCPv4/UseDomainName", v4dhcpParms.useDomainName, //
                        "DHCPv4/UseNTPServers", v4dhcpParms.useNtpServers, //
                        "DHCPv6/OperatingMode",
                        v6dhcpParms.dhcpv6OperatingMode,                   //
                        "DHCPv6/UseDNSServers", v6dhcpParms.useDnsServers, //
                        "DHCPv6/UseDomainName", v6dhcpParms.useDomainName, //
                        "DHCPv6/UseNTPServers", v6dhcpParms.useNtpServers, //
                        "FQDN", fqdn,                                      //
                        "HostName", hostname,                              //
                        "InterfaceEnabled", interfaceEnabled,              //
                        "IPv4StaticAddresses", ipv4StaticAddresses,        //
                        "IPv6DefaultGateway", ipv6DefaultGateway,          //
                        "IPv6StaticAddresses", ipv6StaticAddresses,        //
                        "IPv6StaticDefaultGateways",
                        ipv6StaticDefaultGateways,                         //
                        "InterfaceEnabled", interfaceEnabled,              //
                        "MACAddress", macAddress,                          //
                        "MTUSize", mtuSize,                                //
                        "StatelessAddressAutoConfig/IPv6AutoConfigEnabled",
                        ipv6AutoConfigEnabled,                             //
                        "StaticNameServers", staticNameServers             //
                        ))
                {
                    return;
                }

                // Get single eth interface data, and call the below callback
                // for JSON preparation
                // 获取单个 eth 接口的数据，并调用下面的回调准备 JSON。
                getEthernetIfaceData(
                    ifaceId,
                    // ast-grep-ignore: long-lambda
                    // ast-grep-ignore: long-lambda（忽略指令：ast-grep 跳过过长 lambda 检查）
                    [asyncResp, ifaceId, hostname = std::move(hostname),
                     fqdn = std::move(fqdn), macAddress = std::move(macAddress),
                     ipv4StaticAddresses = std::move(ipv4StaticAddresses),
                     ipv6DefaultGateway = std::move(ipv6DefaultGateway),
                     ipv6StaticAddresses = std::move(ipv6StaticAddresses),
                     ipv6StaticDefaultGateway =
                         std::move(ipv6StaticDefaultGateways),
                     staticNameServers = std::move(staticNameServers), mtuSize,
                     ipv6AutoConfigEnabled,
                     v4dhcpParms = std::move(v4dhcpParms),
                     v6dhcpParms = std::move(v6dhcpParms), interfaceEnabled](
                        const bool success,
                        const EthernetInterfaceData& ethData,
                        const std::vector<IPv4AddressData>& ipv4Data,
                        const std::vector<IPv6AddressData>& ipv6Data,
                        const std::vector<StaticGatewayData>&
                            ipv6GatewayData) mutable {
                        if (!success)
                        {
                            // ... otherwise return error
                            // TODO(Pawel)consider distinguish between non
                            // existing object, and other errors
                            // ...否则返回错误。
                            // TODO(Pawel)：考虑区分不存在的对象和其他错误。
                            messages::resourceNotFound(
                                asyncResp->res, "EthernetInterface", ifaceId);
                            return;
                        }

                        handleDHCPPatch(ifaceId, ethData, v4dhcpParms,
                                        v6dhcpParms, asyncResp);

                        if (hostname)
                        {
                            handleHostnamePatch(*hostname, asyncResp);
                        }

                        if (ipv6AutoConfigEnabled)
                        {
                            handleSLAACAutoConfigPatch(
                                ifaceId, *ipv6AutoConfigEnabled, asyncResp);
                        }

                        if (fqdn)
                        {
                            handleFqdnPatch(ifaceId, *fqdn, asyncResp);
                        }

                        if (macAddress)
                        {
                            handleMACAddressPatch(ifaceId, *macAddress,
                                                  asyncResp);
                        }

                        if (ipv4StaticAddresses)
                        {
                            handleIPv4StaticPatch(ifaceId, *ipv4StaticAddresses,
                                                  ethData, ipv4Data, asyncResp);
                        }

                        if (staticNameServers)
                        {
                            handleStaticNameServersPatch(
                                ifaceId, *staticNameServers, asyncResp);
                        }

                        if (ipv6DefaultGateway)
                        {
                            messages::propertyNotWritable(asyncResp->res,
                                                          "IPv6DefaultGateway");
                        }

                        if (ipv6StaticAddresses)
                        {
                            handleIPv6StaticAddressesPatch(ifaceId,
                                                           *ipv6StaticAddresses,
                                                           ipv6Data, asyncResp);
                        }

                        if (ipv6StaticDefaultGateway)
                        {
                            handleIPv6DefaultGateway(
                                ifaceId, *ipv6StaticDefaultGateway,
                                ipv6GatewayData, asyncResp);
                        }

                        if (interfaceEnabled)
                        {
                            setDbusProperty(
                                asyncResp, "InterfaceEnabled",
                                "xyz.openbmc_project.Network",
                                sdbusplus::object_path(
                                    "/xyz/openbmc_project/network") /
                                    ifaceId,
                                "xyz.openbmc_project.Network.EthernetInterface",
                                "NICEnabled", *interfaceEnabled);
                        }

                        if (mtuSize)
                        {
                            handleMTUSizePatch(ifaceId, *mtuSize, asyncResp);
                        }
                    });
            });

    BMCWEB_ROUTE(app, "/redfish/v1/Managers/<str>/EthernetInterfaces/<str>/")
        .privileges(
            redfish::privileges::
                deleteEthernetInterfaceSubOverManagerEthernetInterfaceCollection)
        .methods(boost::beast::http::verb::delete_)(
            // ast-grep-ignore: long-lambda
            // ast-grep-ignore: long-lambda（忽略指令：ast-grep 跳过过长 lambda 检查）
            [&app](const crow::Request& req,
                   const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
                   const std::string& managerId, const std::string& ifaceId) {
                if (!redfish::setUpRedfishRoute(app, req, asyncResp))
                {
                    return;
                }

                if (managerId != BMCWEB_REDFISH_MANAGER_URI_NAME)
                {
                    messages::resourceNotFound(asyncResp->res, "Manager",
                                               managerId);
                    return;
                }

                dbus::utility::async_method_call(
                    asyncResp,
                    [asyncResp, ifaceId](const boost::system::error_code& ec,
                                         const sdbusplus::message_t& m) {
                        afterDelete(asyncResp, ifaceId, ec, m);
                    },
                    "xyz.openbmc_project.Network",
                    std::string("/xyz/openbmc_project/network/") + ifaceId,
                    "xyz.openbmc_project.Object.Delete", "Delete");
            });
}

} // namespace redfish
