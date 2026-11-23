// SPDX-License-Identifier: Apache-2.0
// SPDX-FileCopyrightText: Copyright OpenBMC Authors
#pragma once

#include <algorithm>
#include <cstddef>
#include <ranges>
#include <string>
#include <string_view>
#include <vector>

namespace bmcweb
{
// This is a naive replacement for boost::split until
// https://github.com/llvm/llvm-project/issues/40486
// is resolved
// 这是 boost::split 的一个简易替代，直到
// https://github.com/llvm/llvm-project/issues/40486
// 得到解决
inline void split(std::vector<std::string>& strings, std::string_view str,
                  char delim)
{
    size_t start = 0;
    size_t end = 0;
    while (end <= str.size())
    {
        end = str.find(delim, start);
        strings.emplace_back(str.substr(start, end - start));
        start = end + 1;
    }
}

inline char asciiToLower(char c)
{
    // Converts a character to lower case without relying on std::locale
    // 不依赖 std::locale 将字符转换为小写
    if ('A' <= c && c <= 'Z')
    {
        c -= static_cast<char>('A' - 'a');
    }
    return c;
}

inline bool asciiIEquals(std::string_view left, std::string_view right)
{
    return std::ranges::equal(left, right, [](char lChar, char rChar) {
        return asciiToLower(lChar) == asciiToLower(rChar);
    });
}

} // namespace bmcweb
