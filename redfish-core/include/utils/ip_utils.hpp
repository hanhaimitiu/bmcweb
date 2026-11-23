// SPDX-License-Identifier: Apache-2.0
// SPDX-FileCopyrightText: Copyright OpenBMC Authors
#pragma once

#include <boost/asio/ip/address.hpp>
#include <boost/asio/ip/address_v4.hpp>
#include <boost/asio/ip/address_v6.hpp>

#include <cstdint>
#include <string>

namespace redfish
{
namespace ip_util
{

/**
 * @brief Converts boost::asio::ip::address to string
 * Will automatically convert IPv4-mapped IPv6 address back to IPv4.
 *
 * @param[in] ipAddr IP address to convert
 *
 * @return IP address string
 */
/* @brief 将 boost::asio::ip::address 转换为字符串
 * 会自动将 IPv4-mapped 的 IPv6 地址转回 IPv4。
 * @param[in] ipAddr 要转换的 IP 地址
 * @return IP 地址字符串 */
inline std::string toString(const boost::asio::ip::address& ipAddr)
{
    if (ipAddr.is_v6() && ipAddr.to_v6().is_v4_mapped())
    {
        return boost::asio::ip::make_address_v4(boost::asio::ip::v4_mapped,
                                                ipAddr.to_v6())
            .to_string();
    }
    return ipAddr.to_string();
}

/**
 * @brief Helper function that verifies IP address to check if it is in
 *        proper format. If bits pointer is provided, also calculates active
 *        bit count for Subnet Mask.
 *
 * @param[in]  ip     IP that will be verified
 * @param[out] bits   Calculated mask in bits notation
 *
 * @return true in case of success, false otherwise
 */
/* @brief 辅助函数，验证 IP 地址以检查其格式是否
 *        正确。如果提供了 bits 指针，还会计算子网掩码的
 *        有效位数。
 * @param[in]  ip    要验证的 IP
 * @param[out] bits  以位数表示的计算所得掩码
 * @return 成功返回 true，否则返回 false */
inline bool ipv4VerifyIpAndGetBitcount(const std::string& ip,
                                       uint8_t* prefixLength = nullptr)
{
    boost::system::error_code ec;
    boost::asio::ip::address_v4 addr = boost::asio::ip::make_address_v4(ip, ec);
    if (ec)
    {
        return false;
    }

    if (prefixLength != nullptr)
    {
        uint8_t prefix = 0;
        boost::asio::ip::address_v4::bytes_type maskBytes = addr.to_bytes();
        bool maskFinished = false;
        for (unsigned char byte : maskBytes)
        {
            if (maskFinished)
            {
                if (byte != 0U)
                {
                    return false;
                }
                continue;
            }
            switch (byte)
            {
                case 255:
                    prefix += 8;
                    break;
                case 254:
                    prefix += 7;
                    maskFinished = true;
                    break;
                case 252:
                    prefix += 6;
                    maskFinished = true;
                    break;
                case 248:
                    prefix += 5;
                    maskFinished = true;
                    break;
                case 240:
                    prefix += 4;
                    maskFinished = true;
                    break;
                case 224:
                    prefix += 3;
                    maskFinished = true;
                    break;
                case 192:
                    prefix += 2;
                    maskFinished = true;
                    break;
                case 128:
                    prefix += 1;
                    maskFinished = true;
                    break;
                case 0:
                    maskFinished = true;
                    break;
                default:
                    // Invalid netmask
                    // 无效的 netmask（子网掩码）
                    return false;
            }
        }
        *prefixLength = prefix;
    }

    return true;
}

} // namespace ip_util
} // namespace redfish
