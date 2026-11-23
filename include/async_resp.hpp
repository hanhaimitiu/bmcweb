// SPDX-License-Identifier: Apache-2.0
// SPDX-FileCopyrightText: Copyright OpenBMC Authors
#pragma once

#include "http_response.hpp"

#include <utility>

namespace bmcweb
{

/**
 * AsyncResp
 * Gathers data needed for response processing after async calls are done
 */
/* AsyncResp
 * 收集异步调用完成后进行响应处理所需的数据 */

class AsyncResp
{
  public:
    AsyncResp() = default;
    explicit AsyncResp(crow::Response&& resIn) : res(std::move(resIn)) {}

    AsyncResp(const AsyncResp&) = delete;
    AsyncResp(AsyncResp&&) = delete;
    AsyncResp& operator=(const AsyncResp&) = delete;
    AsyncResp& operator=(AsyncResp&&) = delete;

    ~AsyncResp()
    {
        res.end();
    }

    crow::Response res;
};

} // namespace bmcweb
