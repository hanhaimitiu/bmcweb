// SPDX-License-Identifier: Apache-2.0
// SPDX-FileCopyrightText: Copyright OpenBMC Authors
#include "registries.hpp"

// We need the registries_selector pulled into some cpp part so that the
// registration hooks run.
// NOLINTNEXTLINE(misc-include-cleaner)
// 我们需要把 registries_selector 引入到某个 cpp 部分，
// 以便注册钩子能够运行。
// NOLINTNEXTLINE(misc-include-cleaner)
#include "registries_selector.hpp"
#include "str_utility.hpp"

#include <algorithm>
#include <cstring>
#include <functional>
#include <map>
#include <optional>
#include <ranges>
#include <span>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace redfish::registries
{

auto allRegistries() -> std::map<std::string, RegistryEntry>&
{
    static std::map<std::string, RegistryEntry> registries;
    return registries;
}

auto getRegistryFromPrefix(const std::string& registryName)
    -> std::optional<RegistryEntryRef>
{
    auto& registries = allRegistries();
    if (auto it = registries.find(registryName); it != registries.end())
    {
        return std::ref(it->second);
    }

    return std::nullopt;
}

auto getRegistryMessagesFromPrefix(const std::string& registryName)
    -> MessageEntries
{
    auto registry = getRegistryFromPrefix(registryName);
    if (!registry)
    {
        return {};
    }

    return registry->get().entries;
}

const Message* getMessageFromRegistry(const std::string& messageKey,
                                      std::span<const MessageEntry> registry)
{
    std::span<const MessageEntry>::iterator messageIt = std::ranges::find_if(
        registry, [&messageKey](const MessageEntry& messageEntry) {
            return std::strcmp(messageEntry.first, messageKey.c_str()) == 0;
        });
    if (messageIt != registry.end())
    {
        return &messageIt->second;
    }

    return nullptr;
}

std::optional<MessageId> getMessageComponents(std::string_view message)
{
    // Redfish Message are in the form
    // RegistryName.MajorVersion.MinorVersion.MessageKey
    // Redfish 消息的格式为
    // RegistryName.MajorVersion.MinorVersion.MessageKey
    std::vector<std::string> fields;
    fields.reserve(4);
    bmcweb::split(fields, message, '.');
    if (fields.size() != 4)
    {
        return std::nullopt;
    }

    return MessageId(std::move(fields[0]), std::move(fields[1]),
                     std::move(fields[2]), std::move(fields[3]));
}

const Message* getMessage(std::string_view messageID)
{
    std::optional<MessageId> msgComponents = getMessageComponents(messageID);
    if (!msgComponents)
    {
        return nullptr;
    }

    // Find the right registry and check it for the MessageKey
    // 找到正确的注册表并检查其中的 MessageKey
    return getMessageFromRegistry(
        msgComponents->messageKey,
        getRegistryMessagesFromPrefix(msgComponents->registryName));
}

} // namespace redfish::registries
