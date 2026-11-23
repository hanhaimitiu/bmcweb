// SPDX-License-Identifier: Apache-2.0
// SPDX-FileCopyrightText: Copyright OpenBMC Authors
#pragma once
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
 * 这是一个自动生成的头文件，其中包含
 * Redfish DMTF 定义消息的相关定义。
 * 除运行 parse_registries.py 脚本外，
 * 请勿修改此注册表。本文件中所含的
 * 定义归 DMTF 所有。对这些文件的任何修改，
 * 都应先推送到 DMTF github 组织中的
 * 相应注册表。 */
#include "registries.hpp"

#include <array>

// clang-format off

namespace redfish::registries
{
struct Platform
{
static constexpr Header header = {
    "Copyright 2023 DMTF. All rights reserved.",
    "#MessageRegistry.v1_6_2.MessageRegistry",
    1,
    0,
    1,
    "Compute Platform Message Registry",
    "en",
    "This registry defines messages for compute platforms, covering topics related to processor, memory, and I/O device connectivity.",
    "Platform",
    "DMTF",
};

static constexpr const char* url =
    "https://redfish.dmtf.org/registries/Platform.1.0.1.json";

static constexpr std::array registry =
{
    MessageEntry{
        "OperatingSystemCrash",
        {
            "Indicates the operating system was halted due to a catastrophic error.",
            "An operating system crash occurred.",
            "Critical",
            0,
            {},
            "Check additional diagnostic data if available.",
        }},
    MessageEntry{
        "PlatformError",
        {
            "Indicates that a platform error occurred.",
            "A platform error occurred.",
            "Warning",
            0,
            {},
            "Check additional diagnostic data if available.",
        }},
    MessageEntry{
        "PlatformErrorAtLocation",
        {
            "Indicates that a platform error occurred and device or other location information is available.",
            "A platform error occurred at location '%1'.",
            "Warning",
            1,
            {
                "string",
            },
            "Check additional diagnostic data if available.",
        }},
    MessageEntry{
        "UnhandledExceptionDetectedAfterReset",
        {
            "Indicates that an unhandled exception caused the platform to reset.",
            "An unhandled exception caused a platform reset.",
            "Critical",
            0,
            {},
            "Check additional diagnostic data if available.",
        }},

};

enum class Index
{
    operatingSystemCrash = 0,
    platformError = 1,
    platformErrorAtLocation = 2,
    unhandledExceptionDetectedAfterReset = 3,
};
}; // struct platform

[[gnu::constructor]] inline void registerPlatform()
{ registerRegistry<Platform>(); }

} // namespace redfish::registries
