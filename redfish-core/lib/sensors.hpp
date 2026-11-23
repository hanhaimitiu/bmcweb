// SPDX-License-Identifier: Apache-2.0
// SPDX-FileCopyrightText: Copyright OpenBMC Authors
// SPDX-FileCopyrightText: Copyright 2018 Intel Corporation
#pragma once

#include "bmcweb_config.h"

#include "app.hpp"
#include "async_resp.hpp"
#include "dbus_singleton.hpp"
#include "dbus_utility.hpp"
#include "error_messages.hpp"
#include "generated/enums/redundancy.hpp"
#include "generated/enums/resource.hpp"
#include "http_request.hpp"
#include "logging.hpp"
#include "query.hpp"
#include "registries/privilege_registry.hpp"
#include "str_utility.hpp"
#include "utils/chassis_utils.hpp"
#include "utils/dbus_utils.hpp"
#include "utils/json_utils.hpp"
#include "utils/query_param.hpp"
#include "utils/sensor_utils.hpp"

#include <asm-generic/errno.h>

#include <boost/beast/http/status.hpp>
#include <boost/beast/http/verb.hpp>
#include <boost/system/error_code.hpp>
#include <boost/url/format.hpp>
#include <boost/url/url.hpp>
#include <nlohmann/json.hpp>
#include <sdbusplus/message/native_types.hpp>
#include <sdbusplus/unpack_properties.hpp>

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <iterator>
#include <map>
#include <memory>
#include <optional>
#include <ranges>
#include <set>
#include <span>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

namespace redfish
{

namespace sensors
{

// clang-format off
namespace dbus
{
constexpr auto powerPaths = std::to_array<std::string_view>({
    "/xyz/openbmc_project/sensors/voltage",
    "/xyz/openbmc_project/sensors/power"
});

constexpr auto getSensorPaths(){
    if constexpr(BMCWEB_REDFISH_NEW_POWERSUBSYSTEM_THERMALSUBSYSTEM){
    return std::to_array<std::string_view>({
        "/xyz/openbmc_project/sensors/power",
        "/xyz/openbmc_project/sensors/current",
        "/xyz/openbmc_project/sensors/airflow",
        "/xyz/openbmc_project/sensors/humidity",
        "/xyz/openbmc_project/sensors/voltage",
        "/xyz/openbmc_project/sensors/fan_tach",
        "/xyz/openbmc_project/sensors/temperature",
        "/xyz/openbmc_project/sensors/fan_pwm",
        "/xyz/openbmc_project/sensors/altitude",
        "/xyz/openbmc_project/sensors/energy",
        "/xyz/openbmc_project/sensors/liquidflow",
        "/xyz/openbmc_project/sensors/pressure",
        "/xyz/openbmc_project/sensors/utilization",
        "/xyz/openbmc_project/sensors/charge",
        "/xyz/openbmc_project/sensors/frequency",
        "/xyz/openbmc_project/sensors/rotational_position"});
    } else {
      return  std::to_array<std::string_view>({"/xyz/openbmc_project/sensors/power",
        "/xyz/openbmc_project/sensors/current",
        "/xyz/openbmc_project/sensors/airflow",
        "/xyz/openbmc_project/sensors/humidity",
        "/xyz/openbmc_project/sensors/utilization"});
}
}

constexpr auto sensorPaths = getSensorPaths();

constexpr auto thermalPaths = std::to_array<std::string_view>({
    "/xyz/openbmc_project/sensors/fan_tach",
    "/xyz/openbmc_project/sensors/temperature",
    "/xyz/openbmc_project/sensors/fan_pwm"
});

} // namespace dbus
// clang-format on

constexpr std::string_view powerNodeStr = sensor_utils::chassisSubNodeToString(
    sensor_utils::ChassisSubNode::powerNode);
constexpr std::string_view sensorsNodeStr =
    sensor_utils::chassisSubNodeToString(
        sensor_utils::ChassisSubNode::sensorsNode);
constexpr std::string_view thermalNodeStr =
    sensor_utils::chassisSubNodeToString(
        sensor_utils::ChassisSubNode::thermalNode);

using sensorPair =
    std::pair<std::string_view, std::span<const std::string_view>>;
static constexpr std::array<sensorPair, 3> paths = {
    {{sensors::powerNodeStr, dbus::powerPaths},
     {sensors::sensorsNodeStr, dbus::sensorPaths},
     {sensors::thermalNodeStr, dbus::thermalPaths}}};

} // namespace sensors

/**
 * SensorsAsyncResp
 * Gathers data needed for response processing after async calls are done
 */
/* SensorsAsyncResp
 * 在异步调用完成后，收集响应处理所需的数据。 */
class SensorsAsyncResp
{
  public:
    using DataCompleteCb = std::function<void(
        const boost::beast::http::status status,
        const std::map<std::string, std::string>& uriToDbus)>;

    struct SensorData
    {
        std::string name;
        std::string uri;
        std::string dbusPath;
    };

    SensorsAsyncResp(const std::shared_ptr<bmcweb::AsyncResp>& asyncRespIn,
                     const std::string& chassisIdIn,
                     std::span<const std::string_view> typesIn,
                     std::string_view subNode) :
        asyncResp(asyncRespIn), chassisId(chassisIdIn), types(typesIn),
        chassisSubNode(subNode), efficientExpand(false)
    {}

    // Store extra data about sensor mapping and return it in callback
    // 存储有关传感器映射的额外数据，并在回调中返回。
    SensorsAsyncResp(const std::shared_ptr<bmcweb::AsyncResp>& asyncRespIn,
                     const std::string& chassisIdIn,
                     std::span<const std::string_view> typesIn,
                     std::string_view subNode,
                     DataCompleteCb&& creationComplete) :
        asyncResp(asyncRespIn), chassisId(chassisIdIn), types(typesIn),
        chassisSubNode(subNode), efficientExpand(false),
        metadata{std::vector<SensorData>()},
        dataComplete{std::move(creationComplete)}
    {}

    // sensor collections expand
    // 传感器集合展开（expand）。
    SensorsAsyncResp(const std::shared_ptr<bmcweb::AsyncResp>& asyncRespIn,
                     const std::string& chassisIdIn,
                     std::span<const std::string_view> typesIn,
                     const std::string_view& subNode, bool efficientExpandIn) :
        asyncResp(asyncRespIn), chassisId(chassisIdIn), types(typesIn),
        chassisSubNode(subNode), efficientExpand(efficientExpandIn)
    {}

    ~SensorsAsyncResp()
    {
        if (asyncResp->res.result() ==
            boost::beast::http::status::internal_server_error)
        {
            // Reset the json object to clear out any data that made it in
            // before the error happened todo(ed) handle error condition with
            // proper code
            // 重置 json 对象，清除错误发生前已写入的数据。
            // todo(ed)：用恰当的代码处理错误情况。
            asyncResp->res.jsonValue = nlohmann::json::object();
        }

        if (dataComplete && metadata)
        {
            std::map<std::string, std::string> map;
            if (asyncResp->res.result() == boost::beast::http::status::ok)
            {
                for (auto& sensor : *metadata)
                {
                    map.emplace(sensor.uri, sensor.dbusPath);
                }
            }
            dataComplete(asyncResp->res.result(), map);
        }
    }

    SensorsAsyncResp(const SensorsAsyncResp&) = delete;
    SensorsAsyncResp(SensorsAsyncResp&&) = delete;
    SensorsAsyncResp& operator=(const SensorsAsyncResp&) = delete;
    SensorsAsyncResp& operator=(SensorsAsyncResp&&) = delete;

    void addMetadata(const nlohmann::json& sensorObject,
                     const std::string& dbusPath)
    {
        if (!metadata)
        {
            return;
        }
        const auto nameIt = sensorObject.find("Name");
        if (nameIt == sensorObject.end())
        {
            return;
        }
        const auto idIt = sensorObject.find("@odata.id");
        if (idIt == sensorObject.end())
        {
            return;
        }
        const std::string* name = nameIt->get_ptr<const std::string*>();
        if (name == nullptr)
        {
            return;
        }
        const std::string* id = idIt->get_ptr<const std::string*>();
        if (id == nullptr)
        {
            return;
        }
        metadata->emplace_back(SensorData{*name, *id, dbusPath});
    }

    void updateUri(const std::string& name, const std::string& uri)
    {
        if (metadata)
        {
            for (auto& sensor : *metadata)
            {
                if (sensor.name == name)
                {
                    sensor.uri = uri;
                }
            }
        }
    }

    const std::shared_ptr<bmcweb::AsyncResp> asyncResp;
    const std::string chassisId;
    const std::span<const std::string_view> types;
    const std::string chassisSubNode;
    const bool efficientExpand;

