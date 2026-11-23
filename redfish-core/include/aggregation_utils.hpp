// SPDX-License-Identifier: Apache-2.0
// SPDX-FileCopyrightText: Copyright OpenBMC Authors
#pragma once

/****************************************************************
 *                 READ THIS WARNING FIRST
 * This is an auto-generated header which contains definitions
 * for Redfish DMTF defined schemas.
 * DO NOT modify this registry outside of running the
 * update_schemas.py script.  The definitions contained within
 * this file are owned by DMTF.  Any modifications to these files
 * should be first pushed to the relevant registry in the DMTF
 * github organization.
 ***************************************************************/
/*                  请先阅读此警告
 * 这是一个自动生成的头文件，其中包含针对 Redfish DMTF 定义的
 * schema 的定义。
 * 除运行 update_schemas.py 脚本外，请勿修改此注册表。
 * 本文件中包含的定义归 DMTF 所有。
 * 对这些文件的任何修改，
 * 都应先推送到 DMTF
 * github 组织中的相应注册表。 */
// clang-format off
#include <array>
#include <string_view>

namespace redfish
{
// Note that each URI actually begins with "/redfish/v1"
// They've been omitted to save space and reduce search time
// 注意，每个 URI 实际上都以 "/redfish/v1" 开头
// 为节省空间和缩短搜索时间，此处将其省略
constexpr std::array<std::string_view, 59> topCollections{
    "/AggregationService/Aggregates",
    "/AggregationService/AggregationSources",
    "/AggregationService/ConnectionMethods",
    "/AutomationNodes",
    "/Cables",
    "/CertificateService/CertificateEnrollments",
    "/CertificateService/EnrollmentCACertificates",
    "/Chassis",
    "/ComponentIntegrity",
    "/CompositionService/ActivePool",
    "/CompositionService/CompositionReservations",
    "/CompositionService/FreePool",
    "/CompositionService/ResourceBlocks",
    "/CompositionService/ResourceZones",
    "/EventService/Subscriptions",
    "/Fabrics",
    "/Facilities",
    "/JobService/JobDocuments",
    "/JobService/JobExecutors",
    "/JobService/Jobs",
    "/JobService/Log/Entries",
    "/KeyService/NVMeoFKeyPolicies",
    "/KeyService/NVMeoFSecrets",
    "/LicenseService/Licenses",
    "/Managers",
    "/NVMeDomains",
    "/PolicyService/Policies",
    "/PowerEquipment/ElectricalBuses",
    "/PowerEquipment/FloorPDUs",
    "/PowerEquipment/PowerShelves",
    "/PowerEquipment/RackPDUs",
    "/PowerEquipment/Switchgear",
    "/PowerEquipment/TransferSwitches",
    "/RegisteredClients",
    "/Registries",
    "/ResourceBlocks",
    "/Storage",
    "/StorageServices",
    "/StorageSystems",
    "/Systems",
    "/TaskService/TaskMonitors",
    "/TaskService/Tasks",
    "/TelemetryService/LogService/Entries",
    "/TelemetryService/MetricDefinitions",
    "/TelemetryService/MetricReportDefinitions",
    "/TelemetryService/MetricReports",
    "/TelemetryService/TelemetryData",
    "/TelemetryService/Triggers",
    "/ThermalEquipment/CDUs",
    "/ThermalEquipment/CoolingLoops",
    "/ThermalEquipment/HeatExchangers",
    "/ThermalEquipment/ImmersionUnits",
    "/ThermalEquipment/RPUs",
    "/UpdateService/ClientCertificates",
    "/UpdateService/FirmwareInventory",
    "/UpdateService/LocalImageStore",
    "/UpdateService/RemoteServerCertificates",
    "/UpdateService/RemoteServerSSHKeys",
    "/UpdateService/SoftwareInventory",
};
} // namespace redfish
