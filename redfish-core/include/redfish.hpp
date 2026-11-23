// SPDX-License-Identifier: Apache-2.0
// SPDX-FileCopyrightText: Copyright OpenBMC Authors
#pragma once

#include "app.hpp"
#include "async_resp.hpp"
#include "http_request.hpp"
#include "redfish_oem_routing.hpp"
#include "sub_request.hpp"
#include "verb.hpp"

#include <memory>

namespace redfish
{
/*
 * @brief Top level class installing and providing Redfish services
 */
/* @brief 安装并提供 Redfish 服务的顶层类 */
class RedfishService
{
  public:
    /*
     * @brief Redfish service constructor
     *
     * Loads Redfish configuration and installs schema resources
     *
     * @param[in] app   Crow app on which Redfish will initialize
     */
    /* @brief Redfish 服务构造函数
     * 加载 Redfish 配置并安装 schema 资源
     * @param[in] app   Redfish 将在其上初始化的 Crow app */
    explicit RedfishService(App& app);

    // Temporary change to make redfish instance available in other places
    // like query delegation.
    // 临时改动，使 redfish 实例可在其他地方使用，
    // 例如查询委派（query delegation）。
    static RedfishService& getInstance(App& app)
    {
        static RedfishService redfish(app);
        return redfish;
    }

    void validate()
    {
        oemRouter.validate();
    }

    template <StringLiteral Rule>
    auto& newRoute(HttpVerb method)
    {
        return oemRouter.newRule<Rule>(method);
    }

    void handleSubRoute(
        const crow::Request& req,
        const std::shared_ptr<bmcweb::AsyncResp>& asyncResp) const
    {
        auto subReq = std::make_shared<SubRequest>(req);
        if (!subReq->needHandling())
        {
            return;
        }
        oemRouter.handle(subReq, asyncResp);
    }

    OemRouter oemRouter;
};

template <StringLiteral Path>
auto& REDFISH_SUB_ROUTE(RedfishService& service, HttpVerb method)
{
    return service.newRoute<Path>(method);
}

} // namespace redfish
