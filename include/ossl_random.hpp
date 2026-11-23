// SPDX-License-Identifier: Apache-2.0
// SPDX-FileCopyrightText: Copyright OpenBMC Authors
#pragma once

#include <cstddef>
#include <cstdint>
#include <limits>
#include <string>
#include <string_view>

namespace bmcweb
{

struct OpenSSLGenerator
{
    uint8_t operator()();

    static constexpr uint8_t max()
    {
        return std::numeric_limits<uint8_t>::max();
    }
    static constexpr uint8_t min()
    {
        return std::numeric_limits<uint8_t>::min();
    }

    bool error() const
    {
        return err;
    }

    // all generators require this variable
    // 所有生成器都需要这个变量
    using result_type = uint8_t;

  private:
    // RAND_bytes() returns 1 on success, 0 otherwise. -1 if bad function
    // RAND_bytes() 成功时返回 1，否则返回 0；函数本身出错时返回 -1
    static constexpr int opensslSuccess = 1;
    bool err = false;
};

std::string getRandomUUID();

std::string getRandomIdOfLength(size_t length);

bool constantTimeStringCompare(std::string_view a, std::string_view b);
struct ConstantTimeCompare
{
    bool operator()(std::string_view a, std::string_view b) const;
};

} // namespace bmcweb