  private:
    std::optional<std::vector<SensorData>> metadata;
    DataCompleteCb dataComplete;
};

using InventoryItem = sensor_utils::InventoryItem;

/**
 * @brief Get objects with connection necessary for sensors
 * @param SensorsAsyncResp Pointer to object holding response data
 * @param sensorNames Sensors retrieved from chassis
 * @param callback Callback for processing gathered connections
 */
/* @brief 获取传感器连接所需的对象。
 * @param SensorsAsyncResp 指向保存响应数据的对象的指针。
 * @param sensorNames 从 chassis 检索到的传感器。
 * @param callback 用于处理收集到的连接的回调。 */
template <typename Callback>
void getObjectsWithConnection(
    const std::shared_ptr<SensorsAsyncResp>& sensorsAsyncResp,
    const std::shared_ptr<std::set<std::string>>& sensorNames,
    Callback&& callback)
{
    BMCWEB_LOG_DEBUG("getObjectsWithConnection enter");
    const std::string path = "/xyz/openbmc_project/sensors";
    constexpr std::array<std::string_view, 1> interfaces = {
        "xyz.openbmc_project.Sensor.Value"};

    // Make call to ObjectMapper to find all sensors objects
    // 调用 ObjectMapper 查找所有传感器对象。
    dbus::utility::getSubTree(
        path, 2, interfaces,
        // ast-grep-ignore: long-lambda
        // ast-grep-ignore: long-lambda（忽略指令：ast-grep 跳过过长 lambda 检查）
        [callback = std::forward<Callback>(callback), sensorsAsyncResp,
         sensorNames](const boost::system::error_code& ec,
                      const dbus::utility::MapperGetSubTreeResponse& subtree) {
            // Response handler for parsing objects subtree
            // 用于解析对象子树的响应 handler。
            BMCWEB_LOG_DEBUG("getObjectsWithConnection resp_handler enter");
            if (ec)
            {
                messages::internalError(sensorsAsyncResp->asyncResp->res);
                BMCWEB_LOG_ERROR(
                    "getObjectsWithConnection resp_handler: Dbus error {}", ec);
                return;
            }

            BMCWEB_LOG_DEBUG("Found {} subtrees", subtree.size());

            // Make unique list of connections only for requested sensor types
            // and found in the chassis
            // 仅为请求的传感器类型且在 chassis 中找到的对象，
            // 生成去重后的连接列表。
            std::set<std::string> connections;
            std::set<std::pair<std::string, std::string>> objectsWithConnection;

            BMCWEB_LOG_DEBUG("sensorNames list count: {}", sensorNames->size());
            for (const std::string& tsensor : *sensorNames)
            {
                BMCWEB_LOG_DEBUG("Sensor to find: {}", tsensor);
            }

            for (const std::pair<std::string,
                                 std::vector<std::pair<
                                     std::string, std::vector<std::string>>>>&
                     object : subtree)
            {
                if (sensorNames->contains(object.first))
                {
                    for (const std::pair<std::string, std::vector<std::string>>&
                             objData : object.second)
                    {
                        BMCWEB_LOG_DEBUG("Adding connection: {}",
                                         objData.first);
                        connections.insert(objData.first);
                        objectsWithConnection.insert(
                            std::make_pair(object.first, objData.first));
                    }
                }
            }
            BMCWEB_LOG_DEBUG("Found {} connections", connections.size());
            callback(std::move(connections), std::move(objectsWithConnection));
            BMCWEB_LOG_DEBUG("getObjectsWithConnection resp_handler exit");
        });
    BMCWEB_LOG_DEBUG("getObjectsWithConnection exit");
}

/**
 * @brief Create connections necessary for sensors
 * @param SensorsAsyncResp Pointer to object holding response data
 * @param sensorNames Sensors retrieved from chassis
 * @param callback Callback for processing gathered connections
 */
/* @brief 创建传感器所需的连接。
 * @param SensorsAsyncResp 指向保存响应数据的对象的指针。
 * @param sensorNames 从 chassis 检索到的传感器。
 * @param callback 用于处理收集到的连接的回调。 */
template <typename Callback>
void getConnections(const std::shared_ptr<SensorsAsyncResp>& sensorsAsyncResp,
                    const std::shared_ptr<std::set<std::string>>& sensorNames,
                    Callback&& callback)
{
    auto objectsWithConnectionCb =
        [callback = std::forward<Callback>(callback)](
            const std::set<std::string>& connections,
            const std::set<std::pair<std::string, std::string>>&
            /*objectsWithConnection*/) { callback(connections); };
    getObjectsWithConnection(sensorsAsyncResp, sensorNames,
                             std::move(objectsWithConnectionCb));
}

/**
 * @brief Shrinks the list of sensors for processing
 * @param SensorsAsyncResp  The class holding the Redfish response
 * @param allSensors  A list of all the sensors associated to the
 * chassis element (i.e. baseboard, front panel, etc...)
 * @param activeSensors A list that is a reduction of the incoming
 * allSensors list.  Eliminate Thermal sensors when a Power request is
 * made, and eliminate Power sensors when a Thermal request is made.
 */
/* @brief 缩减待处理的传感器列表。
 * @param SensorsAsyncResp 持有 Redfish 响应的类。
 * @param allSensors 与 chassis 元素（如基板、前面板等）关联的所有传感器列表。
 * @param activeSensors 对传入的 allSensors 列表进行缩减后的列表。
 * 发出 Power 请求时剔除 Thermal 传感器，发出 Thermal 请求时剔除 Power 传感器。 */
inline void reduceSensorList(
    crow::Response& res, std::string_view chassisSubNode,
    std::span<const std::string_view> sensorTypes,
    const std::vector<std::string>* allSensors,
    const std::shared_ptr<std::set<std::string>>& activeSensors)
{
    if ((allSensors == nullptr) || (activeSensors == nullptr))
    {
        messages::resourceNotFound(res, chassisSubNode,
                                   chassisSubNode == sensors::thermalNodeStr
                                       ? "Temperatures"
                                       : "Voltages");

        return;
    }
    if (allSensors->empty())
    {
        // Nothing to do, the activeSensors object is also empty
        // 无需操作，activeSensors 对象同样为空。
        return;
    }

    for (std::string_view type : sensorTypes)
    {
        for (const std::string& sensor : *allSensors)
        {
            if (sensor.starts_with(type))
            {
                activeSensors->emplace(sensor);
            }
        }
    }
}

/*
 *Populates the top level collection for a given subnode.  Populates
 *SensorCollection, Power, or Thermal schemas.
 *
 * */
/* 为给定子节点填充顶层集合。
 * 填充 SensorCollection、Power 或 Thermal schema。 */
inline void populateChassisNode(nlohmann::json& jsonValue,
                                std::string_view chassisSubNode)
{
    if (chassisSubNode == sensors::powerNodeStr)
    {
        jsonValue["@odata.type"] = "#Power.v1_5_2.Power";
    }
    else if (chassisSubNode == sensors::thermalNodeStr)
    {
        jsonValue["@odata.type"] = "#Thermal.v1_4_0.Thermal";
        jsonValue["Fans"] = nlohmann::json::array();
        jsonValue["Temperatures"] = nlohmann::json::array();
    }
    else if (chassisSubNode == sensors::sensorsNodeStr)
    {
        jsonValue["@odata.type"] = "#SensorCollection.SensorCollection";
        jsonValue["Description"] = "Collection of Sensors for this Chassis";
        jsonValue["Members"] = nlohmann::json::array();
        jsonValue["Members@odata.count"] = 0;
    }

    if (chassisSubNode != sensors::sensorsNodeStr)
    {
        jsonValue["Id"] = chassisSubNode;
    }
    jsonValue["Name"] = chassisSubNode;
}

/**
 * @brief Retrieves requested chassis sensors and redundancy data from DBus .
 * @param SensorsAsyncResp   Pointer to object holding response data
 * @param callback  Callback for next step in gathered sensor processing
 */
/* @brief 从 DBus 检索请求的 chassis 传感器和冗余数据。
 * @param SensorsAsyncResp 指向保存响应数据的对象的指针。
 * @param callback 用于传感器处理下一步的回调。 */
template <typename Callback>
void getChassis(const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
                std::string_view chassisId, std::string_view chassisSubNode,
                std::span<const std::string_view> sensorTypes,
                Callback&& callback)
{
    BMCWEB_LOG_DEBUG("getChassis enter");

    // Get the Chassis Collection
    // 获取 Chassis Collection。
    dbus::utility::getSubTreePaths(
        "/xyz/openbmc_project/inventory", 0, chassisInterfaces,
        // ast-grep-ignore: long-lambda
        // ast-grep-ignore: long-lambda（忽略指令：ast-grep 跳过过长 lambda 检查）
        [callback = std::forward<Callback>(callback), asyncResp,
         chassisIdStr{std::string(chassisId)},
         chassisSubNode{std::string(chassisSubNode)},
         sensorTypes](const boost::system::error_code& ec,
                      const dbus::utility::MapperGetSubTreePathsResponse&
                          chassisPaths) mutable {
            BMCWEB_LOG_DEBUG("getChassis respHandler enter");
            if (ec)
            {
                BMCWEB_LOG_ERROR("getChassis respHandler DBUS error: {}", ec);
                messages::internalError(asyncResp->res);
                return;
            }
            const std::string* chassisPath = nullptr;
            for (const std::string& chassis : chassisPaths)
            {
                sdbusplus::object_path path(chassis);
                std::string chassisName = path.filename();
                if (chassisName.empty())
                {
                    BMCWEB_LOG_ERROR("Failed to find '/' in {}", chassis);
                    continue;
                }
                if (chassisName == chassisIdStr)
                {
                    chassisPath = &chassis;
                    break;
                }
            }
            if (chassisPath == nullptr)
            {
                messages::resourceNotFound(asyncResp->res, "Chassis",
                                           chassisIdStr);
                return;
            }
            populateChassisNode(asyncResp->res.jsonValue, chassisSubNode);

            asyncResp->res.jsonValue["@odata.id"] = boost::urls::format(
                "/redfish/v1/Chassis/{}/{}", chassisIdStr, chassisSubNode);

            // Get the list of all sensors for this Chassis element
            // 获取该 Chassis 元素的所有传感器列表。
            std::string sensorPath = *chassisPath + "/all_sensors";
            dbus::utility::getAssociationEndPoints(
                // ast-grep-ignore: long-lambda
                // ast-grep-ignore: long-lambda（忽略指令：ast-grep 跳过过长 lambda 检查）
                sensorPath, [asyncResp, chassisSubNode, sensorTypes,
                             callback = std::forward<Callback>(callback)](
                                const boost::system::error_code& ec2,
                                const dbus::utility::MapperEndPoints&
                                    nodeSensorList) mutable {
                    if (ec2)
                    {
                        if (ec2.value() != EBADR)
                        {
                            messages::internalError(asyncResp->res);
                            return;
                        }
                    }
                    const std::shared_ptr<std::set<std::string>>
                        culledSensorList =
                            std::make_shared<std::set<std::string>>();
                    reduceSensorList(asyncResp->res, chassisSubNode,
                                     sensorTypes, &nodeSensorList,
                                     culledSensorList);
                    BMCWEB_LOG_DEBUG("Finishing with {}",
                                     culledSensorList->size());
                    callback(culledSensorList);
                });
        });
    BMCWEB_LOG_DEBUG("getChassis exit");
}

/**
 * @brief Builds a json sensor representation of a sensor.
 * @param sensorName  The name of the sensor to be built
 * @param sensorType  The type (temperature, fan_tach, etc) of the sensor to
 * build
 * @param chassisSubNode The subnode (thermal, sensor, etc) of the sensor
 * @param interfacesDict  A dictionary of the interfaces and properties of said
 * interfaces to be built from
 * @param sensorJson  The json object to fill
 * @param inventoryItem D-Bus inventory item associated with the sensor.  Will
 * be nullptr if no associated inventory item was found.
 */
/* @brief 为传感器构建 json 表示。
 * @param sensorName 要构建的传感器名称。
 * @param sensorType 要构建的传感器类型（temperature、fan_tach 等）。
 * @param chassisSubNode 传感器的子节点（thermal、sensor 等）。
 * @param interfacesDict 要构建的接口及其属性的字典。
 * @param sensorJson 要填充的 json 对象。
 * @param inventoryItem 与传感器关联的 D-Bus inventory item。
 * 如果未找到关联的 inventory item，则为 nullptr。 */
inline void objectInterfacesToJson(
    const std::string& sensorName, const std::string& sensorType,
    const sensor_utils::ChassisSubNode chassisSubNode,
    const dbus::utility::DBusInterfacesMap& interfacesDict,
    nlohmann::json& sensorJson, InventoryItem* inventoryItem)
{
    for (const auto& [interface, valuesDict] : interfacesDict)
    {
        sensor_utils::objectPropertiesToJson(
            sensorName, sensorType, chassisSubNode, valuesDict, sensorJson,
            inventoryItem);
    }
    BMCWEB_LOG_DEBUG("Added sensor {}", sensorName);
}

inline void populateFanRedundancy(
    const std::shared_ptr<SensorsAsyncResp>& sensorsAsyncResp)
{
    constexpr std::array<std::string_view, 1> interfaces = {
        "xyz.openbmc_project.Control.FanRedundancy"};
    dbus::utility::getSubTree(
        "/xyz/openbmc_project/control", 2, interfaces,
        // ast-grep-ignore: long-lambda
        // ast-grep-ignore: long-lambda（忽略指令：ast-grep 跳过过长 lambda 检查）
        [sensorsAsyncResp](
            const boost::system::error_code& ec,
            const dbus::utility::MapperGetSubTreeResponse& resp) {
            if (ec)
            {
                return; // don't have to have this interface
            }
            for (const std::pair<std::string, dbus::utility::MapperServiceMap>&
                     pathPair : resp)
            {
                const std::string& path = pathPair.first;
                const dbus::utility::MapperServiceMap& objDict =
                    pathPair.second;
                if (objDict.empty())
                {
                    continue; // this should be impossible
                }

                const std::string& owner = objDict.begin()->first;
                dbus::utility::getAssociationEndPoints(
                    path + "/chassis",
                    // ast-grep-ignore: long-lambda
                    // ast-grep-ignore: long-lambda（忽略指令：ast-grep 跳过过长 lambda 检查）
                    [path, owner, sensorsAsyncResp](
                        const boost::system::error_code& ec2,
                        const dbus::utility::MapperEndPoints& endpoints) {
                        if (ec2)
                        {
                            return; // if they don't have an association we
                                    // can't tell what chassis is
                                    // 无法判断是哪个 chassis。
                        }
                        auto found = std::ranges::find_if(
                            endpoints,
                            [sensorsAsyncResp](const std::string& entry) {
                                return entry.contains(
                                    sensorsAsyncResp->chassisId);
                            });

                        if (found == endpoints.end())
                        {
                            return;
                        }
                        dbus::utility::getAllProperties(
                            *crow::connections::systemBus, owner, path,
                            "xyz.openbmc_project.Control.FanRedundancy",
                            // ast-grep-ignore: long-lambda
                            // ast-grep-ignore: long-lambda（忽略指令：ast-grep 跳过过长 lambda 检查）
                            [path, sensorsAsyncResp](
                                const boost::system::error_code& ec3,
                                const dbus::utility::DBusPropertiesMap& ret) {
                                if (ec3)
                                {
                                    return; // don't have to have this
                                            // interface
                                            // 接口（不是必需的）。
                                }

                                const uint8_t* allowedFailures = nullptr;
                                const std::vector<std::string>* collection =
                                    nullptr;
                                const std::string* status = nullptr;

                                const bool success =
                                    sdbusplus::unpackPropertiesNoThrow(
                                        dbus_utils::UnpackErrorPrinter(), ret,
                                        "AllowedFailures", allowedFailures,
                                        "Collection", collection, "Status",
                                        status);

                                if (!success)
                                {
                                    messages::internalError(
                                        sensorsAsyncResp->asyncResp->res);
                                    return;
                                }

                                if (allowedFailures == nullptr ||
                                    collection == nullptr || status == nullptr)
                                {
                                    BMCWEB_LOG_ERROR(
                                        "Invalid redundancy interface");
                                    messages::internalError(
                                        sensorsAsyncResp->asyncResp->res);
                                    return;
                                }

                                sdbusplus::object_path objectPath(path);
                                std::string name = objectPath.filename();
                                if (name.empty())
                                {
                                    // this should be impossible
                                    // 这应该不可能发生。
                                    messages::internalError(
                                        sensorsAsyncResp->asyncResp->res);
                                    return;
                                }
                                std::ranges::replace(name, '_', ' ');

                                resource::Health health =
                                    resource::Health::Critical;

                                if (status->ends_with("Full"))
                                {
                                    health = resource::Health::OK;
                                }
                                else if (status->ends_with("Degraded"))
                                {
                                    health = resource::Health::Warning;
                                }
                                nlohmann::json::array_t redfishCollection;
                                const auto& fanRedfish =
                                    sensorsAsyncResp->asyncResp->res
                                        .jsonValue["Fans"];
                                for (const std::string& item : *collection)
                                {
                                    sdbusplus::object_path itemPath(item);
                                    std::string itemName = itemPath.filename();
                                    if (itemName.empty())
                                    {
                                        continue;
                                    }
                                    /*
                                    todo(ed): merge patch that fixes the names
                                    std::replace(itemName.begin(),
                                                 itemName.end(), '_', ' ');*/
                                    /* todo(ed)：合并修复名称的 patch。
                                     * std::replace(itemName.begin(),
                                     * itemName.end(), '_', ' '); */
                                    auto schemaItem = std::ranges::find_if(
                                        fanRedfish,
                                        [itemName](const nlohmann::json& fan) {
                                            return fan["Name"] == itemName;
                                        });
                                    if (schemaItem != fanRedfish.end())
                                    {
                                        nlohmann::json::object_t collectionId;
                                        collectionId["@odata.id"] =
                                            (*schemaItem)["@odata.id"];
                                        redfishCollection.emplace_back(
                                            std::move(collectionId));
                                    }
                                    else
                                    {
                                        BMCWEB_LOG_ERROR(
                                            "failed to find fan in schema");
                                        messages::internalError(
                                            sensorsAsyncResp->asyncResp->res);
                                        return;
                                    }
                                }

                                size_t minNumNeeded =
                                    collection->empty()
                                        ? 0
                                        : collection->size() - *allowedFailures;
                                nlohmann::json& jResp =
                                    sensorsAsyncResp->asyncResp->res
                                        .jsonValue["Redundancy"];

                                nlohmann::json::object_t redundancy;
                                boost::urls::url url = boost::urls::format(
                                    "/redfish/v1/Chassis/{}/{}",
                                    sensorsAsyncResp->chassisId,
                                    sensorsAsyncResp->chassisSubNode);
                                url.set_fragment(
                                    ("/Redundancy"_json_pointer / jResp.size())
                                        .to_string());
                                redundancy["@odata.id"] = std::move(url);
                                redundancy["@odata.type"] =
                                    "#Redundancy.v1_3_2.Redundancy";
                                redundancy["MinNumNeeded"] = minNumNeeded;
                                redundancy["Mode"] =
                                    redundancy::RedundancyType::NPlusM;
                                redundancy["Name"] = name;
                                redundancy["RedundancySet"] = redfishCollection;
                                redundancy["Status"]["Health"] = health;
                                redundancy["Status"]["State"] =
                                    resource::State::Enabled;

                                jResp.emplace_back(std::move(redundancy));
                            });
                    });
            }
        });
}

inline void sortJSONResponse(
    const std::shared_ptr<SensorsAsyncResp>& sensorsAsyncResp)
{
    nlohmann::json& response = sensorsAsyncResp->asyncResp->res.jsonValue;
    std::array<std::string, 2> sensorHeaders{"Temperatures", "Fans"};
    if (sensorsAsyncResp->chassisSubNode == sensors::powerNodeStr)
    {
        sensorHeaders = {"Voltages", "PowerSupplies"};
    }
    for (const std::string& sensorGroup : sensorHeaders)
    {
        nlohmann::json::iterator entry = response.find(sensorGroup);
        if (entry == response.end())
        {
            continue;
        }
        nlohmann::json::array_t* arr =
            entry->get_ptr<nlohmann::json::array_t*>();
        if (arr == nullptr)
        {
            continue;
        }
        json_util::sortJsonArrayByKey(*arr, "Name");

        // add the index counts to the end of each entry
        // 在每个条目末尾添加索引计数。
        size_t count = 0;
        for (nlohmann::json& sensorJson : *entry)
        {
            nlohmann::json::iterator odata = sensorJson.find("@odata.id");
            if (odata == sensorJson.end())
            {
                continue;
            }
            const auto nameIt = sensorJson.find("Name");
            if (nameIt == sensorJson.end())
            {
                continue;
            }
            std::string* value = odata->get_ptr<std::string*>();
            if (value == nullptr)
            {
                continue;
            }
            const std::string* name = nameIt->get_ptr<const std::string*>();
            if (name == nullptr)
            {
                continue;
            }
            *value += "/" + std::to_string(count);
            sensorJson["MemberId"] = std::to_string(count);
            count++;
            sensorsAsyncResp->updateUri(*name, *value);
        }
    }
}

/**
 * @brief Finds the inventory item with the specified object path.
 * @param inventoryItems D-Bus inventory items associated with sensors.
 * @param invItemObjPath D-Bus object path of inventory item.
 * @return Inventory item within vector, or nullptr if no match found.
 */
/* @brief 查找具有指定对象路径的 inventory item。
 * @param inventoryItems 与传感器关联的 D-Bus inventory items。
 * @param invItemObjPath inventory item 的 D-Bus 对象路径。
 * @return vector 中的 inventory item；若未找到匹配项则返回 nullptr。 */
inline InventoryItem* findInventoryItem(
    const std::shared_ptr<std::vector<InventoryItem>>& inventoryItems,
    const std::string& invItemObjPath)
{
    for (InventoryItem& inventoryItem : *inventoryItems)
    {
        if (inventoryItem.objectPath == invItemObjPath)
        {
            return &inventoryItem;
        }
    }
    return nullptr;
}

/**
 * @brief Finds the inventory item associated with the specified sensor.
 * @param inventoryItems D-Bus inventory items associated with sensors.
 * @param sensorObjPath D-Bus object path of sensor.
 * @return Inventory item within vector, or nullptr if no match found.
 */
/* @brief 查找与指定传感器关联的 inventory item。
 * @param inventoryItems 与传感器关联的 D-Bus inventory items。
 * @param sensorObjPath 传感器的 D-Bus 对象路径。
 * @return vector 中的 inventory item；若未找到匹配项则返回 nullptr。 */
inline InventoryItem* findInventoryItemForSensor(
    const std::shared_ptr<std::vector<InventoryItem>>& inventoryItems,
    const std::string& sensorObjPath)
{
    for (InventoryItem& inventoryItem : *inventoryItems)
    {
        if (inventoryItem.sensors.contains(sensorObjPath))
        {
            return &inventoryItem;
        }
    }
    return nullptr;
}

/**
 * @brief Adds inventory item and associated sensor to specified vector.
 *
 * Adds a new InventoryItem to the vector if necessary.  Searches for an
 * existing InventoryItem with the specified object path.  If not found, one is
 * added to the vector.
 *
 * Next, the specified sensor is added to the set of sensors associated with the
 * InventoryItem.
 *
 * @param inventoryItems D-Bus inventory items associated with sensors.
 * @param invItemObjPath D-Bus object path of inventory item.
 * @param sensorObjPath D-Bus object path of sensor
 */
/* @brief 将 inventory item 和关联的传感器添加到指定 vector。
 * 如有必要，向 vector 添加新的 InventoryItem。
 * 搜索具有指定对象路径的现有 InventoryItem。
 * 若未找到，则将其添加到 vector 中。
 * 接下来，将指定传感器添加到与该 InventoryItem 关联的传感器集合中。
 * @param inventoryItems 与传感器关联的 D-Bus inventory items。
 * @param invItemObjPath inventory item 的 D-Bus 对象路径。
 * @param sensorObjPath 传感器的 D-Bus 对象路径。 */
inline void addInventoryItem(
    const std::shared_ptr<std::vector<InventoryItem>>& inventoryItems,
    const std::string& invItemObjPath, const std::string& sensorObjPath)
{
    // Look for inventory item in vector
    // 在 vector 中查找 inventory item。
    InventoryItem* inventoryItem =
        findInventoryItem(inventoryItems, invItemObjPath);

    // If inventory item doesn't exist in vector, add it
    // 如果 vector 中不存在该 inventory item，则添加它。
    if (inventoryItem == nullptr)
    {
        inventoryItems->emplace_back(invItemObjPath);
        inventoryItem = &(inventoryItems->back());
    }

    // Add sensor to set of sensors associated with inventory item
    // 将传感器添加到与 inventory item 关联的传感器集合中。
    inventoryItem->sensors.emplace(sensorObjPath);
}

/**
 * @brief Stores D-Bus data in the specified inventory item.
 *
 * Finds D-Bus data in the specified map of interfaces.  Stores the data in the
 * specified InventoryItem.
 *
 * This data is later used to provide sensor property values in the JSON
 * response.
 *
 * @param inventoryItem Inventory item where data will be stored.
 * @param interfacesDict Map containing D-Bus interfaces and their properties
 * for the specified inventory item.
 */
/* @brief 将 D-Bus 数据存储到指定的 inventory item 中。
 * 在指定的接口映射中查找 D-Bus 数据，并将其存储到指定的 InventoryItem 中。
 * 这些数据稍后用于在 JSON 响应中提供传感器属性值。
 * @param inventoryItem 用于存储数据的 inventory item。
 * @param interfacesDict 包含指定 inventory item 的 D-Bus 接口及其属性的映射。 */
inline void storeInventoryItemData(
    InventoryItem& inventoryItem,
    const dbus::utility::DBusInterfacesMap& interfacesDict)
{
    // Get properties from Inventory.Item interface
    // 从 Inventory.Item 接口获取属性。

    for (const auto& [interface, values] : interfacesDict)
    {
        if (interface == "xyz.openbmc_project.Inventory.Item")
        {
            for (const auto& [name, dbusValue] : values)
            {
                if (name == "Present")
                {
                    const bool* value = std::get_if<bool>(&dbusValue);
                    if (value != nullptr)
                    {
                        inventoryItem.isPresent = *value;
                    }
                }
            }
        }
        // Check if Inventory.Item.PowerSupply interface is present
        // 检查是否存在 Inventory.Item.PowerSupply 接口。

        if (interface == "xyz.openbmc_project.Inventory.Item.PowerSupply")
        {
            inventoryItem.isPowerSupply = true;
        }

        // Get properties from Inventory.Decorator.Asset interface
        // 从 Inventory.Decorator.Asset 接口获取属性。
        if (interface == "xyz.openbmc_project.Inventory.Decorator.Asset")
        {
            for (const auto& [name, dbusValue] : values)
            {
                if (name == "Manufacturer")
                {
                    const std::string* value =
                        std::get_if<std::string>(&dbusValue);
                    if (value != nullptr)
                    {
                        inventoryItem.manufacturer = *value;
                    }
                }
                if (name == "Model")
                {
                    const std::string* value =
                        std::get_if<std::string>(&dbusValue);
                    if (value != nullptr)
                    {
                        inventoryItem.model = *value;
                    }
                }
                if (name == "SerialNumber")
                {
                    const std::string* value =
                        std::get_if<std::string>(&dbusValue);
                    if (value != nullptr)
                    {
                        inventoryItem.serialNumber = *value;
                    }
                }
                if (name == "PartNumber")
                {
                    const std::string* value =
                        std::get_if<std::string>(&dbusValue);
                    if (value != nullptr)
                    {
                        inventoryItem.partNumber = *value;
                    }
                }
            }
        }

        if (interface ==
            "xyz.openbmc_project.State.Decorator.OperationalStatus")
        {
            for (const auto& [name, dbusValue] : values)
            {
                if (name == "Functional")
                {
                    const bool* value = std::get_if<bool>(&dbusValue);
                    if (value != nullptr)
                    {
                        inventoryItem.isFunctional = *value;
                    }
                }
            }
        }
    }
}

/**
 * @brief Gets D-Bus data for inventory items associated with sensors.
 *
 * Uses the specified connections (services) to obtain D-Bus data for inventory
 * items associated with sensors.  Stores the resulting data in the
 * inventoryItems vector.
 *
 * This data is later used to provide sensor property values in the JSON
 * response.
 *
 * Finds the inventory item data asynchronously.  Invokes callback when data has
 * been obtained.
 *
 * The callback must have the following signature:
 *   @code
 *   callback(void)
 *   @endcode
 *
 * This function is called recursively, obtaining data asynchronously from one
 * connection in each call.  This ensures the callback is not invoked until the
 * last asynchronous function has completed.
 *
 * @param sensorsAsyncResp Pointer to object holding response data.
 * @param inventoryItems D-Bus inventory items associated with sensors.
 * @param invConnections Connections that provide data for the inventory items.
 * implements ObjectManager.
 * @param callback Callback to invoke when inventory data has been obtained.
 * @param invConnectionsIndex Current index in invConnections.  Only specified
 * in recursive calls to this function.
 */
/* @brief 获取与传感器关联的 inventory items 的 D-Bus 数据。
 * 使用指定的连接（服务）获取与传感器关联的 inventory items 的 D-Bus 数据。
 * 将结果数据存入 inventoryItems vector。
 * 这些数据稍后用于在 JSON 响应中提供传感器属性值。
 * 异步查找 inventory item 数据。获取数据后调用回调。
 * 回调必须具有以下签名：
 *   @code
 *   callback(void)
 *   @endcode
 * 该函数以递归方式调用，每次调用从一个连接异步获取数据。
 * 这确保只有在最后一个异步函数完成后才调用回调。
 * @param sensorsAsyncResp 指向保存响应数据的对象的指针。
 * @param inventoryItems 与传感器关联的 D-Bus inventory items。
 * @param invConnections 为 inventory items 提供数据的连接。
 * 实现了 ObjectManager。
 * @param callback 获取 inventory 数据后调用的回调。
 * @param invConnectionsIndex 当前在 invConnections 中的索引。
 * 仅在递归调用该函数时指定。 */
template <typename Callback>
void getInventoryItemsData(
    const std::shared_ptr<SensorsAsyncResp>& sensorsAsyncResp,
    const std::shared_ptr<std::vector<InventoryItem>>& inventoryItems,
    const std::shared_ptr<std::set<std::string>>& invConnections,
    Callback&& callback, size_t invConnectionsIndex = 0)
{
    BMCWEB_LOG_DEBUG("getInventoryItemsData enter");

    // If no more connections left, call callback
    // 如果没有剩余连接，则调用回调。
    if (invConnectionsIndex >= invConnections->size())
    {
        callback();
        BMCWEB_LOG_DEBUG("getInventoryItemsData exit");
        return;
    }

    // Get inventory item data from current connection
    // 从当前连接获取 inventory item 数据。
    auto it = invConnections->begin();
    std::advance(it, invConnectionsIndex);
    if (it != invConnections->end())
    {
        const std::string& invConnection = *it;

        // Get all object paths and their interfaces for current connection
        // 获取当前连接的所有对象路径及其接口。
        sdbusplus::object_path path("/xyz/openbmc_project/inventory");
        dbus::utility::getManagedObjects(
            invConnection, path,
            // ast-grep-ignore: long-lambda
            // ast-grep-ignore: long-lambda（忽略指令：ast-grep 跳过过长 lambda 检查）
            [sensorsAsyncResp, inventoryItems, invConnections,
             callback = std::forward<Callback>(callback), invConnectionsIndex](
                const boost::system::error_code& ec,
                const dbus::utility::ManagedObjectType& resp) mutable {
                BMCWEB_LOG_DEBUG("getInventoryItemsData respHandler enter");
                if (ec)
                {
                    BMCWEB_LOG_ERROR(
                        "getInventoryItemsData respHandler DBus error {}", ec);
                    messages::internalError(sensorsAsyncResp->asyncResp->res);
                    return;
                }

                // Loop through returned object paths
                // 遍历返回的对象路径。
                for (const auto& objDictEntry : resp)
                {
                    const std::string& objPath =
                        static_cast<const std::string&>(objDictEntry.first);

                    // If this object path is one of the specified inventory
                    // items
                    // 如果该对象路径是指定 inventory items 之一。
                    InventoryItem* inventoryItem =
                        findInventoryItem(inventoryItems, objPath);
                    if (inventoryItem != nullptr)
                    {
                        // Store inventory data in InventoryItem
                        // 将 inventory 数据存储到 InventoryItem 中。
                        storeInventoryItemData(*inventoryItem,
                                               objDictEntry.second);
                    }
                }

                // Recurse to get inventory item data from next connection
                // 递归从下一个连接获取 inventory item 数据。
                getInventoryItemsData(sensorsAsyncResp, inventoryItems,
                                      invConnections, std::move(callback),
                                      invConnectionsIndex + 1);

                BMCWEB_LOG_DEBUG("getInventoryItemsData respHandler exit");
            });
    }

    BMCWEB_LOG_DEBUG("getInventoryItemsData exit");
}

/**
 * @brief Gets connections that provide D-Bus data for inventory items.
 *
 * Gets the D-Bus connections (services) that provide data for the inventory
 * items that are associated with sensors.
 *
 * Finds the connections asynchronously.  Invokes callback when information has
 * been obtained.
 *
 * The callback must have the following signature:
 *   @code
 *   callback(std::shared_ptr<std::set<std::string>> invConnections)
 *   @endcode
 *
 * @param sensorsAsyncResp Pointer to object holding response data.
 * @param inventoryItems D-Bus inventory items associated with sensors.
 * @param callback Callback to invoke when connections have been obtained.
 */
/* @brief 获取为 inventory items 提供 D-Bus 数据的连接。
 * 获取为与传感器关联的 inventory items 提供数据的 D-Bus 连接（服务）。
 * 异步查找连接。获取信息后调用回调。
 * 回调必须具有以下签名：
 *   @code
 *   callback(std::shared_ptr<std::set<std::string>> invConnections)
 *   @endcode
 * @param sensorsAsyncResp 指向保存响应数据的对象的指针。
 * @param inventoryItems 与传感器关联的 D-Bus inventory items。
 * @param callback 获取连接后调用的回调。 */
template <typename Callback>
void getInventoryItemsConnections(
    const std::shared_ptr<SensorsAsyncResp>& sensorsAsyncResp,
    const std::shared_ptr<std::vector<InventoryItem>>& inventoryItems,
    Callback&& callback)
{
    BMCWEB_LOG_DEBUG("getInventoryItemsConnections enter");

    const std::string path = "/xyz/openbmc_project/inventory";
    constexpr std::array<std::string_view, 4> interfaces = {
        "xyz.openbmc_project.Inventory.Item",
        "xyz.openbmc_project.Inventory.Item.PowerSupply",
        "xyz.openbmc_project.Inventory.Decorator.Asset",
        "xyz.openbmc_project.State.Decorator.OperationalStatus"};

    // Make call to ObjectMapper to find all inventory items
    // 调用 ObjectMapper 查找所有 inventory items。
    dbus::utility::getSubTree(
        path, 0, interfaces,
        // ast-grep-ignore: long-lambda
        // ast-grep-ignore: long-lambda（忽略指令：ast-grep 跳过过长 lambda 检查）
        [callback = std::forward<Callback>(callback), sensorsAsyncResp,
         inventoryItems](
            const boost::system::error_code& ec,
            const dbus::utility::MapperGetSubTreeResponse& subtree) mutable {
            // Response handler for parsing output from GetSubTree
            // 用于解析 GetSubTree 输出的响应 handler。
            BMCWEB_LOG_DEBUG("getInventoryItemsConnections respHandler enter");
            if (ec)
            {
                messages::internalError(sensorsAsyncResp->asyncResp->res);
                BMCWEB_LOG_ERROR(
                    "getInventoryItemsConnections respHandler DBus error {}",
                    ec);
                return;
            }

            // Make unique list of connections for desired inventory items
            // 为目标 inventory items 生成去重后的连接列表。
            std::shared_ptr<std::set<std::string>> invConnections =
                std::make_shared<std::set<std::string>>();

            // Loop through objects from GetSubTree
            // 遍历 GetSubTree 返回的对象。
            for (const std::pair<std::string,
                                 std::vector<std::pair<
                                     std::string, std::vector<std::string>>>>&
                     object : subtree)
            {
                // Check if object path is one of the specified inventory items
                // 检查对象路径是否为指定 inventory items 之一。
                const std::string& objPath = object.first;
                if (findInventoryItem(inventoryItems, objPath) != nullptr)
                {
                    // Store all connections to inventory item
                    // 存储到该 inventory item 的所有连接。
                    for (const std::pair<std::string, std::vector<std::string>>&
                             objData : object.second)
                    {
                        const std::string& invConnection = objData.first;
                        invConnections->insert(invConnection);
                    }
                }
            }

            callback(invConnections);
            BMCWEB_LOG_DEBUG("getInventoryItemsConnections respHandler exit");
        });
    BMCWEB_LOG_DEBUG("getInventoryItemsConnections exit");
}

/**
 * @brief Gets associations from sensors to inventory items.
 *
 * Looks for ObjectMapper associations from the specified sensors to related
 * inventory items.
 *
 * Finds the inventory items asynchronously.  Invokes callback when information
 * has been obtained.
 *
 * The callback must have the following signature:
 *   @code
 *   callback(std::shared_ptr<std::vector<InventoryItem>> inventoryItems)
 *   @endcode
 *
 * @param sensorsAsyncResp Pointer to object holding response data.
 * @param sensorNames All sensors within the current chassis.
 * implements ObjectManager.
 * @param callback Callback to invoke when inventory items have been obtained.
 */
/* @brief 获取从传感器到 inventory items 的关联。
 * 查找从指定传感器到相关 inventory items 的 ObjectMapper 关联。
 * 异步查找 inventory items。获取信息后调用回调。
 * 回调必须具有以下签名：
 *   @code
 *   callback(std::shared_ptr<std::vector<InventoryItem>> inventoryItems)
 *   @endcode
 * @param sensorsAsyncResp 指向保存响应数据的对象的指针。
 * @param sensorNames 当前 chassis 中的所有传感器。
 * 实现了 ObjectManager。
 * @param callback 获取 inventory items 后调用的回调。 */
template <typename Callback>
void getInventoryItemAssociations(
    const std::shared_ptr<SensorsAsyncResp>& sensorsAsyncResp,
    const std::shared_ptr<std::set<std::string>>& sensorNames,
    Callback&& callback)
{
    BMCWEB_LOG_DEBUG("getInventoryItemAssociations enter");

    // Call GetManagedObjects on the ObjectMapper to get all associations
    // 对 ObjectMapper 调用 GetManagedObjects 以获取所有关联。
    sdbusplus::object_path path("/");
    dbus::utility::getManagedObjects(
        "xyz.openbmc_project.ObjectMapper", path,
        // ast-grep-ignore: long-lambda
        // ast-grep-ignore: long-lambda（忽略指令：ast-grep 跳过过长 lambda 检查）
        [callback = std::forward<Callback>(callback), sensorsAsyncResp,
         sensorNames](const boost::system::error_code& ec,
                      const dbus::utility::ManagedObjectType& resp) mutable {
            BMCWEB_LOG_DEBUG("getInventoryItemAssociations respHandler enter");
            if (ec)
            {
                BMCWEB_LOG_ERROR(
                    "getInventoryItemAssociations respHandler DBus error {}",
                    ec);
                messages::internalError(sensorsAsyncResp->asyncResp->res);
                return;
            }

            // Create vector to hold list of inventory items
            // 创建 vector 保存 inventory items 列表。
            std::shared_ptr<std::vector<InventoryItem>> inventoryItems =
                std::make_shared<std::vector<InventoryItem>>();

            // Loop through returned object paths
            // 遍历返回的对象路径。
            std::string sensorAssocPath;
            sensorAssocPath.reserve(128); // avoid memory allocations
            for (const auto& objDictEntry : resp)
            {
                const std::string& objPath =
                    static_cast<const std::string&>(objDictEntry.first);

                // If path is inventory association for one of the specified
                // sensors
                // 如果路径是指定传感器之一的 inventory 关联。
                for (const std::string& sensorName : *sensorNames)
                {
                    sensorAssocPath = sensorName;
                    sensorAssocPath += "/inventory";
                    if (objPath == sensorAssocPath)
                    {
                        // Get Association interface for object path
                        // 获取对象路径的 Association 接口。
                        for (const auto& [interface, values] :
                             objDictEntry.second)
                        {
                            if (interface == "xyz.openbmc_project.Association")
                            {
                                for (const auto& [valueName, value] : values)
                                {
                                    if (valueName == "endpoints")
                                    {
                                        const std::vector<std::string>*
                                            endpoints = std::get_if<
                                                std::vector<std::string>>(
                                                &value);
                                        if ((endpoints != nullptr) &&
                                            !endpoints->empty())
                                        {
                                            // Add inventory item to vector
                                            // 将 inventory item 添加到 vector。
                                            const std::string& invItemPath =
                                                endpoints->front();
                                            addInventoryItem(inventoryItems,
                                                             invItemPath,
                                                             sensorName);
                                        }
                                    }
                                }
                            }
                        }
                        break;
                    }
                }
            }

            callback(inventoryItems);
            BMCWEB_LOG_DEBUG("getInventoryItemAssociations respHandler exit");
        });

    BMCWEB_LOG_DEBUG("getInventoryItemAssociations exit");
}

/**
 * @brief Gets D-Bus data for Power Supply Attributes such as EfficiencyPercent
 *
 * Uses the specified connections (services) (currently assumes just one) to
 * obtain D-Bus data for Power Supply Attributes. Stores the resulting data in
 * the inventoryItems vector. Only stores data in Power Supply inventoryItems.
 *
 * This data is later used to provide sensor property values in the JSON
 * response.
 *
 * Finds the Power Supply Attributes data asynchronously.  Invokes callback
 * when data has been obtained.
 *
 * The callback must have the following signature:
 *   @code
 *   callback(std::shared_ptr<std::vector<InventoryItem>> inventoryItems)
 *   @endcode
 *
 * @param sensorsAsyncResp Pointer to object holding response data.
 * @param inventoryItems D-Bus inventory items associated with sensors.
 * @param psAttributesConnections Connections that provide data for the Power
 *        Supply Attributes
 * @param callback Callback to invoke when data has been obtained.
 */
/* @brief 获取 Power Supply 属性（如 EfficiencyPercent）的 D-Bus 数据。
 * 使用指定的连接（服务）（目前假设只有一个）获取 Power Supply 属性的 D-Bus 数据。
 * 将结果数据存入 inventoryItems vector。
 * 只存储到 Power Supply 类型的 inventoryItems 中。
 * 这些数据稍后用于在 JSON 响应中提供传感器属性值。
 * 异步查找 Power Supply 属性数据。获取数据后调用回调。
 * 回调必须具有以下签名：
 *   @code
 *   callback(std::shared_ptr<std::vector<InventoryItem>> inventoryItems)
 *   @endcode
 * @param sensorsAsyncResp 指向保存响应数据的对象的指针。
 * @param inventoryItems 与传感器关联的 D-Bus inventory items。
 * @param psAttributesConnections 为 Power Supply 属性提供数据的连接。
 * @param callback 获取数据后调用的回调。 */
template <typename Callback>
void getPowerSupplyAttributesData(
    const std::shared_ptr<SensorsAsyncResp>& sensorsAsyncResp,
    const std::shared_ptr<std::vector<InventoryItem>>& inventoryItems,
    const std::map<std::string, std::string>& psAttributesConnections,
    Callback&& callback)
{
    BMCWEB_LOG_DEBUG("getPowerSupplyAttributesData enter");

    if (psAttributesConnections.empty())
    {
        BMCWEB_LOG_DEBUG("Can't find PowerSupplyAttributes, no connections!");
        callback(inventoryItems);
        return;
    }

    // Assuming just one connection (service) for now
    // 目前假设只有一个连接（服务）。
    auto it = psAttributesConnections.begin();

    const std::string& psAttributesPath = (*it).first;
    const std::string& psAttributesConnection = (*it).second;

    // Response handler for Get DeratingFactor property
    // ast-grep-ignore: long-lambda
    // Get DeratingFactor 属性的响应 handler。
    // ast-grep-ignore: long-lambda（忽略指令：ast-grep 跳过过长 lambda 检查）
    auto respHandler = [sensorsAsyncResp, inventoryItems,
                        callback = std::forward<Callback>(callback)](
                           const boost::system::error_code& ec,
                           uint32_t value) mutable {
        BMCWEB_LOG_DEBUG("getPowerSupplyAttributesData respHandler enter");
        if (ec)
        {
            BMCWEB_LOG_ERROR(
                "getPowerSupplyAttributesData respHandler DBus error {}", ec);
            messages::internalError(sensorsAsyncResp->asyncResp->res);
            return;
        }

        BMCWEB_LOG_DEBUG("PS EfficiencyPercent value: {}", value);
        // Store value in Power Supply Inventory Items
        // 将值存储到 Power Supply Inventory Items 中。
        for (InventoryItem& inventoryItem : *inventoryItems)
        {
            if (inventoryItem.isPowerSupply)
            {
                inventoryItem.powerSupplyEfficiencyPercent =
                    static_cast<int>(value);
            }
        }

        BMCWEB_LOG_DEBUG("getPowerSupplyAttributesData respHandler exit");
        callback(inventoryItems);
    };

    // Get the DeratingFactor property for the PowerSupplyAttributes
    // Currently only property on the interface/only one we care about
    // 获取 PowerSupplyAttributes 的 DeratingFactor 属性。
    // 目前这是该接口上唯一的属性/我们唯一关心的属性。
    dbus::utility::getProperty<uint32_t>(
        psAttributesConnection, psAttributesPath,
        "xyz.openbmc_project.Control.PowerSupplyAttributes", "DeratingFactor",
        std::move(respHandler));

    BMCWEB_LOG_DEBUG("getPowerSupplyAttributesData exit");
}

/**
 * @brief Gets the Power Supply Attributes such as EfficiencyPercent
 *
 * Gets the D-Bus connection (service) that provides Power Supply Attributes
 * data. Then gets the Power Supply Attributes data from the connection
 * (currently just assumes 1 connection) and stores the data in the inventory
 * item.
 *
 * This data is later used to provide sensor property values in the JSON
 * response. DeratingFactor on D-Bus is mapped to EfficiencyPercent on Redfish.
 *
 * Finds the Power Supply Attributes data asynchronously. Invokes callback
 * when information has been obtained.
 *
 * The callback must have the following signature:
 *   @code
 *   callback(std::shared_ptr<std::vector<InventoryItem>> inventoryItems)
 *   @endcode
 *
 * @param sensorsAsyncResp Pointer to object holding response data.
 * @param inventoryItems D-Bus inventory items associated with sensors.
 * @param callback Callback to invoke when data has been obtained.
 */
/* @brief 获取 Power Supply 属性（如 EfficiencyPercent）。
 * 获取提供 Power Supply 属性数据的 D-Bus 连接（服务），
 * 然后从该连接获取 Power Supply 属性数据（目前假设只有 1 个连接），
 * 并将数据存储到 inventory item 中。
 * 这些数据稍后用于在 JSON 响应中提供传感器属性值。
 * D-Bus 上的 DeratingFactor 映射到 Redfish 上的 EfficiencyPercent。
 * 异步查找 Power Supply 属性数据。获取信息后调用回调。
 * 回调必须具有以下签名：
 *   @code
 *   callback(std::shared_ptr<std::vector<InventoryItem>> inventoryItems)
 *   @endcode
 * @param sensorsAsyncResp 指向保存响应数据的对象的指针。
 * @param inventoryItems 与传感器关联的 D-Bus inventory items。
 * @param callback 获取数据后调用的回调。 */
template <typename Callback>
void getPowerSupplyAttributes(
    const std::shared_ptr<SensorsAsyncResp>& sensorsAsyncResp,
    const std::shared_ptr<std::vector<InventoryItem>>& inventoryItems,
    Callback&& callback)
{
    BMCWEB_LOG_DEBUG("getPowerSupplyAttributes enter");

    // Only need the power supply attributes when the Power Schema
    // 只有在 Power Schema 时才需要电源属性。
    if (sensorsAsyncResp->chassisSubNode != sensors::powerNodeStr)
    {
        BMCWEB_LOG_DEBUG("getPowerSupplyAttributes exit since not Power");
        callback(inventoryItems);
        return;
    }

    constexpr std::array<std::string_view, 1> interfaces = {
        "xyz.openbmc_project.Control.PowerSupplyAttributes"};

    // Make call to ObjectMapper to find the PowerSupplyAttributes service
    // 调用 ObjectMapper 查找 PowerSupplyAttributes 服务。
    dbus::utility::getSubTree(
        "/xyz/openbmc_project", 0, interfaces,
        // ast-grep-ignore: long-lambda
        // ast-grep-ignore: long-lambda（忽略指令：ast-grep 跳过过长 lambda 检查）
        [callback = std::forward<Callback>(callback), sensorsAsyncResp,
         inventoryItems](
            const boost::system::error_code& ec,
            const dbus::utility::MapperGetSubTreeResponse& subtree) mutable {
            // Response handler for parsing output from GetSubTree
            // 用于解析 GetSubTree 输出的响应 handler。
            BMCWEB_LOG_DEBUG("getPowerSupplyAttributes respHandler enter");
            if (ec)
            {
                messages::internalError(sensorsAsyncResp->asyncResp->res);
                BMCWEB_LOG_ERROR(
                    "getPowerSupplyAttributes respHandler DBus error {}", ec);
                return;
            }
            if (subtree.empty())
            {
                BMCWEB_LOG_DEBUG("Can't find Power Supply Attributes!");
                callback(inventoryItems);
                return;
            }

            // Currently we only support 1 power supply attribute, use this for
            // all the power supplies. Build map of object path to connection.
            // Assume just 1 connection and 1 path for now.
            // 目前我们只支持 1 个电源属性，将其用于所有电源。
            // 构建对象路径到连接的映射。
            // 目前假设只有 1 个连接和 1 个路径。
            std::map<std::string, std::string> psAttributesConnections;

            if (subtree[0].first.empty() || subtree[0].second.empty())
            {
                BMCWEB_LOG_DEBUG("Power Supply Attributes mapper error!");
                callback(inventoryItems);
                return;
            }

            const std::string& psAttributesPath = subtree[0].first;
            const std::string& connection = subtree[0].second.begin()->first;

            if (connection.empty())
            {
                BMCWEB_LOG_DEBUG("Power Supply Attributes mapper error!");
                callback(inventoryItems);
                return;
            }

            psAttributesConnections[psAttributesPath] = connection;
            BMCWEB_LOG_DEBUG("Added mapping {} -> {}", psAttributesPath,
                             connection);

            getPowerSupplyAttributesData(sensorsAsyncResp, inventoryItems,
                                         psAttributesConnections,
                                         std::move(callback));
            BMCWEB_LOG_DEBUG("getPowerSupplyAttributes respHandler exit");
        });
    BMCWEB_LOG_DEBUG("getPowerSupplyAttributes exit");
}

/**
 * @brief Gets inventory items associated with sensors.
 *
 * Finds the inventory items that are associated with the specified sensors.
 * Then gets D-Bus data for the inventory items, such as presence and VPD.
 *
 * This data is later used to provide sensor property values in the JSON
 * response.
 *
 * Finds the inventory items asynchronously.  Invokes callback when the
 * inventory items have been obtained.
 *
 * The callback must have the following signature:
 *   @code
 *   callback(std::shared_ptr<std::vector<InventoryItem>> inventoryItems)
 *   @endcode
 *
 * @param sensorsAsyncResp Pointer to object holding response data.
 * @param sensorNames All sensors within the current chassis.
 * implements ObjectManager.
 * @param callback Callback to invoke when inventory items have been obtained.
 */
/* @brief 获取与传感器关联的 inventory items。
 * 查找与指定传感器关联的 inventory items。
 * 然后获取 inventory items 的 D-Bus 数据，例如 presence 和 VPD。
 * 这些数据稍后用于在 JSON 响应中提供传感器属性值。
 * 异步查找 inventory items。获取 inventory items 后调用回调。
 * 回调必须具有以下签名：
 *   @code
 *   callback(std::shared_ptr<std::vector<InventoryItem>> inventoryItems)
 *   @endcode
 * @param sensorsAsyncResp 指向保存响应数据的对象的指针。
 * @param sensorNames 当前 chassis 中的所有传感器。
 * 实现了 ObjectManager。
 * @param callback 获取 inventory items 后调用的回调。 */
template <typename Callback>
inline void getInventoryItems(
    const std::shared_ptr<SensorsAsyncResp>& sensorsAsyncResp,
    const std::shared_ptr<std::set<std::string>>& sensorNames,
    Callback&& callback)
{
    BMCWEB_LOG_DEBUG("getInventoryItems enter");
    auto getInventoryItemAssociationsCb =
        // ast-grep-ignore: long-lambda
        // ast-grep-ignore: long-lambda（忽略指令：ast-grep 跳过过长 lambda 检查）
        [sensorsAsyncResp, callback = std::forward<Callback>(callback)](
            const std::shared_ptr<std::vector<InventoryItem>>&
                inventoryItems) mutable {
            BMCWEB_LOG_DEBUG("getInventoryItemAssociationsCb enter");
            auto getInventoryItemsConnectionsCb =
                // ast-grep-ignore: long-lambda
                // ast-grep-ignore: long-lambda（忽略指令：ast-grep 跳过过长 lambda 检查）
                [sensorsAsyncResp, inventoryItems,
                 callback = std::forward<Callback>(callback)](
                    const std::shared_ptr<std::set<std::string>>&
                        invConnections) mutable {
                    BMCWEB_LOG_DEBUG("getInventoryItemsConnectionsCb enter");
                    auto getInventoryItemsDataCb =
                        // ast-grep-ignore: long-lambda
                        // ast-grep-ignore: long-lambda（忽略指令：ast-grep 跳过过长 lambda 检查）
                        [sensorsAsyncResp, inventoryItems,
                         callback =
                             std::forward<Callback>(callback)]() mutable {
                            BMCWEB_LOG_DEBUG("getInventoryItemsDataCb enter");

                            // Find Power Supply Attributes and get the data
                            // 查找 Power Supply 属性并获取数据。
                            getPowerSupplyAttributes(sensorsAsyncResp,
                                                     inventoryItems,
                                                     std::move(callback));
                            BMCWEB_LOG_DEBUG("getInventoryItemsDataCb exit");
                        };

                    // Get inventory item data from connections
                    // 从连接获取 inventory item 数据。
                    getInventoryItemsData(sensorsAsyncResp, inventoryItems,
                                          invConnections,
                                          std::move(getInventoryItemsDataCb));
                    BMCWEB_LOG_DEBUG("getInventoryItemsConnectionsCb exit");
                };

            // Get connections that provide inventory item data
            // 获取提供 inventory item 数据的连接。
            getInventoryItemsConnections(
                sensorsAsyncResp, inventoryItems,
                std::move(getInventoryItemsConnectionsCb));
            BMCWEB_LOG_DEBUG("getInventoryItemAssociationsCb exit");
        };

    // Get associations from sensors to inventory items
    // 获取从传感器到 inventory items 的关联。
    getInventoryItemAssociations(sensorsAsyncResp, sensorNames,
                                 std::move(getInventoryItemAssociationsCb));
    BMCWEB_LOG_DEBUG("getInventoryItems exit");
}

/**
 * @brief Returns JSON PowerSupply object for the specified inventory item.
 *
 * Searches for a JSON PowerSupply object that matches the specified inventory
 * item.  If one is not found, a new PowerSupply object is added to the JSON
 * array.
 *
 * Multiple sensors are often associated with one power supply inventory item.
 * As a result, multiple sensor values are stored in one JSON PowerSupply
 * object.
 *
 * @param powerSupplyArray JSON array containing Redfish PowerSupply objects.
 * @param inventoryItem Inventory item for the power supply.
 * @param chassisId Chassis that contains the power supply.
 * @return JSON PowerSupply object for the specified inventory item.
 */
/* @brief 为指定的 inventory item 返回 JSON PowerSupply 对象。
 * 搜索与指定 inventory item 匹配的 JSON PowerSupply 对象。
 * 如果未找到，则向 JSON 数组添加新的 PowerSupply 对象。
 * 通常多个传感器与一个电源 inventory item 关联。
 * 因此，多个传感器值存储在一个 JSON PowerSupply 对象中。
 * @param powerSupplyArray 包含 Redfish PowerSupply 对象的 JSON 数组。
 * @param inventoryItem 电源的 inventory item。
 * @param chassisId 包含该电源的 chassis。
 * @return 指定 inventory item 的 JSON PowerSupply 对象。 */
inline nlohmann::json& getPowerSupply(nlohmann::json& powerSupplyArray,
                                      const InventoryItem& inventoryItem,
                                      const std::string& chassisId)
{
    std::string nameS;
    nameS.resize(inventoryItem.name.size());
    std::ranges::replace_copy(inventoryItem.name, nameS.begin(), '_', ' ');
    // Check if matching PowerSupply object already exists in JSON array
    // 检查 JSON 数组中是否已存在匹配的 PowerSupply 对象。
    for (nlohmann::json& powerSupply : powerSupplyArray)
    {
        nlohmann::json::iterator nameIt = powerSupply.find("Name");
        if (nameIt == powerSupply.end())
        {
            continue;
        }
        const std::string* name = nameIt->get_ptr<std::string*>();
        if (name == nullptr)
        {
            continue;
        }
        if (nameS == *name)
        {
            return powerSupply;
        }
    }

    // Add new PowerSupply object to JSON array
    // 向 JSON 数组添加新的 PowerSupply 对象。
    powerSupplyArray.push_back({});
    nlohmann::json& powerSupply = powerSupplyArray.back();
    boost::urls::url url =
        boost::urls::format("/redfish/v1/Chassis/{}/Power", chassisId);
    url.set_fragment(("/PowerSupplies"_json_pointer).to_string());
    powerSupply["@odata.id"] = std::move(url);
    std::string escaped;
    escaped.resize(inventoryItem.name.size());
    std::ranges::replace_copy(inventoryItem.name, escaped.begin(), '_', ' ');
    powerSupply["Name"] = std::move(escaped);
    powerSupply["Manufacturer"] = inventoryItem.manufacturer;
    powerSupply["Model"] = inventoryItem.model;
    powerSupply["PartNumber"] = inventoryItem.partNumber;
    powerSupply["SerialNumber"] = inventoryItem.serialNumber;

    if (inventoryItem.powerSupplyEfficiencyPercent >= 0)
    {
        powerSupply["EfficiencyPercent"] =
            inventoryItem.powerSupplyEfficiencyPercent;
    }

    powerSupply["Status"]["State"] =
        sensor_utils::getState(&inventoryItem, true);
    resource::Health health = inventoryItem.isFunctional
                                  ? resource::Health::OK
                                  : resource::Health::Critical;
    powerSupply["Status"]["Health"] = health;

    return powerSupply;
}

/**
 * @brief Gets the values of the specified sensors.
 *
 * Stores the results as JSON in the SensorsAsyncResp.
 *
 * Gets the sensor values asynchronously.  Stores the results later when the
 * information has been obtained.
 *
 * The sensorNames set contains all requested sensors for the current chassis.
 *
 * To minimize the number of DBus calls, the DBus method
 * org.freedesktop.DBus.ObjectManager.GetManagedObjects() is used to get the
 * values of all sensors provided by a connection (service).
 *
 * The connections set contains all the connections that provide sensor values.
 *
 * The InventoryItem vector contains D-Bus inventory items associated with the
 * sensors.  Inventory item data is needed for some Redfish sensor properties.
 *
 * @param SensorsAsyncResp Pointer to object holding response data.
 * @param sensorNames All requested sensors within the current chassis.
 * @param connections Connections that provide sensor values.
 * implements ObjectManager.
 * @param inventoryItems Inventory items associated with the sensors.
 */
/* @brief 获取指定传感器的值。
 * 将结果作为 JSON 存储到 SensorsAsyncResp 中。
 * 异步获取传感器值。稍后在获得信息时存储结果。
 * sensorNames 集合包含当前 chassis 中所有请求的传感器。
 * 为减少 DBus 调用次数，使用 DBus 方法
 * org.freedesktop.DBus.ObjectManager.GetManagedObjects()
 * 来获取某个连接（服务）提供的所有传感器的值。
 * connections 集合包含所有提供传感器值的连接。
 * InventoryItem vector 包含与传感器关联的 D-Bus inventory items。
 * 某些 Redfish 传感器属性需要 inventory item 数据。
 * @param SensorsAsyncResp 指向保存响应数据的对象的指针。
 * @param sensorNames 当前 chassis 中所有请求的传感器。
 * @param connections 提供传感器值的连接。
 * 实现了 ObjectManager。
 * @param inventoryItems 与传感器关联的 inventory items。 */
inline void getSensorData(
    const std::shared_ptr<SensorsAsyncResp>& sensorsAsyncResp,
    const std::shared_ptr<std::set<std::string>>& sensorNames,
    const std::set<std::string>& connections,
    const std::shared_ptr<std::vector<InventoryItem>>& inventoryItems)
{
    BMCWEB_LOG_DEBUG("getSensorData enter");
    // Get managed objects from all services exposing sensors
    // 从所有暴露传感器的服务获取受管对象。
    for (const std::string& connection : connections)
    {
        sdbusplus::object_path sensorPath("/xyz/openbmc_project/sensors");
        dbus::utility::getManagedObjects(
            connection, sensorPath,
            // ast-grep-ignore: long-lambda
            // ast-grep-ignore: long-lambda（忽略指令：ast-grep 跳过过长 lambda 检查）
            [sensorsAsyncResp, sensorNames,
             inventoryItems](const boost::system::error_code& ec,
                             const dbus::utility::ManagedObjectType& resp) {
                BMCWEB_LOG_DEBUG("getManagedObjectsCb enter");
                if (ec)
                {
                    BMCWEB_LOG_ERROR("getManagedObjectsCb DBUS error: {}", ec);
                    messages::internalError(sensorsAsyncResp->asyncResp->res);
                    return;
                }
                auto chassisSubNode = sensor_utils::chassisSubNodeFromString(
                    sensorsAsyncResp->chassisSubNode);
                // Go through all objects and update response with sensor data
                // 遍历所有对象，用传感器数据更新响应。
                for (const auto& objDictEntry : resp)
                {
                    const std::string& objPath =
                        static_cast<const std::string&>(objDictEntry.first);
                    BMCWEB_LOG_DEBUG("getManagedObjectsCb parsing object {}",
                                     objPath);

                    std::vector<std::string> split;
                    // Reserve space for
                    // /xyz/openbmc_project/sensors/<name>/<subname>
                    // 为以下路径预留空间：
                    // /xyz/openbmc_project/sensors/<name>/<subname>
                    split.reserve(6);
                    // NOLINTNEXTLINE
                    bmcweb::split(split, objPath, '/');
                    if (split.size() < 6)
                    {
                        BMCWEB_LOG_ERROR("Got path that isn't long enough {}",
                                         objPath);
                        continue;
                    }
                    // These indexes aren't intuitive, as split puts an empty
                    // string at the beginning
                    // 这些索引不直观，因为 split 会在开头放入一个空字符串。
                    const std::string& sensorType = split[4];
                    const std::string& sensorName = split[5];
                    BMCWEB_LOG_DEBUG("sensorName {} sensorType {}", sensorName,
                                     sensorType);
                    if (!sensorNames->contains(objPath))
                    {
                        BMCWEB_LOG_DEBUG("{} not in sensor list ", sensorName);
                        continue;
                    }

                    // Find inventory item (if any) associated with sensor
                    // 查找与传感器关联的 inventory item（如果有）。
                    InventoryItem* inventoryItem =
                        findInventoryItemForSensor(inventoryItems, objPath);

                    const std::string& sensorSchema =
                        sensorsAsyncResp->chassisSubNode;

                    nlohmann::json* sensorJson = nullptr;

                    if (sensorSchema == sensors::sensorsNodeStr &&
                        !sensorsAsyncResp->efficientExpand)
                    {
                        std::string sensorId =
                            redfish::sensor_utils::getSensorId(sensorName,
                                                               sensorType);

                        sensorsAsyncResp->asyncResp->res
                            .jsonValue["@odata.id"] = boost::urls::format(
                            "/redfish/v1/Chassis/{}/{}/{}",
                            sensorsAsyncResp->chassisId,
                            sensorsAsyncResp->chassisSubNode, sensorId);
                        sensorJson =
                            &(sensorsAsyncResp->asyncResp->res.jsonValue);
                    }
                    else
                    {
                        std::string fieldName;
                        if (sensorsAsyncResp->efficientExpand)
                        {
                            fieldName = "Members";
                        }
                        else if (sensorType == "temperature")
                        {
                            fieldName = "Temperatures";
                        }
                        else if (sensorType == "fan" ||
                                 sensorType == "fan_tach" ||
                                 sensorType == "fan_pwm")
                        {
                            fieldName = "Fans";
                        }
                        else if (sensorType == "voltage")
                        {
                            fieldName = "Voltages";
                        }
                        else if (sensorType == "power")
                        {
                            if (sensorName == "total_power")
                            {
                                fieldName = "PowerControl";
                            }
                            else if ((inventoryItem != nullptr) &&
                                     (inventoryItem->isPowerSupply))
                            {
                                fieldName = "PowerSupplies";
                            }
                            else
                            {
                                // Other power sensors are in SensorCollection
                                // 其他电源传感器位于 SensorCollection 中。
                                continue;
                            }
                        }
                        else
                        {
                            BMCWEB_LOG_ERROR(
                                "Unsure how to handle sensorType {}",
                                sensorType);
                            continue;
                        }

                        nlohmann::json& tempArray =
                            sensorsAsyncResp->asyncResp->res
                                .jsonValue[fieldName];
                        if (fieldName == "PowerControl")
                        {
                            if (tempArray.empty())
                            {
                                // Put multiple "sensors" into a single
                                // PowerControl. Follows MemberId naming and
                                // naming in power.hpp.
                                // 将多个 "sensor" 放入单个 PowerControl。
                                // 遵循 MemberId 命名以及 power.hpp 中的命名。
                                nlohmann::json::object_t power;
                                boost::urls::url url = boost::urls::format(
                                    "/redfish/v1/Chassis/{}/{}",
                                    sensorsAsyncResp->chassisId,
                                    sensorsAsyncResp->chassisSubNode);
                                url.set_fragment(
                                    (""_json_pointer / fieldName / "0")
                                        .to_string());
                                power["@odata.id"] = std::move(url);
                                tempArray.emplace_back(std::move(power));
                            }
                            sensorJson = &(tempArray.back());
                        }
                        else if (fieldName == "PowerSupplies")
                        {
                            if (inventoryItem != nullptr)
                            {
                                sensorJson = &(getPowerSupply(
                                    tempArray, *inventoryItem,
                                    sensorsAsyncResp->chassisId));
                            }
                        }
                        else if (fieldName == "Members")
                        {
                            std::string sensorId =
                                redfish::sensor_utils::getSensorId(sensorName,
                                                                   sensorType);

                            nlohmann::json::object_t member;
                            member["@odata.id"] = boost::urls::format(
                                "/redfish/v1/Chassis/{}/{}/{}",
                                sensorsAsyncResp->chassisId,
                                sensorsAsyncResp->chassisSubNode, sensorId);
                            tempArray.emplace_back(std::move(member));
                            sensorJson = &(tempArray.back());
                        }
                        else
                        {
                            nlohmann::json::object_t member;
                            boost::urls::url url = boost::urls::format(
                                "/redfish/v1/Chassis/{}/{}",
                                sensorsAsyncResp->chassisId,
                                sensorsAsyncResp->chassisSubNode);
                            url.set_fragment(
                                (""_json_pointer / fieldName).to_string());
                            member["@odata.id"] = std::move(url);
                            tempArray.emplace_back(std::move(member));
                            sensorJson = &(tempArray.back());
                        }
                    }

                    if (sensorJson != nullptr)
                    {
                        objectInterfacesToJson(
                            sensorName, sensorType, chassisSubNode,
                            objDictEntry.second, *sensorJson, inventoryItem);

                        std::string path = "/xyz/openbmc_project/sensors/";
                        path += sensorType;
                        path += "/";
                        path += sensorName;
                        sensorsAsyncResp->addMetadata(*sensorJson, path);
                    }
                }
                if (sensorsAsyncResp.use_count() == 1)
                {
                    sortJSONResponse(sensorsAsyncResp);
                    if (chassisSubNode ==
                            sensor_utils::ChassisSubNode::sensorsNode &&
                        sensorsAsyncResp->efficientExpand)
                    {
                        sensorsAsyncResp->asyncResp->res
                            .jsonValue["Members@odata.count"] =
                            sensorsAsyncResp->asyncResp->res
                                .jsonValue["Members"]
                                .size();
                    }
                    else if (chassisSubNode ==
                             sensor_utils::ChassisSubNode::thermalNode)
                    {
                        populateFanRedundancy(sensorsAsyncResp);
                    }
                }
                BMCWEB_LOG_DEBUG("getManagedObjectsCb exit");
            });
    }
    BMCWEB_LOG_DEBUG("getSensorData exit");
}

inline void processSensorList(
    const std::shared_ptr<SensorsAsyncResp>& sensorsAsyncResp,
    const std::shared_ptr<std::set<std::string>>& sensorNames)
{
    // ast-grep-ignore: long-lambda
    // ast-grep-ignore: long-lambda（忽略指令：ast-grep 跳过过长 lambda 检查）
    auto getConnectionCb = [sensorsAsyncResp, sensorNames](
                               const std::set<std::string>& connections) {
        BMCWEB_LOG_DEBUG("getConnectionCb enter");
        auto getInventoryItemsCb =
            [sensorsAsyncResp, sensorNames, connections](
                const std::shared_ptr<std::vector<InventoryItem>>&
                    inventoryItems) mutable {
                BMCWEB_LOG_DEBUG("getInventoryItemsCb enter");
                // Get sensor data and store results in JSON
                // 获取传感器数据并将结果存入 JSON。
                getSensorData(sensorsAsyncResp, sensorNames, connections,
                              inventoryItems);
                BMCWEB_LOG_DEBUG("getInventoryItemsCb exit");
            };

        // Get inventory items associated with sensors
        // 获取与传感器关联的 inventory items。
        getInventoryItems(sensorsAsyncResp, sensorNames,
                          std::move(getInventoryItemsCb));

        BMCWEB_LOG_DEBUG("getConnectionCb exit");
    };

    // Get set of connections that provide sensor values
    // 获取提供传感器值的连接集合。
    getConnections(sensorsAsyncResp, sensorNames, std::move(getConnectionCb));
}

/**
 * @brief Entry point for retrieving sensors data related to requested
 *        chassis.
 * @param SensorsAsyncResp   Pointer to object holding response data
 */
/* @brief 检索与请求 chassis 相关的传感器数据的入口。
 * @param SensorsAsyncResp 指向保存响应数据的对象的指针。 */
inline void getChassisData(
    const std::shared_ptr<SensorsAsyncResp>& sensorsAsyncResp)
{
    BMCWEB_LOG_DEBUG("getChassisData enter");
    auto getChassisCb =
        [sensorsAsyncResp](
            const std::shared_ptr<std::set<std::string>>& sensorNames) {
            BMCWEB_LOG_DEBUG("getChassisCb enter");
            processSensorList(sensorsAsyncResp, sensorNames);
            BMCWEB_LOG_DEBUG("getChassisCb exit");
        };
    // SensorCollection doesn't contain the Redundancy property
    // SensorCollection 不包含 Redundancy 属性。
    if (sensorsAsyncResp->chassisSubNode != sensors::sensorsNodeStr)
    {
        sensorsAsyncResp->asyncResp->res.jsonValue["Redundancy"] =
            nlohmann::json::array();
    }
    // Get set of sensors in chassis
    // 获取 chassis 中的传感器集合。
    getChassis(sensorsAsyncResp->asyncResp, sensorsAsyncResp->chassisId,
               sensorsAsyncResp->chassisSubNode, sensorsAsyncResp->types,
               std::move(getChassisCb));
    BMCWEB_LOG_DEBUG("getChassisData exit");
}

/**
 * @brief Find the requested sensorName in the list of all sensors supplied by
 * the chassis node
 *
 * @param sensorName   The sensor name supplied in the PATCH request
 * @param sensorsList  The list of sensors managed by the chassis node
 * @param sensorsModified  The list of sensors that were found as a result of
 *                         repeated calls to this function
 */
/* @brief 在 chassis 节点提供的所有传感器列表中查找请求的 sensorName。
 * @param sensorName PATCH 请求中提供的传感器名称。
 * @param sensorsList chassis 节点管理的传感器列表。
 * @param sensorsModified 通过反复调用该函数而找到的传感器列表。 */
inline bool findSensorNameUsingSensorPath(
    std::string_view sensorName, const std::set<std::string>& sensorsList,
    std::set<std::string>& sensorsModified)
{
    for (const auto& chassisSensor : sensorsList)
    {
        sdbusplus::object_path path(chassisSensor);
        std::string thisSensorName = path.filename();
        if (thisSensorName.empty())
        {
            continue;
        }
        if (thisSensorName == sensorName)
        {
            sensorsModified.emplace(chassisSensor);
            return true;
        }
    }
    return false;
}

/**
 * @brief Entry point for overriding sensor values of given sensor
 *
 * @param sensorAsyncResp   response object
 * @param allCollections   Collections extract from sensors' request patch info
 * @param chassisSubNode   Chassis Node for which the query has to happen
 */
/* @brief 覆盖给定传感器值的入口。
 * @param sensorAsyncResp 响应对象。
 * @param allCollections 从传感器的 PATCH 请求信息中提取的集合。
 * @param chassisSubNode 要查询的 Chassis 节点。 */
inline void setSensorsOverride(
    const std::shared_ptr<SensorsAsyncResp>& sensorAsyncResp,
    std::unordered_map<std::string, std::vector<nlohmann::json::object_t>>&
        allCollections)
{
    BMCWEB_LOG_INFO("setSensorsOverride for subNode{}",
                    sensorAsyncResp->chassisSubNode);

    std::string_view propertyValueName;
    std::unordered_map<std::string, std::pair<double, std::string>> overrideMap;
    std::string memberId;
    double value = 0.0;
    for (auto& collectionItems : allCollections)
    {
        if (collectionItems.first == "Temperatures")
        {
            propertyValueName = "ReadingCelsius";
        }
        else if (collectionItems.first == "Fans")
        {
            propertyValueName = "Reading";
        }
        else
        {
            propertyValueName = "ReadingVolts";
        }
        for (auto& item : collectionItems.second)
        {
            if (!json_util::readJsonObject(                //
                    item, sensorAsyncResp->asyncResp->res, //
                    "MemberId", memberId,                  //
                    propertyValueName, value               //
                    ))
            {
                return;
            }
            overrideMap.emplace(memberId,
                                std::make_pair(value, collectionItems.first));
        }
    }

    // ast-grep-ignore: long-lambda
    // ast-grep-ignore: long-lambda（忽略指令：ast-grep 跳过过长 lambda 检查）
    auto getChassisSensorListCb = [sensorAsyncResp, overrideMap,
                                   propertyValueNameStr =
                                       std::string(propertyValueName)](
                                      const std::shared_ptr<
                                          std::set<std::string>>& sensorsList) {
        // Match sensor names in the PATCH request to those managed by the
        // chassis node
        // 将 PATCH 请求中的传感器名称与 chassis 节点管理的传感器名称匹配。
        const std::shared_ptr<std::set<std::string>> sensorNames =
            std::make_shared<std::set<std::string>>();
        for (const auto& item : overrideMap)
        {
            const auto& sensor = item.first;
            std::pair<std::string, std::string> sensorNameType =
                redfish::sensor_utils::splitSensorNameAndType(sensor);
            if (!findSensorNameUsingSensorPath(sensorNameType.second,
                                               *sensorsList, *sensorNames))
            {
                BMCWEB_LOG_INFO("Unable to find memberId {}", item.first);
                messages::resourceNotFound(sensorAsyncResp->asyncResp->res,
                                           item.second.second, item.first);
                return;
            }
        }
        // Get the connection to which the memberId belongs
        // ast-grep-ignore: long-lambda
        // 获取 memberId 所属的连接。
        // ast-grep-ignore: long-lambda（忽略指令：ast-grep 跳过过长 lambda 检查）
        auto getObjectsWithConnectionCb = [sensorAsyncResp, overrideMap,
                                           propertyValueNameStr](
                                              const std::set<
                                                  std::string>& /*connections*/,
                                              const std::set<std::pair<
                                                  std::string, std::string>>&
                                                  objectsWithConnection) {
            if (objectsWithConnection.size() != overrideMap.size())
            {
                BMCWEB_LOG_INFO(
                    "Unable to find all objects with proper connection {} requested {}",
                    objectsWithConnection.size(), overrideMap.size());
                messages::resourceNotFound(
                    sensorAsyncResp->asyncResp->res,
                    sensorAsyncResp->chassisSubNode == sensors::thermalNodeStr
                        ? "Temperatures"
                        : "Voltages",
                    "Count");
                return;
            }
            for (const auto& item : objectsWithConnection)
            {
                sdbusplus::object_path path(item.first);
                std::string sensorName = path.filename();
                if (sensorName.empty())
                {
                    messages::internalError(sensorAsyncResp->asyncResp->res);
                    return;
                }
                std::string id = redfish::sensor_utils::getSensorId(
                    sensorName, path.parent_path().filename());

                const auto& iterator = overrideMap.find(id);
                if (iterator == overrideMap.end())
                {
                    BMCWEB_LOG_INFO("Unable to find sensor object{}",
                                    item.first);
                    messages::internalError(sensorAsyncResp->asyncResp->res);
                    return;
                }
                setDbusProperty(sensorAsyncResp->asyncResp,
                                propertyValueNameStr, item.second, item.first,
                                "xyz.openbmc_project.Sensor.Value", "Value",
                                iterator->second.first);
            }
        };
        // Get object with connection for the given sensor name
        // 获取给定传感器名称对应的带连接的对象。
        getObjectsWithConnection(sensorAsyncResp, sensorNames,
                                 std::move(getObjectsWithConnectionCb));
    };
    // get full sensor list for the given chassisId and cross verify the sensor.
    // 获取给定 chassisId 的完整传感器列表并交叉验证该传感器。
    getChassis(sensorAsyncResp->asyncResp, sensorAsyncResp->chassisId,
               sensorAsyncResp->chassisSubNode, sensorAsyncResp->types,
               std::move(getChassisSensorListCb));
}

/**
 * @brief Retrieves mapping of Redfish URIs to sensor value property to D-Bus
 * path of the sensor.
 *
 * Function builds valid Redfish response for sensor query of given chassis and
 * node. It then builds metadata about Redfish<->D-Bus correlations and provides
 * it to caller in a callback.
 *
 * @param chassis   Chassis for which retrieval should be performed
 * @param node  Node (group) of sensors. See sensor_utils::node for supported
 * values
 * @param mapComplete   Callback to be called with retrieval result
 */
/* @brief 检索 Redfish URI 到传感器值属性再到传感器 D-Bus 路径的映射。
 * 该函数为给定 chassis 和节点的传感器查询构建有效的 Redfish 响应，
 * 然后构建有关 Redfish<->D-Bus 对应关系的元数据，并通过回调提供给调用方。
 * @param chassis 要执行检索的 chassis。
 * @param node 传感器节点（组）。支持的值参见 sensor_utils::node。
 * @param mapComplete 用检索结果调用的回调。 */
template <typename Callback>
inline void retrieveUriToDbusMap(
    const std::string& chassis, const std::string& node, Callback&& mapComplete)
{
    decltype(sensors::paths)::const_iterator pathIt =
        std::find_if(sensors::paths.cbegin(), sensors::paths.cend(),
                     [&node](auto&& val) { return val.first == node; });
    if (pathIt == sensors::paths.cend())
    {
        BMCWEB_LOG_ERROR("Wrong node provided : {}", node);
        std::map<std::string, std::string> noop;
        mapComplete(boost::beast::http::status::bad_request, noop);
        return;
    }

    auto asyncResp = std::make_shared<bmcweb::AsyncResp>();
    auto callback =
        [asyncResp, mapCompleteCb = std::forward<Callback>(mapComplete)](
            const boost::beast::http::status status,
            const std::map<std::string, std::string>& uriToDbus) {
            mapCompleteCb(status, uriToDbus);
        };

    auto resp = std::make_shared<SensorsAsyncResp>(
        asyncResp, chassis, pathIt->second, node, std::move(callback));
    getChassisData(resp);
}

namespace sensors
{

inline void getChassisCallback(
    const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
    std::string_view chassisId, std::string_view chassisSubNode,
    const std::shared_ptr<std::set<std::string>>& sensorNames)
{
    BMCWEB_LOG_DEBUG("getChassisCallback enter ");

    nlohmann::json& entriesArray = asyncResp->res.jsonValue["Members"];
    for (const std::string& sensor : *sensorNames)
    {
        BMCWEB_LOG_DEBUG("Adding sensor: {}", sensor);

        sdbusplus::object_path path(sensor);
        std::string sensorName = path.filename();
        if (sensorName.empty())
        {
            BMCWEB_LOG_ERROR("Invalid sensor path: {}", sensor);
            messages::internalError(asyncResp->res);
            return;
        }
        std::string type = path.parent_path().filename();
        std::string id = redfish::sensor_utils::getSensorId(sensorName, type);

        nlohmann::json::object_t member;
        member["@odata.id"] = boost::urls::format(
            "/redfish/v1/Chassis/{}/{}/{}", chassisId, chassisSubNode, id);

        entriesArray.emplace_back(std::move(member));
    }

    asyncResp->res.jsonValue["Members@odata.count"] = entriesArray.size();
    BMCWEB_LOG_DEBUG("getChassisCallback exit");
}

inline void handleSensorCollectionGet(
    App& app, const crow::Request& req,
    const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
    const std::string& chassisId)
{
    query_param::QueryCapabilities capabilities = {
        .canDelegateExpandLevel = 1,
    };
    query_param::Query delegatedQuery;
    if (!redfish::setUpRedfishRouteWithDelegation(app, req, asyncResp,
                                                  delegatedQuery, capabilities))
    {
        return;
    }

    if (delegatedQuery.expandType != query_param::ExpandType::None)
    {
        // we perform efficient expand.
        // 我们执行高效展开（efficient expand）。
        auto sensorsAsyncResp = std::make_shared<SensorsAsyncResp>(
            asyncResp, chassisId, sensors::dbus::sensorPaths,
            sensors::sensorsNodeStr,
            /*efficientExpand=*/true);
        getChassisData(sensorsAsyncResp);

        BMCWEB_LOG_DEBUG(
            "SensorCollection doGet exit via efficient expand handler");
        return;
    }

    // We get all sensors as hyperlinkes in the chassis (this
    // implies we reply on the default query parameters handler)
    // 我们以超链接形式获取 chassis 中的所有传感器
    // （这意味着我们依赖默认的查询参数处理器）。
    getChassis(asyncResp, chassisId, sensors::sensorsNodeStr, dbus::sensorPaths,
               std::bind_front(sensors::getChassisCallback, asyncResp,
                               chassisId, sensors::sensorsNodeStr));
}

inline void getSensorFromDbus(
    const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
    const std::string& sensorPath,
    const ::dbus::utility::MapperGetObject& mapperResponse)
{
    if (mapperResponse.size() != 1)
    {
        messages::internalError(asyncResp->res);
        return;
    }
    const auto& valueIface = *mapperResponse.begin();
    const std::string& connectionName = valueIface.first;
    BMCWEB_LOG_DEBUG("Looking up {}", connectionName);
    BMCWEB_LOG_DEBUG("Path {}", sensorPath);

    ::dbus::utility::getAllProperties(
        *crow::connections::systemBus, connectionName, sensorPath, "",
        // ast-grep-ignore: long-lambda
        // ast-grep-ignore: long-lambda（忽略指令：ast-grep 跳过过长 lambda 检查）
        [asyncResp,
         sensorPath](const boost::system::error_code& ec,
                     const ::dbus::utility::DBusPropertiesMap& valuesDict) {
            if (ec)
            {
                messages::internalError(asyncResp->res);
                return;
            }
            sdbusplus::object_path path(sensorPath);
            std::string name = path.filename();
            path = path.parent_path();
            std::string type = path.filename();
            sensor_utils::objectPropertiesToJson(
                name, type, sensor_utils::ChassisSubNode::sensorsNode,
                valuesDict, asyncResp->res.jsonValue, nullptr);
        });
}

inline void handleSensorGet(App& app, const crow::Request& req,
                            const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
                            const std::string& chassisId,
                            const std::string& sensorId)
{
    if (!redfish::setUpRedfishRoute(app, req, asyncResp))
    {
        return;
    }
    std::pair<std::string, std::string> nameType =
        redfish::sensor_utils::splitSensorNameAndType(sensorId);
    if (nameType.first.empty() || nameType.second.empty())
    {
        messages::resourceNotFound(asyncResp->res, "Sensor", sensorId);
        return;
    }

    asyncResp->res.jsonValue["@odata.id"] = boost::urls::format(
        "/redfish/v1/Chassis/{}/Sensors/{}", chassisId, sensorId);

    BMCWEB_LOG_DEBUG("Sensor doGet enter");

    constexpr std::array<std::string_view, 1> interfaces = {
        "xyz.openbmc_project.Sensor.Value"};
    std::string sensorPath = "/xyz/openbmc_project/sensors/" + nameType.first +
                             '/' + nameType.second;
    // Get a list of all of the sensors that implement Sensor.Value
    // and get the path and service name associated with the sensor
    // 获取所有实现 Sensor.Value 的传感器列表，
    // 并获取与该传感器关联的路径和服务名。
    ::dbus::utility::getDbusObject(
        sensorPath, interfaces,
        // ast-grep-ignore: long-lambda
        // ast-grep-ignore: long-lambda（忽略指令：ast-grep 跳过过长 lambda 检查）
        [asyncResp, sensorId,
         sensorPath](const boost::system::error_code& ec,
                     const ::dbus::utility::MapperGetObject& subtree) {
            BMCWEB_LOG_DEBUG("respHandler1 enter");
            if (ec == boost::system::errc::io_error)
            {
                BMCWEB_LOG_WARNING("Sensor not found from getSensorPaths");
                messages::resourceNotFound(asyncResp->res, "Sensor", sensorId);
                return;
            }
            if (ec)
            {
                messages::internalError(asyncResp->res);
                BMCWEB_LOG_ERROR(
                    "Sensor getSensorPaths resp_handler: Dbus error {}", ec);
                return;
            }
            getSensorFromDbus(asyncResp, sensorPath, subtree);
            BMCWEB_LOG_DEBUG("respHandler1 exit");
        });
}

} // namespace sensors

inline void requestRoutesSensorCollection(App& app)
{
    BMCWEB_ROUTE(app, "/redfish/v1/Chassis/<str>/Sensors/")
        .privileges(redfish::privileges::getSensorCollection)
        .methods(boost::beast::http::verb::get)(
            std::bind_front(sensors::handleSensorCollectionGet, std::ref(app)));
}

inline void requestRoutesSensor(App& app)
{
    BMCWEB_ROUTE(app, "/redfish/v1/Chassis/<str>/Sensors/<str>/")
        .privileges(redfish::privileges::getSensor)
        .methods(boost::beast::http::verb::get)(
            std::bind_front(sensors::handleSensorGet, std::ref(app)));
}

} // namespace redfish
