// SPDX-License-Identifier: Apache-2.0
// SPDX-FileCopyrightText: Copyright OpenBMC Authors
#pragma once

#include <array>
#include <string_view>

namespace redfish
{

// Interfaces which imply a D-Bus object represents a Processor
// 表示某 D-Bus 对象代表一个 Processor 的接口
constexpr std::array<std::string_view, 2> processorInterfaces = {
    "xyz.openbmc_project.Inventory.Item.Cpu",
    "xyz.openbmc_project.Inventory.Item.Accelerator"};

// Interfaces which imply a D-Bus object represents a Processor Core
// 表示某 D-Bus 对象代表一个 Processor Core 的接口
constexpr std::array<std::string_view, 1> processorCoreInterfaces = {
    "xyz.openbmc_project.Inventory.Item.CpuCore"};

} // namespace redfish
