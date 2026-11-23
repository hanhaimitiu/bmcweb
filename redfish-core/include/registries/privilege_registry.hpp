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
#include "privileges.hpp"

#include <array>

// clang-format off

namespace redfish::privileges
{
const std::array<Privileges, 1> privilegeSetLogin = {{
    {"Login"}
}};
const std::array<Privileges, 1> privilegeSetConfigureComponents = {{
    {"ConfigureComponents"}
}};
const std::array<Privileges, 1> privilegeSetConfigureUsers = {{
    {"ConfigureUsers"}
}};
const std::array<Privileges, 1> privilegeSetConfigureManager = {{
    {"ConfigureManager"}
}};
const std::array<Privileges, 2> privilegeSetConfigureManagerOrConfigureComponents = {{
    {"ConfigureManager"},
    {"ConfigureComponents"}
}};
const std::array<Privileges, 2> privilegeSetConfigureManagerOrConfigureSelf = {{
    {"ConfigureManager"},
    {"ConfigureSelf"}
}};
const std::array<Privileges, 3> privilegeSetConfigureManagerOrConfigureUsersOrConfigureSelf = {{
    {"ConfigureManager"},
    {"ConfigureUsers"},
    {"ConfigureSelf"}
}};
const std::array<Privileges, 2> privilegeSetLoginOrNoAuth = {{
    {"Login"},
    {}
}};
// AccelerationFunction
// 加速功能（AccelerationFunction）
const static auto& getAccelerationFunction = privilegeSetLogin;
const static auto& headAccelerationFunction = privilegeSetLogin;
const static auto& patchAccelerationFunction = privilegeSetConfigureComponents;
const static auto& putAccelerationFunction = privilegeSetConfigureComponents;
const static auto& deleteAccelerationFunction = privilegeSetConfigureComponents;
const static auto& postAccelerationFunction = privilegeSetConfigureComponents;

// AccelerationFunctionCollection
// 加速功能集合（AccelerationFunctionCollection）
const static auto& getAccelerationFunctionCollection = privilegeSetLogin;
const static auto& headAccelerationFunctionCollection = privilegeSetLogin;
const static auto& patchAccelerationFunctionCollection = privilegeSetConfigureComponents;
const static auto& putAccelerationFunctionCollection = privilegeSetConfigureComponents;
const static auto& deleteAccelerationFunctionCollection = privilegeSetConfigureComponents;
const static auto& postAccelerationFunctionCollection = privilegeSetConfigureComponents;

// AccountService
// 账户服务（AccountService）
const static auto& getAccountService = privilegeSetLogin;
const static auto& headAccountService = privilegeSetLogin;
const static auto& patchAccountService = privilegeSetConfigureUsers;
const static auto& putAccountService = privilegeSetConfigureUsers;
const static auto& deleteAccountService = privilegeSetConfigureUsers;
const static auto& postAccountService = privilegeSetConfigureUsers;

// ActionInfo
// 操作信息（ActionInfo）
const static auto& getActionInfo = privilegeSetLogin;
const static auto& headActionInfo = privilegeSetLogin;
const static auto& patchActionInfo = privilegeSetConfigureManager;
const static auto& putActionInfo = privilegeSetConfigureManager;
const static auto& deleteActionInfo = privilegeSetConfigureManager;
const static auto& postActionInfo = privilegeSetConfigureManager;

// AddressPool
// 地址池（AddressPool）
const static auto& getAddressPool = privilegeSetLogin;
const static auto& headAddressPool = privilegeSetLogin;
const static auto& patchAddressPool = privilegeSetConfigureComponents;
const static auto& putAddressPool = privilegeSetConfigureComponents;
const static auto& deleteAddressPool = privilegeSetConfigureComponents;
const static auto& postAddressPool = privilegeSetConfigureComponents;

// AddressPoolCollection
// 地址池集合（AddressPoolCollection）
const static auto& getAddressPoolCollection = privilegeSetLogin;
const static auto& headAddressPoolCollection = privilegeSetLogin;
const static auto& patchAddressPoolCollection = privilegeSetConfigureComponents;
const static auto& putAddressPoolCollection = privilegeSetConfigureComponents;
const static auto& deleteAddressPoolCollection = privilegeSetConfigureComponents;
const static auto& postAddressPoolCollection = privilegeSetConfigureComponents;

// Aggregate
// 聚合（Aggregate）
const static auto& getAggregate = privilegeSetLogin;
const static auto& headAggregate = privilegeSetLogin;
const static auto& patchAggregate = privilegeSetConfigureManagerOrConfigureComponents;
const static auto& putAggregate = privilegeSetConfigureManagerOrConfigureComponents;
const static auto& deleteAggregate = privilegeSetConfigureManagerOrConfigureComponents;
const static auto& postAggregate = privilegeSetConfigureManagerOrConfigureComponents;

// AggregateCollection
// 聚合集合（AggregateCollection）
const static auto& getAggregateCollection = privilegeSetLogin;
const static auto& headAggregateCollection = privilegeSetLogin;
const static auto& patchAggregateCollection = privilegeSetConfigureManagerOrConfigureComponents;
const static auto& putAggregateCollection = privilegeSetConfigureManagerOrConfigureComponents;
const static auto& deleteAggregateCollection = privilegeSetConfigureManagerOrConfigureComponents;
const static auto& postAggregateCollection = privilegeSetConfigureManagerOrConfigureComponents;

// AggregationService
// 聚合服务（AggregationService）
const static auto& getAggregationService = privilegeSetLogin;
const static auto& headAggregationService = privilegeSetLogin;
const static auto& patchAggregationService = privilegeSetConfigureManager;
const static auto& putAggregationService = privilegeSetConfigureManager;
const static auto& deleteAggregationService = privilegeSetConfigureManager;
const static auto& postAggregationService = privilegeSetConfigureManager;

// AggregationSource
// 聚合源（AggregationSource）
const static auto& getAggregationSource = privilegeSetLogin;
const static auto& headAggregationSource = privilegeSetLogin;
const static auto& patchAggregationSource = privilegeSetConfigureManager;
const static auto& putAggregationSource = privilegeSetConfigureManager;
const static auto& deleteAggregationSource = privilegeSetConfigureManager;
const static auto& postAggregationSource = privilegeSetConfigureManager;

// AggregationSourceCollection
// 聚合源集合（AggregationSourceCollection）
const static auto& getAggregationSourceCollection = privilegeSetLogin;
const static auto& headAggregationSourceCollection = privilegeSetLogin;
const static auto& patchAggregationSourceCollection = privilegeSetConfigureManager;
const static auto& putAggregationSourceCollection = privilegeSetConfigureManager;
const static auto& deleteAggregationSourceCollection = privilegeSetConfigureManager;
const static auto& postAggregationSourceCollection = privilegeSetConfigureManager;

// AllowDeny
// 允许/拒绝（AllowDeny）
const static auto& getAllowDeny = privilegeSetLogin;
const static auto& headAllowDeny = privilegeSetLogin;
const static auto& patchAllowDeny = privilegeSetConfigureManager;
const static auto& putAllowDeny = privilegeSetConfigureManager;
const static auto& deleteAllowDeny = privilegeSetConfigureManager;
const static auto& postAllowDeny = privilegeSetConfigureManager;

// AllowDenyCollection
// 允许/拒绝集合（AllowDenyCollection）
const static auto& getAllowDenyCollection = privilegeSetLogin;
const static auto& headAllowDenyCollection = privilegeSetLogin;
const static auto& patchAllowDenyCollection = privilegeSetConfigureManager;
const static auto& putAllowDenyCollection = privilegeSetConfigureManager;
const static auto& deleteAllowDenyCollection = privilegeSetConfigureManager;
const static auto& postAllowDenyCollection = privilegeSetConfigureManager;

// Application
// 应用（Application）
const static auto& getApplication = privilegeSetLogin;
const static auto& headApplication = privilegeSetLogin;
const static auto& patchApplication = privilegeSetConfigureComponents;
const static auto& putApplication = privilegeSetConfigureComponents;
const static auto& deleteApplication = privilegeSetConfigureComponents;
const static auto& postApplication = privilegeSetConfigureComponents;

// ApplicationCollection
// 应用集合（ApplicationCollection）
const static auto& getApplicationCollection = privilegeSetLogin;
const static auto& headApplicationCollection = privilegeSetLogin;
const static auto& patchApplicationCollection = privilegeSetConfigureComponents;
const static auto& putApplicationCollection = privilegeSetConfigureComponents;
const static auto& deleteApplicationCollection = privilegeSetConfigureComponents;
const static auto& postApplicationCollection = privilegeSetConfigureComponents;

// Assembly
// 组件装配（Assembly）
const static auto& getAssembly = privilegeSetLogin;
const static auto& headAssembly = privilegeSetLogin;
const static auto& patchAssembly = privilegeSetConfigureComponents;
const static auto& putAssembly = privilegeSetConfigureComponents;
const static auto& deleteAssembly = privilegeSetConfigureComponents;
const static auto& postAssembly = privilegeSetConfigureComponents;

// AttributeRegistry
// 属性注册表（AttributeRegistry）
const static auto& getAttributeRegistry = privilegeSetLogin;
const static auto& headAttributeRegistry = privilegeSetLogin;
const static auto& patchAttributeRegistry = privilegeSetConfigureManager;
const static auto& putAttributeRegistry = privilegeSetConfigureManager;
const static auto& deleteAttributeRegistry = privilegeSetConfigureManager;
const static auto& postAttributeRegistry = privilegeSetConfigureManager;

// Battery
// 电池（Battery）
const static auto& getBattery = privilegeSetLogin;
const static auto& headBattery = privilegeSetLogin;
const static auto& patchBattery = privilegeSetConfigureManager;
const static auto& putBattery = privilegeSetConfigureManager;
const static auto& deleteBattery = privilegeSetConfigureManager;
const static auto& postBattery = privilegeSetConfigureManager;

// BatteryCollection
// 电池集合（BatteryCollection）
const static auto& getBatteryCollection = privilegeSetLogin;
const static auto& headBatteryCollection = privilegeSetLogin;
const static auto& patchBatteryCollection = privilegeSetConfigureManager;
const static auto& putBatteryCollection = privilegeSetConfigureManager;
const static auto& deleteBatteryCollection = privilegeSetConfigureManager;
const static auto& postBatteryCollection = privilegeSetConfigureManager;

// BatteryMetrics
// 电池指标（BatteryMetrics）
const static auto& getBatteryMetrics = privilegeSetLogin;
const static auto& headBatteryMetrics = privilegeSetLogin;
const static auto& patchBatteryMetrics = privilegeSetConfigureManager;
const static auto& putBatteryMetrics = privilegeSetConfigureManager;
const static auto& deleteBatteryMetrics = privilegeSetConfigureManager;
const static auto& postBatteryMetrics = privilegeSetConfigureManager;

// Bios
// 基本输入输出系统（BIOS）（Bios）
const static auto& getBios = privilegeSetLogin;
const static auto& headBios = privilegeSetLogin;
const static auto& patchBios = privilegeSetConfigureComponents;
const static auto& postBios = privilegeSetConfigureComponents;
const static auto& putBios = privilegeSetConfigureComponents;
const static auto& deleteBios = privilegeSetConfigureComponents;

// BootOption
// 启动选项（BootOption）
const static auto& getBootOption = privilegeSetLogin;
const static auto& headBootOption = privilegeSetLogin;
const static auto& patchBootOption = privilegeSetConfigureComponents;
const static auto& putBootOption = privilegeSetConfigureComponents;
const static auto& deleteBootOption = privilegeSetConfigureComponents;
const static auto& postBootOption = privilegeSetConfigureComponents;

// BootOptionCollection
// 启动选项集合（BootOptionCollection）
const static auto& getBootOptionCollection = privilegeSetLogin;
const static auto& headBootOptionCollection = privilegeSetLogin;
const static auto& patchBootOptionCollection = privilegeSetConfigureComponents;
const static auto& putBootOptionCollection = privilegeSetConfigureComponents;
const static auto& deleteBootOptionCollection = privilegeSetConfigureComponents;
const static auto& postBootOptionCollection = privilegeSetConfigureComponents;

// Cable
// 线缆（Cable）
const static auto& getCable = privilegeSetLogin;
const static auto& headCable = privilegeSetLogin;
const static auto& patchCable = privilegeSetConfigureComponents;
const static auto& putCable = privilegeSetConfigureComponents;
const static auto& deleteCable = privilegeSetConfigureComponents;
const static auto& postCable = privilegeSetConfigureComponents;

// CableCollection
// 线缆集合（CableCollection）
const static auto& getCableCollection = privilegeSetLogin;
const static auto& headCableCollection = privilegeSetLogin;
const static auto& patchCableCollection = privilegeSetConfigureComponents;
const static auto& putCableCollection = privilegeSetConfigureComponents;
const static auto& deleteCableCollection = privilegeSetConfigureComponents;
const static auto& postCableCollection = privilegeSetConfigureComponents;

// Certificate
// 证书（Certificate）
const static auto& getCertificate = privilegeSetConfigureManager;
const static auto& headCertificate = privilegeSetConfigureManager;
const static auto& patchCertificate = privilegeSetConfigureManager;
const static auto& putCertificate = privilegeSetConfigureManager;
const static auto& deleteCertificate = privilegeSetConfigureManager;
const static auto& postCertificate = privilegeSetConfigureManager;

// Subordinate override for ComputerSystem -> Certificate
// ComputerSystem -> Certificate 的下级覆盖（subordinate override）
const static auto& getCertificateSubOverComputerSystem = privilegeSetConfigureComponents;
const static auto& headCertificateSubOverComputerSystem = privilegeSetConfigureComponents;
const static auto& patchCertificateSubOverComputerSystem = privilegeSetConfigureComponents;
const static auto& putCertificateSubOverComputerSystem = privilegeSetConfigureComponents;
const static auto& deleteCertificateSubOverComputerSystem = privilegeSetConfigureComponents;
const static auto& postCertificateSubOverComputerSystem = privilegeSetConfigureComponents;

// CertificateCollection
// 证书集合（CertificateCollection）
const static auto& getCertificateCollection = privilegeSetConfigureManager;
const static auto& headCertificateCollection = privilegeSetConfigureManager;
const static auto& patchCertificateCollection = privilegeSetConfigureManager;
const static auto& putCertificateCollection = privilegeSetConfigureManager;
const static auto& deleteCertificateCollection = privilegeSetConfigureManager;
const static auto& postCertificateCollection = privilegeSetConfigureManager;

// Subordinate override for ComputerSystem -> CertificateCollection
// ComputerSystem -> CertificateCollection 的下级覆盖（subordinate override）
const static auto& getCertificateCollectionSubOverComputerSystem = privilegeSetConfigureComponents;
const static auto& headCertificateCollectionSubOverComputerSystem = privilegeSetConfigureComponents;
const static auto& patchCertificateCollectionSubOverComputerSystem = privilegeSetConfigureComponents;
const static auto& putCertificateCollectionSubOverComputerSystem = privilegeSetConfigureComponents;
const static auto& deleteCertificateCollectionSubOverComputerSystem = privilegeSetConfigureComponents;
const static auto& postCertificateCollectionSubOverComputerSystem = privilegeSetConfigureComponents;

// CertificateLocations
// 证书位置（CertificateLocations）
const static auto& getCertificateLocations = privilegeSetConfigureManager;
const static auto& headCertificateLocations = privilegeSetConfigureManager;
const static auto& patchCertificateLocations = privilegeSetConfigureManager;
const static auto& putCertificateLocations = privilegeSetConfigureManager;
const static auto& deleteCertificateLocations = privilegeSetConfigureManager;
const static auto& postCertificateLocations = privilegeSetConfigureManager;

// CertificateService
// 证书服务（CertificateService）
const static auto& getCertificateService = privilegeSetLogin;
const static auto& headCertificateService = privilegeSetLogin;
const static auto& patchCertificateService = privilegeSetConfigureManager;
const static auto& putCertificateService = privilegeSetConfigureManager;
const static auto& deleteCertificateService = privilegeSetConfigureManager;
const static auto& postCertificateService = privilegeSetConfigureManager;

// Chassis
// 机箱（Chassis）
const static auto& getChassis = privilegeSetLogin;
const static auto& headChassis = privilegeSetLogin;
const static auto& patchChassis = privilegeSetConfigureComponents;
const static auto& putChassis = privilegeSetConfigureComponents;
const static auto& deleteChassis = privilegeSetConfigureComponents;
const static auto& postChassis = privilegeSetConfigureComponents;

// ChassisCollection
// 机箱集合（ChassisCollection）
const static auto& getChassisCollection = privilegeSetLogin;
const static auto& headChassisCollection = privilegeSetLogin;
const static auto& patchChassisCollection = privilegeSetConfigureComponents;
const static auto& putChassisCollection = privilegeSetConfigureComponents;
const static auto& deleteChassisCollection = privilegeSetConfigureComponents;
const static auto& postChassisCollection = privilegeSetConfigureComponents;

// Circuit
// 电路（Circuit）
const static auto& getCircuit = privilegeSetLogin;
const static auto& headCircuit = privilegeSetLogin;
const static auto& patchCircuit = privilegeSetConfigureComponents;
const static auto& putCircuit = privilegeSetConfigureComponents;
const static auto& deleteCircuit = privilegeSetConfigureComponents;
const static auto& postCircuit = privilegeSetConfigureComponents;

// CircuitCollection
// 电路集合（CircuitCollection）
const static auto& getCircuitCollection = privilegeSetLogin;
const static auto& headCircuitCollection = privilegeSetLogin;
const static auto& patchCircuitCollection = privilegeSetConfigureComponents;
const static auto& putCircuitCollection = privilegeSetConfigureComponents;
const static auto& deleteCircuitCollection = privilegeSetConfigureComponents;
const static auto& postCircuitCollection = privilegeSetConfigureComponents;

// ComponentIntegrity
// 组件完整性（ComponentIntegrity）
const static auto& getComponentIntegrity = privilegeSetLogin;
const static auto& headComponentIntegrity = privilegeSetLogin;
const static auto& patchComponentIntegrity = privilegeSetConfigureManager;
const static auto& putComponentIntegrity = privilegeSetConfigureManager;
const static auto& deleteComponentIntegrity = privilegeSetConfigureManager;
const static auto& postComponentIntegrity = privilegeSetConfigureManager;

// ComponentIntegrityCollection
// 组件完整性集合（ComponentIntegrityCollection）
const static auto& getComponentIntegrityCollection = privilegeSetLogin;
const static auto& headComponentIntegrityCollection = privilegeSetLogin;
const static auto& patchComponentIntegrityCollection = privilegeSetConfigureManager;
const static auto& putComponentIntegrityCollection = privilegeSetConfigureManager;
const static auto& deleteComponentIntegrityCollection = privilegeSetConfigureManager;
const static auto& postComponentIntegrityCollection = privilegeSetConfigureManager;

// CompositionReservation
// 组合预留（CompositionReservation）
const static auto& getCompositionReservation = privilegeSetLogin;
const static auto& headCompositionReservation = privilegeSetLogin;
const static auto& patchCompositionReservation = privilegeSetConfigureManager;
const static auto& putCompositionReservation = privilegeSetConfigureManager;
const static auto& deleteCompositionReservation = privilegeSetConfigureManager;
const static auto& postCompositionReservation = privilegeSetConfigureManager;

// CompositionReservationCollection
// 组合预留集合（CompositionReservationCollection）
const static auto& getCompositionReservationCollection = privilegeSetLogin;
const static auto& headCompositionReservationCollection = privilegeSetLogin;
const static auto& patchCompositionReservationCollection = privilegeSetConfigureManager;
const static auto& putCompositionReservationCollection = privilegeSetConfigureManager;
const static auto& deleteCompositionReservationCollection = privilegeSetConfigureManager;
const static auto& postCompositionReservationCollection = privilegeSetConfigureManager;

// CompositionService
// 组合服务（CompositionService）
const static auto& getCompositionService = privilegeSetLogin;
const static auto& headCompositionService = privilegeSetLogin;
const static auto& patchCompositionService = privilegeSetConfigureManager;
const static auto& putCompositionService = privilegeSetConfigureManager;
const static auto& deleteCompositionService = privilegeSetConfigureManager;
const static auto& postCompositionService = privilegeSetConfigureManager;

// ComputerSystem
// 计算机系统（ComputerSystem）
const static auto& getComputerSystem = privilegeSetLogin;
const static auto& headComputerSystem = privilegeSetLogin;
const static auto& patchComputerSystem = privilegeSetConfigureComponents;
const static auto& postComputerSystem = privilegeSetConfigureComponents;
const static auto& putComputerSystem = privilegeSetConfigureComponents;
const static auto& deleteComputerSystem = privilegeSetConfigureComponents;

// ComputerSystemCollection
// 计算机系统集合（ComputerSystemCollection）
const static auto& getComputerSystemCollection = privilegeSetLogin;
const static auto& headComputerSystemCollection = privilegeSetLogin;
const static auto& patchComputerSystemCollection = privilegeSetConfigureComponents;
const static auto& postComputerSystemCollection = privilegeSetConfigureComponents;
const static auto& putComputerSystemCollection = privilegeSetConfigureComponents;
const static auto& deleteComputerSystemCollection = privilegeSetConfigureComponents;

// Connection
// 连接（Connection）
const static auto& getConnection = privilegeSetLogin;
const static auto& headConnection = privilegeSetLogin;
const static auto& patchConnection = privilegeSetConfigureComponents;
const static auto& postConnection = privilegeSetConfigureComponents;
const static auto& putConnection = privilegeSetConfigureComponents;
const static auto& deleteConnection = privilegeSetConfigureComponents;

// ConnectionCollection
// 连接集合（ConnectionCollection）
const static auto& getConnectionCollection = privilegeSetLogin;
const static auto& headConnectionCollection = privilegeSetLogin;
const static auto& patchConnectionCollection = privilegeSetConfigureComponents;
const static auto& postConnectionCollection = privilegeSetConfigureComponents;
const static auto& putConnectionCollection = privilegeSetConfigureComponents;
const static auto& deleteConnectionCollection = privilegeSetConfigureComponents;

// ConnectionMethod
// 连接方式（ConnectionMethod）
const static auto& getConnectionMethod = privilegeSetLogin;
const static auto& headConnectionMethod = privilegeSetLogin;
const static auto& patchConnectionMethod = privilegeSetConfigureManager;
const static auto& putConnectionMethod = privilegeSetConfigureManager;
const static auto& deleteConnectionMethod = privilegeSetConfigureManager;
const static auto& postConnectionMethod = privilegeSetConfigureManager;

// ConnectionMethodCollection
// 连接方式集合（ConnectionMethodCollection）
const static auto& getConnectionMethodCollection = privilegeSetLogin;
const static auto& headConnectionMethodCollection = privilegeSetLogin;
const static auto& patchConnectionMethodCollection = privilegeSetConfigureManager;
const static auto& putConnectionMethodCollection = privilegeSetConfigureManager;
const static auto& deleteConnectionMethodCollection = privilegeSetConfigureManager;
const static auto& postConnectionMethodCollection = privilegeSetConfigureManager;

// Container
// 容器（Container）
const static auto& getContainer = privilegeSetLogin;
const static auto& headContainer = privilegeSetLogin;
const static auto& patchContainer = privilegeSetConfigureComponents;
const static auto& putContainer = privilegeSetConfigureComponents;
const static auto& deleteContainer = privilegeSetConfigureComponents;
const static auto& postContainer = privilegeSetConfigureComponents;

// ContainerCollection
// 容器集合（ContainerCollection）
const static auto& getContainerCollection = privilegeSetLogin;
const static auto& headContainerCollection = privilegeSetLogin;
const static auto& patchContainerCollection = privilegeSetConfigureComponents;
const static auto& putContainerCollection = privilegeSetConfigureComponents;
const static auto& deleteContainerCollection = privilegeSetConfigureComponents;
const static auto& postContainerCollection = privilegeSetConfigureComponents;

// ContainerImage
// 容器镜像（ContainerImage）
const static auto& getContainerImage = privilegeSetLogin;
const static auto& headContainerImage = privilegeSetLogin;
const static auto& patchContainerImage = privilegeSetConfigureComponents;
const static auto& putContainerImage = privilegeSetConfigureComponents;
const static auto& deleteContainerImage = privilegeSetConfigureComponents;
const static auto& postContainerImage = privilegeSetConfigureComponents;

// ContainerImageCollection
// 容器镜像集合（ContainerImageCollection）
const static auto& getContainerImageCollection = privilegeSetLogin;
const static auto& headContainerImageCollection = privilegeSetLogin;
const static auto& patchContainerImageCollection = privilegeSetConfigureComponents;
const static auto& putContainerImageCollection = privilegeSetConfigureComponents;
const static auto& deleteContainerImageCollection = privilegeSetConfigureComponents;
const static auto& postContainerImageCollection = privilegeSetConfigureComponents;

// Control
// 控制（Control）
const static auto& getControl = privilegeSetLogin;
const static auto& headControl = privilegeSetLogin;
const static auto& patchControl = privilegeSetConfigureManager;
const static auto& putControl = privilegeSetConfigureManager;
const static auto& deleteControl = privilegeSetConfigureManager;
const static auto& postControl = privilegeSetConfigureManager;

// ControlCollection
// 控制集合（ControlCollection）
const static auto& getControlCollection = privilegeSetLogin;
const static auto& headControlCollection = privilegeSetLogin;
const static auto& patchControlCollection = privilegeSetConfigureManager;
const static auto& putControlCollection = privilegeSetConfigureManager;
const static auto& deleteControlCollection = privilegeSetConfigureManager;
const static auto& postControlCollection = privilegeSetConfigureManager;

// CoolantConnector
// 冷却液连接器（CoolantConnector）
const static auto& getCoolantConnector = privilegeSetLogin;
const static auto& headCoolantConnector = privilegeSetLogin;
const static auto& patchCoolantConnector = privilegeSetConfigureComponents;
const static auto& putCoolantConnector = privilegeSetConfigureComponents;
const static auto& deleteCoolantConnector = privilegeSetConfigureComponents;
const static auto& postCoolantConnector = privilegeSetConfigureComponents;

// CoolantConnectorCollection
// 冷却液连接器集合（CoolantConnectorCollection）
const static auto& getCoolantConnectorCollection = privilegeSetLogin;
const static auto& headCoolantConnectorCollection = privilegeSetLogin;
const static auto& patchCoolantConnectorCollection = privilegeSetConfigureComponents;
const static auto& putCoolantConnectorCollection = privilegeSetConfigureComponents;
const static auto& deleteCoolantConnectorCollection = privilegeSetConfigureComponents;
const static auto& postCoolantConnectorCollection = privilegeSetConfigureComponents;

// CoolingLoop
// 冷却回路（CoolingLoop）
const static auto& getCoolingLoop = privilegeSetLogin;
const static auto& headCoolingLoop = privilegeSetLogin;
const static auto& patchCoolingLoop = privilegeSetConfigureComponents;
const static auto& putCoolingLoop = privilegeSetConfigureComponents;
const static auto& deleteCoolingLoop = privilegeSetConfigureComponents;
const static auto& postCoolingLoop = privilegeSetConfigureComponents;

// CoolingLoopCollection
// 冷却回路集合（CoolingLoopCollection）
const static auto& getCoolingLoopCollection = privilegeSetLogin;
const static auto& headCoolingLoopCollection = privilegeSetLogin;
const static auto& patchCoolingLoopCollection = privilegeSetConfigureComponents;
const static auto& putCoolingLoopCollection = privilegeSetConfigureComponents;
const static auto& deleteCoolingLoopCollection = privilegeSetConfigureComponents;
const static auto& postCoolingLoopCollection = privilegeSetConfigureComponents;

// CoolingUnit
// 冷却单元（CoolingUnit）
const static auto& getCoolingUnit = privilegeSetLogin;
const static auto& headCoolingUnit = privilegeSetLogin;
const static auto& patchCoolingUnit = privilegeSetConfigureComponents;
const static auto& putCoolingUnit = privilegeSetConfigureComponents;
const static auto& deleteCoolingUnit = privilegeSetConfigureComponents;
const static auto& postCoolingUnit = privilegeSetConfigureComponents;

// CoolingUnitCollection
// 冷却单元集合（CoolingUnitCollection）
const static auto& getCoolingUnitCollection = privilegeSetLogin;
const static auto& headCoolingUnitCollection = privilegeSetLogin;
const static auto& patchCoolingUnitCollection = privilegeSetConfigureComponents;
const static auto& putCoolingUnitCollection = privilegeSetConfigureComponents;
const static auto& deleteCoolingUnitCollection = privilegeSetConfigureComponents;
const static auto& postCoolingUnitCollection = privilegeSetConfigureComponents;

// CXLLogicalDevice
// CXL 逻辑设备（CXLLogicalDevice）
const static auto& getCXLLogicalDevice = privilegeSetLogin;
const static auto& headCXLLogicalDevice = privilegeSetLogin;
const static auto& patchCXLLogicalDevice = privilegeSetConfigureComponents;
const static auto& putCXLLogicalDevice = privilegeSetConfigureComponents;
const static auto& deleteCXLLogicalDevice = privilegeSetConfigureComponents;
const static auto& postCXLLogicalDevice = privilegeSetConfigureComponents;

// CXLLogicalDeviceCollection
// CXL 逻辑设备集合（CXLLogicalDeviceCollection）
const static auto& getCXLLogicalDeviceCollection = privilegeSetLogin;
const static auto& headCXLLogicalDeviceCollection = privilegeSetLogin;
const static auto& patchCXLLogicalDeviceCollection = privilegeSetConfigureComponents;
const static auto& putCXLLogicalDeviceCollection = privilegeSetConfigureComponents;
const static auto& deleteCXLLogicalDeviceCollection = privilegeSetConfigureComponents;
const static auto& postCXLLogicalDeviceCollection = privilegeSetConfigureComponents;

// Drive
// 驱动器（Drive）
const static auto& getDrive = privilegeSetLogin;
const static auto& headDrive = privilegeSetLogin;
const static auto& patchDrive = privilegeSetConfigureComponents;
const static auto& postDrive = privilegeSetConfigureComponents;
const static auto& putDrive = privilegeSetConfigureComponents;
const static auto& deleteDrive = privilegeSetConfigureComponents;

// DriveCollection
// 驱动器集合（DriveCollection）
const static auto& getDriveCollection = privilegeSetLogin;
const static auto& headDriveCollection = privilegeSetLogin;
const static auto& patchDriveCollection = privilegeSetConfigureComponents;
const static auto& postDriveCollection = privilegeSetConfigureComponents;
const static auto& putDriveCollection = privilegeSetConfigureComponents;
const static auto& deleteDriveCollection = privilegeSetConfigureComponents;

// DriveMetrics
// 驱动器指标（DriveMetrics）
const static auto& getDriveMetrics = privilegeSetLogin;
const static auto& headDriveMetrics = privilegeSetLogin;
const static auto& patchDriveMetrics = privilegeSetConfigureComponents;
const static auto& putDriveMetrics = privilegeSetConfigureComponents;
const static auto& deleteDriveMetrics = privilegeSetConfigureComponents;
const static auto& postDriveMetrics = privilegeSetConfigureComponents;

// Endpoint
// 端点（Endpoint）
const static auto& getEndpoint = privilegeSetLogin;
const static auto& headEndpoint = privilegeSetLogin;
const static auto& patchEndpoint = privilegeSetConfigureComponents;
const static auto& postEndpoint = privilegeSetConfigureComponents;
const static auto& putEndpoint = privilegeSetConfigureComponents;
const static auto& deleteEndpoint = privilegeSetConfigureComponents;

// EndpointCollection
// 端点集合（EndpointCollection）
const static auto& getEndpointCollection = privilegeSetLogin;
const static auto& headEndpointCollection = privilegeSetLogin;
const static auto& patchEndpointCollection = privilegeSetConfigureComponents;
const static auto& postEndpointCollection = privilegeSetConfigureComponents;
const static auto& putEndpointCollection = privilegeSetConfigureComponents;
const static auto& deleteEndpointCollection = privilegeSetConfigureComponents;

// EndpointGroup
// 端点组（EndpointGroup）
const static auto& getEndpointGroup = privilegeSetLogin;
const static auto& headEndpointGroup = privilegeSetLogin;
const static auto& patchEndpointGroup = privilegeSetConfigureComponents;
const static auto& postEndpointGroup = privilegeSetConfigureComponents;
const static auto& putEndpointGroup = privilegeSetConfigureComponents;
const static auto& deleteEndpointGroup = privilegeSetConfigureComponents;

// EndpointGroupCollection
// 端点组集合（EndpointGroupCollection）
const static auto& getEndpointGroupCollection = privilegeSetLogin;
const static auto& headEndpointGroupCollection = privilegeSetLogin;
const static auto& patchEndpointGroupCollection = privilegeSetConfigureComponents;
const static auto& postEndpointGroupCollection = privilegeSetConfigureComponents;
const static auto& putEndpointGroupCollection = privilegeSetConfigureComponents;
const static auto& deleteEndpointGroupCollection = privilegeSetConfigureComponents;

// EnvironmentMetrics
// 环境指标（EnvironmentMetrics）
const static auto& getEnvironmentMetrics = privilegeSetLogin;
const static auto& headEnvironmentMetrics = privilegeSetLogin;
const static auto& patchEnvironmentMetrics = privilegeSetConfigureManager;
const static auto& putEnvironmentMetrics = privilegeSetConfigureManager;
const static auto& deleteEnvironmentMetrics = privilegeSetConfigureManager;
const static auto& postEnvironmentMetrics = privilegeSetConfigureManager;

// Subordinate override for Processor -> EnvironmentMetrics
// Processor -> EnvironmentMetrics 的下级覆盖（subordinate override）
const static auto& patchEnvironmentMetricsSubOverProcessor = privilegeSetConfigureComponents;
const static auto& putEnvironmentMetricsSubOverProcessor = privilegeSetConfigureComponents;
const static auto& deleteEnvironmentMetricsSubOverProcessor = privilegeSetConfigureComponents;
const static auto& postEnvironmentMetricsSubOverProcessor = privilegeSetConfigureComponents;

// Subordinate override for Memory -> EnvironmentMetrics
// Memory -> EnvironmentMetrics 的下级覆盖（subordinate override）
const static auto& patchEnvironmentMetricsSubOverMemory = privilegeSetConfigureComponents;
const static auto& putEnvironmentMetricsSubOverMemory = privilegeSetConfigureComponents;
const static auto& deleteEnvironmentMetricsSubOverMemory = privilegeSetConfigureComponents;
const static auto& postEnvironmentMetricsSubOverMemory = privilegeSetConfigureComponents;

// Subordinate override for Drive -> EnvironmentMetrics
// Drive -> EnvironmentMetrics 的下级覆盖（subordinate override）
const static auto& patchEnvironmentMetricsSubOverDrive = privilegeSetConfigureComponents;
const static auto& putEnvironmentMetricsSubOverDrive = privilegeSetConfigureComponents;
const static auto& deleteEnvironmentMetricsSubOverDrive = privilegeSetConfigureComponents;
const static auto& postEnvironmentMetricsSubOverDrive = privilegeSetConfigureComponents;

// Subordinate override for PCIeDevice -> EnvironmentMetrics
// PCIeDevice -> EnvironmentMetrics 的下级覆盖（subordinate override）
const static auto& patchEnvironmentMetricsSubOverPCIeDevice = privilegeSetConfigureComponents;
const static auto& putEnvironmentMetricsSubOverPCIeDevice = privilegeSetConfigureComponents;
const static auto& deleteEnvironmentMetricsSubOverPCIeDevice = privilegeSetConfigureComponents;
const static auto& postEnvironmentMetricsSubOverPCIeDevice = privilegeSetConfigureComponents;

// Subordinate override for StorageController -> EnvironmentMetrics
// StorageController -> EnvironmentMetrics 的下级覆盖（subordinate override）
const static auto& patchEnvironmentMetricsSubOverStorageController = privilegeSetConfigureComponents;
const static auto& putEnvironmentMetricsSubOverStorageController = privilegeSetConfigureComponents;
const static auto& deleteEnvironmentMetricsSubOverStorageController = privilegeSetConfigureComponents;
const static auto& postEnvironmentMetricsSubOverStorageController = privilegeSetConfigureComponents;

// Subordinate override for Port -> EnvironmentMetrics
// Port -> EnvironmentMetrics 的下级覆盖（subordinate override）
const static auto& patchEnvironmentMetricsSubOverPort = privilegeSetConfigureComponents;
const static auto& putEnvironmentMetricsSubOverPort = privilegeSetConfigureComponents;
const static auto& deleteEnvironmentMetricsSubOverPort = privilegeSetConfigureComponents;
const static auto& postEnvironmentMetricsSubOverPort = privilegeSetConfigureComponents;

// EthernetInterface
// 以太网接口（EthernetInterface）
const static auto& getEthernetInterface = privilegeSetLogin;
const static auto& headEthernetInterface = privilegeSetLogin;
const static auto& patchEthernetInterface = privilegeSetConfigureComponents;
const static auto& postEthernetInterface = privilegeSetConfigureComponents;
const static auto& putEthernetInterface = privilegeSetConfigureComponents;
const static auto& deleteEthernetInterface = privilegeSetConfigureComponents;

// Subordinate override for Manager -> EthernetInterfaceCollection -> EthernetInterface
// Manager -> EthernetInterfaceCollection -> EthernetInterface 的下级覆盖（subordinate override）
const static auto& patchEthernetInterfaceSubOverManagerEthernetInterfaceCollection = privilegeSetConfigureManager;
const static auto& postEthernetInterfaceSubOverManagerEthernetInterfaceCollection = privilegeSetConfigureManager;
const static auto& putEthernetInterfaceSubOverManagerEthernetInterfaceCollection = privilegeSetConfigureManager;
const static auto& deleteEthernetInterfaceSubOverManagerEthernetInterfaceCollection = privilegeSetConfigureManager;

// EthernetInterfaceCollection
// 以太网接口集合（EthernetInterfaceCollection）
const static auto& getEthernetInterfaceCollection = privilegeSetLogin;
const static auto& headEthernetInterfaceCollection = privilegeSetLogin;
const static auto& patchEthernetInterfaceCollection = privilegeSetConfigureComponents;
const static auto& postEthernetInterfaceCollection = privilegeSetConfigureComponents;
const static auto& putEthernetInterfaceCollection = privilegeSetConfigureComponents;
const static auto& deleteEthernetInterfaceCollection = privilegeSetConfigureComponents;

// EventDestination
// 事件目标（EventDestination）
const static auto& getEventDestination = privilegeSetLogin;
const static auto& headEventDestination = privilegeSetLogin;
const static auto& patchEventDestination = privilegeSetConfigureManagerOrConfigureSelf;
const static auto& postEventDestination = privilegeSetConfigureManagerOrConfigureSelf;
const static auto& putEventDestination = privilegeSetConfigureManagerOrConfigureSelf;
const static auto& deleteEventDestination = privilegeSetConfigureManagerOrConfigureSelf;

// EventDestinationCollection
// 事件目标集合（EventDestinationCollection）
const static auto& getEventDestinationCollection = privilegeSetLogin;
const static auto& headEventDestinationCollection = privilegeSetLogin;
const static auto& patchEventDestinationCollection = privilegeSetConfigureManagerOrConfigureComponents;
const static auto& postEventDestinationCollection = privilegeSetConfigureManagerOrConfigureComponents;
const static auto& putEventDestinationCollection = privilegeSetConfigureManagerOrConfigureComponents;
const static auto& deleteEventDestinationCollection = privilegeSetConfigureManagerOrConfigureComponents;

// EventService
// 事件服务（EventService）
const static auto& getEventService = privilegeSetLogin;
const static auto& headEventService = privilegeSetLogin;
const static auto& patchEventService = privilegeSetConfigureManager;
const static auto& postEventService = privilegeSetConfigureManager;
const static auto& putEventService = privilegeSetConfigureManager;
const static auto& deleteEventService = privilegeSetConfigureManager;

// ExternalAccountProvider
// 外部账户提供者（ExternalAccountProvider）
const static auto& getExternalAccountProvider = privilegeSetLogin;
const static auto& headExternalAccountProvider = privilegeSetLogin;
const static auto& patchExternalAccountProvider = privilegeSetConfigureManager;
const static auto& putExternalAccountProvider = privilegeSetConfigureManager;
const static auto& deleteExternalAccountProvider = privilegeSetConfigureManager;
const static auto& postExternalAccountProvider = privilegeSetConfigureManager;

// ExternalAccountProviderCollection
// 外部账户提供者集合（ExternalAccountProviderCollection）
const static auto& getExternalAccountProviderCollection = privilegeSetLogin;
const static auto& headExternalAccountProviderCollection = privilegeSetLogin;
const static auto& patchExternalAccountProviderCollection = privilegeSetConfigureManager;
const static auto& putExternalAccountProviderCollection = privilegeSetConfigureManager;
const static auto& deleteExternalAccountProviderCollection = privilegeSetConfigureManager;
const static auto& postExternalAccountProviderCollection = privilegeSetConfigureManager;

// Fabric
// 结构（Fabric）（Fabric）
const static auto& getFabric = privilegeSetLogin;
const static auto& headFabric = privilegeSetLogin;
const static auto& patchFabric = privilegeSetConfigureComponents;
const static auto& postFabric = privilegeSetConfigureComponents;
const static auto& putFabric = privilegeSetConfigureComponents;
const static auto& deleteFabric = privilegeSetConfigureComponents;

// FabricCollection
// 结构（Fabric）集合（FabricCollection）
const static auto& getFabricCollection = privilegeSetLogin;
const static auto& headFabricCollection = privilegeSetLogin;
const static auto& patchFabricCollection = privilegeSetConfigureComponents;
const static auto& postFabricCollection = privilegeSetConfigureComponents;
const static auto& putFabricCollection = privilegeSetConfigureComponents;
const static auto& deleteFabricCollection = privilegeSetConfigureComponents;

// FabricAdapter
// 结构（Fabric）适配器（FabricAdapter）
const static auto& getFabricAdapter = privilegeSetLogin;
const static auto& headFabricAdapter = privilegeSetLogin;
const static auto& patchFabricAdapter = privilegeSetConfigureComponents;
const static auto& postFabricAdapter = privilegeSetConfigureComponents;
const static auto& putFabricAdapter = privilegeSetConfigureComponents;
const static auto& deleteFabricAdapter = privilegeSetConfigureComponents;

// FabricAdapterCollection
// 结构（Fabric）适配器集合（FabricAdapterCollection）
const static auto& getFabricAdapterCollection = privilegeSetLogin;
const static auto& headFabricAdapterCollection = privilegeSetLogin;
const static auto& patchFabricAdapterCollection = privilegeSetConfigureComponents;
const static auto& postFabricAdapterCollection = privilegeSetConfigureComponents;
const static auto& putFabricAdapterCollection = privilegeSetConfigureComponents;
const static auto& deleteFabricAdapterCollection = privilegeSetConfigureComponents;

// Facility
// 设施（Facility）
const static auto& getFacility = privilegeSetLogin;
const static auto& headFacility = privilegeSetLogin;
const static auto& patchFacility = privilegeSetConfigureComponents;
const static auto& putFacility = privilegeSetConfigureComponents;
const static auto& deleteFacility = privilegeSetConfigureComponents;
const static auto& postFacility = privilegeSetConfigureComponents;

// FacilityCollection
// 设施集合（FacilityCollection）
const static auto& getFacilityCollection = privilegeSetLogin;
const static auto& headFacilityCollection = privilegeSetLogin;
const static auto& patchFacilityCollection = privilegeSetConfigureComponents;
const static auto& putFacilityCollection = privilegeSetConfigureComponents;
const static auto& deleteFacilityCollection = privilegeSetConfigureComponents;
const static auto& postFacilityCollection = privilegeSetConfigureComponents;

// Fan
// 风扇（Fan）
const static auto& getFan = privilegeSetLogin;
const static auto& headFan = privilegeSetLogin;
const static auto& patchFan = privilegeSetConfigureManager;
const static auto& putFan = privilegeSetConfigureManager;
const static auto& deleteFan = privilegeSetConfigureManager;
const static auto& postFan = privilegeSetConfigureManager;

// FanCollection
// 风扇集合（FanCollection）
const static auto& getFanCollection = privilegeSetLogin;
const static auto& headFanCollection = privilegeSetLogin;
const static auto& patchFanCollection = privilegeSetConfigureManager;
const static auto& putFanCollection = privilegeSetConfigureManager;
const static auto& deleteFanCollection = privilegeSetConfigureManager;
const static auto& postFanCollection = privilegeSetConfigureManager;

// Filter
// 过滤器（Filter）
const static auto& getFilter = privilegeSetLogin;
const static auto& headFilter = privilegeSetLogin;
const static auto& patchFilter = privilegeSetConfigureComponents;
const static auto& putFilter = privilegeSetConfigureComponents;
const static auto& deleteFilter = privilegeSetConfigureComponents;
const static auto& postFilter = privilegeSetConfigureComponents;

// FilterCollection
// 过滤器集合（FilterCollection）
const static auto& getFilterCollection = privilegeSetLogin;
const static auto& headFilterCollection = privilegeSetLogin;
const static auto& patchFilterCollection = privilegeSetConfigureComponents;
const static auto& putFilterCollection = privilegeSetConfigureComponents;
const static auto& deleteFilterCollection = privilegeSetConfigureComponents;
const static auto& postFilterCollection = privilegeSetConfigureComponents;

// GraphicsController
// 图形控制器（GraphicsController）
const static auto& getGraphicsController = privilegeSetLogin;
const static auto& headGraphicsController = privilegeSetLogin;
const static auto& patchGraphicsController = privilegeSetConfigureComponents;
const static auto& putGraphicsController = privilegeSetConfigureComponents;
const static auto& deleteGraphicsController = privilegeSetConfigureComponents;
const static auto& postGraphicsController = privilegeSetConfigureComponents;

// GraphicsControllerCollection
// 图形控制器集合（GraphicsControllerCollection）
const static auto& getGraphicsControllerCollection = privilegeSetLogin;
const static auto& headGraphicsControllerCollection = privilegeSetLogin;
const static auto& patchGraphicsControllerCollection = privilegeSetConfigureComponents;
const static auto& putGraphicsControllerCollection = privilegeSetConfigureComponents;
const static auto& deleteGraphicsControllerCollection = privilegeSetConfigureComponents;
const static auto& postGraphicsControllerCollection = privilegeSetConfigureComponents;

// Heater
// 加热器（Heater）
const static auto& getHeater = privilegeSetLogin;
const static auto& headHeater = privilegeSetLogin;
const static auto& patchHeater = privilegeSetConfigureManager;
const static auto& putHeater = privilegeSetConfigureManager;
const static auto& deleteHeater = privilegeSetConfigureManager;
const static auto& postHeater = privilegeSetConfigureManager;

// HeaterCollection
// 加热器集合（HeaterCollection）
const static auto& getHeaterCollection = privilegeSetLogin;
const static auto& headHeaterCollection = privilegeSetLogin;
const static auto& patchHeaterCollection = privilegeSetConfigureManager;
const static auto& putHeaterCollection = privilegeSetConfigureManager;
const static auto& deleteHeaterCollection = privilegeSetConfigureManager;
const static auto& postHeaterCollection = privilegeSetConfigureManager;

// HeaterMetrics
// 加热器指标（HeaterMetrics）
const static auto& getHeaterMetrics = privilegeSetLogin;
const static auto& headHeaterMetrics = privilegeSetLogin;
const static auto& patchHeaterMetrics = privilegeSetConfigureManager;
const static auto& putHeaterMetrics = privilegeSetConfigureManager;
const static auto& deleteHeaterMetrics = privilegeSetConfigureManager;
const static auto& postHeaterMetrics = privilegeSetConfigureManager;

// HostInterface
// 主机接口（HostInterface）
const static auto& getHostInterface = privilegeSetLogin;
const static auto& headHostInterface = privilegeSetLogin;
const static auto& patchHostInterface = privilegeSetConfigureManager;
const static auto& postHostInterface = privilegeSetConfigureManager;
const static auto& putHostInterface = privilegeSetConfigureManager;
const static auto& deleteHostInterface = privilegeSetConfigureManager;

// HostInterfaceCollection
// 主机接口集合（HostInterfaceCollection）
const static auto& getHostInterfaceCollection = privilegeSetLogin;
const static auto& headHostInterfaceCollection = privilegeSetLogin;
const static auto& patchHostInterfaceCollection = privilegeSetConfigureManager;
const static auto& postHostInterfaceCollection = privilegeSetConfigureManager;
const static auto& putHostInterfaceCollection = privilegeSetConfigureManager;
const static auto& deleteHostInterfaceCollection = privilegeSetConfigureManager;

// Job
// 作业（Job）
const static auto& getJob = privilegeSetLogin;
const static auto& headJob = privilegeSetLogin;
const static auto& patchJob = privilegeSetConfigureManager;
const static auto& putJob = privilegeSetConfigureManager;
const static auto& deleteJob = privilegeSetConfigureManager;
const static auto& postJob = privilegeSetConfigureManager;

// JobCollection
// 作业集合（JobCollection）
const static auto& getJobCollection = privilegeSetLogin;
const static auto& headJobCollection = privilegeSetLogin;
const static auto& patchJobCollection = privilegeSetConfigureManager;
const static auto& putJobCollection = privilegeSetConfigureManager;
const static auto& deleteJobCollection = privilegeSetConfigureManager;
const static auto& postJobCollection = privilegeSetConfigureManager;

// JobService
// 作业服务（JobService）
const static auto& getJobService = privilegeSetLogin;
const static auto& headJobService = privilegeSetLogin;
const static auto& patchJobService = privilegeSetConfigureManager;
const static auto& putJobService = privilegeSetConfigureManager;
const static auto& deleteJobService = privilegeSetConfigureManager;
const static auto& postJobService = privilegeSetConfigureManager;

// JsonSchemaFile
// JSON Schema 文件（JsonSchemaFile）
const static auto& getJsonSchemaFile = privilegeSetLogin;
const static auto& headJsonSchemaFile = privilegeSetLogin;
const static auto& patchJsonSchemaFile = privilegeSetConfigureManager;
const static auto& postJsonSchemaFile = privilegeSetConfigureManager;
const static auto& putJsonSchemaFile = privilegeSetConfigureManager;
const static auto& deleteJsonSchemaFile = privilegeSetConfigureManager;

// JsonSchemaFileCollection
// JSON Schema 文件集合（JsonSchemaFileCollection）
const static auto& getJsonSchemaFileCollection = privilegeSetLogin;
const static auto& headJsonSchemaFileCollection = privilegeSetLogin;
const static auto& patchJsonSchemaFileCollection = privilegeSetConfigureManager;
const static auto& postJsonSchemaFileCollection = privilegeSetConfigureManager;
const static auto& putJsonSchemaFileCollection = privilegeSetConfigureManager;
const static auto& deleteJsonSchemaFileCollection = privilegeSetConfigureManager;

// Key
// 密钥（Key）
const static auto& getKey = privilegeSetLogin;
const static auto& headKey = privilegeSetLogin;
const static auto& patchKey = privilegeSetConfigureManager;
const static auto& putKey = privilegeSetConfigureManager;
const static auto& deleteKey = privilegeSetConfigureManager;
const static auto& postKey = privilegeSetConfigureManager;

// KeyCollection
// 密钥集合（KeyCollection）
const static auto& getKeyCollection = privilegeSetLogin;
const static auto& headKeyCollection = privilegeSetLogin;
const static auto& patchKeyCollection = privilegeSetConfigureManager;
const static auto& putKeyCollection = privilegeSetConfigureManager;
const static auto& deleteKeyCollection = privilegeSetConfigureManager;
const static auto& postKeyCollection = privilegeSetConfigureManager;

// KeyPolicy
// 密钥策略（KeyPolicy）
const static auto& getKeyPolicy = privilegeSetLogin;
const static auto& headKeyPolicy = privilegeSetLogin;
const static auto& patchKeyPolicy = privilegeSetConfigureManager;
const static auto& putKeyPolicy = privilegeSetConfigureManager;
const static auto& deleteKeyPolicy = privilegeSetConfigureManager;
const static auto& postKeyPolicy = privilegeSetConfigureManager;

// KeyPolicyCollection
// 密钥策略集合（KeyPolicyCollection）
const static auto& getKeyPolicyCollection = privilegeSetLogin;
const static auto& headKeyPolicyCollection = privilegeSetLogin;
const static auto& patchKeyPolicyCollection = privilegeSetConfigureManager;
const static auto& putKeyPolicyCollection = privilegeSetConfigureManager;
const static auto& deleteKeyPolicyCollection = privilegeSetConfigureManager;
const static auto& postKeyPolicyCollection = privilegeSetConfigureManager;

// KeyService
// 密钥服务（KeyService）
const static auto& getKeyService = privilegeSetLogin;
const static auto& headKeyService = privilegeSetLogin;
const static auto& patchKeyService = privilegeSetConfigureManager;
const static auto& putKeyService = privilegeSetConfigureManager;
const static auto& deleteKeyService = privilegeSetConfigureManager;
const static auto& postKeyService = privilegeSetConfigureManager;

// LeakDetection
// 漏水检测（LeakDetection）
const static auto& getLeakDetection = privilegeSetLogin;
const static auto& headLeakDetection = privilegeSetLogin;
const static auto& patchLeakDetection = privilegeSetConfigureComponents;
const static auto& postLeakDetection = privilegeSetConfigureComponents;
const static auto& putLeakDetection = privilegeSetConfigureComponents;
const static auto& deleteLeakDetection = privilegeSetConfigureComponents;

// LeakDetector
// 漏水检测器（LeakDetector）
const static auto& getLeakDetector = privilegeSetLogin;
const static auto& headLeakDetector = privilegeSetLogin;
const static auto& patchLeakDetector = privilegeSetConfigureComponents;
const static auto& postLeakDetector = privilegeSetConfigureComponents;
const static auto& putLeakDetector = privilegeSetConfigureComponents;
const static auto& deleteLeakDetector = privilegeSetConfigureComponents;

// LeakDetectorCollection
// 漏水检测器集合（LeakDetectorCollection）
const static auto& getLeakDetectorCollection = privilegeSetLogin;
const static auto& headLeakDetectorCollection = privilegeSetLogin;
const static auto& patchLeakDetectorCollection = privilegeSetConfigureComponents;
const static auto& postLeakDetectorCollection = privilegeSetConfigureComponents;
const static auto& putLeakDetectorCollection = privilegeSetConfigureComponents;
const static auto& deleteLeakDetectorCollection = privilegeSetConfigureComponents;

// License
const static auto& getLicense = privilegeSetLogin;
const static auto& headLicense = privilegeSetLogin;
const static auto& patchLicense = privilegeSetConfigureManager;
const static auto& putLicense = privilegeSetConfigureManager;
const static auto& deleteLicense = privilegeSetConfigureManager;
const static auto& postLicense = privilegeSetConfigureManager;

// LicenseCollection
const static auto& getLicenseCollection = privilegeSetLogin;
const static auto& headLicenseCollection = privilegeSetLogin;
const static auto& patchLicenseCollection = privilegeSetConfigureManager;
const static auto& putLicenseCollection = privilegeSetConfigureManager;
const static auto& deleteLicenseCollection = privilegeSetConfigureManager;
const static auto& postLicenseCollection = privilegeSetConfigureManager;

// LicenseService
const static auto& getLicenseService = privilegeSetLogin;
const static auto& headLicenseService = privilegeSetLogin;
const static auto& patchLicenseService = privilegeSetConfigureManager;
const static auto& putLicenseService = privilegeSetConfigureManager;
const static auto& deleteLicenseService = privilegeSetConfigureManager;
const static auto& postLicenseService = privilegeSetConfigureManager;

// LogEntry
// 日志条目（LogEntry）
const static auto& getLogEntry = privilegeSetLogin;
const static auto& headLogEntry = privilegeSetLogin;
const static auto& patchLogEntry = privilegeSetConfigureManager;
const static auto& putLogEntry = privilegeSetConfigureManager;
const static auto& deleteLogEntry = privilegeSetConfigureManager;
const static auto& postLogEntry = privilegeSetConfigureManager;

// Subordinate override for ComputerSystem -> LogServiceCollection -> LogService -> LogEntryCollection -> LogEntry
// ComputerSystem -> LogServiceCollection -> LogService -> LogEntryCollection -> LogEntry 的下级覆盖（subordinate override）
const static auto& patchLogEntrySubOverComputerSystemLogServiceCollectionLogServiceLogEntryCollection = privilegeSetConfigureComponents;
const static auto& putLogEntrySubOverComputerSystemLogServiceCollectionLogServiceLogEntryCollection = privilegeSetConfigureComponents;
const static auto& deleteLogEntrySubOverComputerSystemLogServiceCollectionLogServiceLogEntryCollection = privilegeSetConfigureComponents;
const static auto& postLogEntrySubOverComputerSystemLogServiceCollectionLogServiceLogEntryCollection = privilegeSetConfigureComponents;

// Subordinate override for Chassis -> LogServiceCollection -> LogService -> LogEntryCollection -> LogEntry
// Chassis -> LogServiceCollection -> LogService -> LogEntryCollection -> LogEntry 的下级覆盖（subordinate override）
const static auto& getLogEntrySubOverChassisLogServiceCollectionLogServiceLogEntryCollection = privilegeSetLogin;
const static auto& headLogEntrySubOverChassisLogServiceCollectionLogServiceLogEntryCollection = privilegeSetLogin;
const static auto& patchLogEntrySubOverChassisLogServiceCollectionLogServiceLogEntryCollection = privilegeSetConfigureComponents;
const static auto& putLogEntrySubOverChassisLogServiceCollectionLogServiceLogEntryCollection = privilegeSetConfigureComponents;
const static auto& deleteLogEntrySubOverChassisLogServiceCollectionLogServiceLogEntryCollection = privilegeSetConfigureComponents;
const static auto& postLogEntrySubOverChassisLogServiceCollectionLogServiceLogEntryCollection = privilegeSetConfigureComponents;

// LogEntryCollection
// 日志条目集合（LogEntryCollection）
const static auto& getLogEntryCollection = privilegeSetLogin;
const static auto& headLogEntryCollection = privilegeSetLogin;
const static auto& patchLogEntryCollection = privilegeSetConfigureManager;
const static auto& putLogEntryCollection = privilegeSetConfigureManager;
const static auto& deleteLogEntryCollection = privilegeSetConfigureManager;
const static auto& postLogEntryCollection = privilegeSetConfigureManager;

// Subordinate override for ComputerSystem -> LogServiceCollection -> LogService -> LogEntryCollection
// 针对 ComputerSystem -> LogServiceCollection -> LogService -> LogEntryCollection 的从属覆盖（Subordinate override）
const static auto& patchLogEntryCollectionSubOverComputerSystemLogServiceCollectionLogService = privilegeSetConfigureComponents;
const static auto& putLogEntryCollectionSubOverComputerSystemLogServiceCollectionLogService = privilegeSetConfigureComponents;
const static auto& deleteLogEntryCollectionSubOverComputerSystemLogServiceCollectionLogService = privilegeSetConfigureComponents;
const static auto& postLogEntryCollectionSubOverComputerSystemLogServiceCollectionLogService = privilegeSetConfigureComponents;

// Subordinate override for Chassis -> LogServiceCollection -> LogService -> LogEntryCollection
// 针对 Chassis -> LogServiceCollection -> LogService -> LogEntryCollection 的从属覆盖（Subordinate override）
const static auto& getLogEntryCollectionSubOverChassisLogServiceCollectionLogService = privilegeSetLogin;
const static auto& headLogEntryCollectionSubOverChassisLogServiceCollectionLogService = privilegeSetLogin;
const static auto& patchLogEntryCollectionSubOverChassisLogServiceCollectionLogService = privilegeSetConfigureComponents;
const static auto& putLogEntryCollectionSubOverChassisLogServiceCollectionLogService = privilegeSetConfigureComponents;
const static auto& deleteLogEntryCollectionSubOverChassisLogServiceCollectionLogService = privilegeSetConfigureComponents;
const static auto& postLogEntryCollectionSubOverChassisLogServiceCollectionLogService = privilegeSetConfigureComponents;

// LogService
// 日志服务（LogService）
const static auto& getLogService = privilegeSetLogin;
const static auto& headLogService = privilegeSetLogin;
const static auto& patchLogService = privilegeSetConfigureManager;
const static auto& putLogService = privilegeSetConfigureManager;
const static auto& deleteLogService = privilegeSetConfigureManager;
const static auto& postLogService = privilegeSetConfigureManager;

// Subordinate override for ComputerSystem -> LogServiceCollection -> LogService
// 针对 ComputerSystem -> LogServiceCollection -> LogService 的从属覆盖（Subordinate override）
const static auto& patchLogServiceSubOverComputerSystemLogServiceCollection = privilegeSetConfigureComponents;
const static auto& postLogServiceSubOverComputerSystemLogServiceCollection = privilegeSetConfigureComponents;
const static auto& putLogServiceSubOverComputerSystemLogServiceCollection = privilegeSetConfigureComponents;
const static auto& deleteLogServiceSubOverComputerSystemLogServiceCollection = privilegeSetConfigureComponents;

// Subordinate override for Chassis -> LogServiceCollection -> LogService
// 针对 Chassis -> LogServiceCollection -> LogService 的从属覆盖（Subordinate override）
const static auto& getLogServiceSubOverChassisLogServiceCollection = privilegeSetLogin;
const static auto& headLogServiceSubOverChassisLogServiceCollection = privilegeSetLogin;
const static auto& patchLogServiceSubOverChassisLogServiceCollection = privilegeSetConfigureComponents;
const static auto& putLogServiceSubOverChassisLogServiceCollection = privilegeSetConfigureComponents;
const static auto& deleteLogServiceSubOverChassisLogServiceCollection = privilegeSetConfigureComponents;
const static auto& postLogServiceSubOverChassisLogServiceCollection = privilegeSetConfigureComponents;

// LogServiceCollection
// 日志服务集合（LogServiceCollection）
const static auto& getLogServiceCollection = privilegeSetLogin;
const static auto& headLogServiceCollection = privilegeSetLogin;
const static auto& patchLogServiceCollection = privilegeSetConfigureManager;
const static auto& putLogServiceCollection = privilegeSetConfigureManager;
const static auto& deleteLogServiceCollection = privilegeSetConfigureManager;
const static auto& postLogServiceCollection = privilegeSetConfigureManager;

// Subordinate override for ComputerSystem -> LogServiceCollection
// 针对 ComputerSystem -> LogServiceCollection 的从属覆盖（Subordinate override）
const static auto& patchLogServiceCollectionSubOverComputerSystem = privilegeSetConfigureComponents;
const static auto& putLogServiceCollectionSubOverComputerSystem = privilegeSetConfigureComponents;
const static auto& deleteLogServiceCollectionSubOverComputerSystem = privilegeSetConfigureComponents;
const static auto& postLogServiceCollectionSubOverComputerSystem = privilegeSetConfigureComponents;

// Subordinate override for Chassis -> LogServiceCollection
// 针对 Chassis -> LogServiceCollection 的从属覆盖（Subordinate override）
const static auto& getLogServiceCollectionSubOverChassis = privilegeSetLogin;
const static auto& headLogServiceCollectionSubOverChassis = privilegeSetLogin;
const static auto& patchLogServiceCollectionSubOverChassis = privilegeSetConfigureComponents;
const static auto& putLogServiceCollectionSubOverChassis = privilegeSetConfigureComponents;
const static auto& deleteLogServiceCollectionSubOverChassis = privilegeSetConfigureComponents;
const static auto& postLogServiceCollectionSubOverChassis = privilegeSetConfigureComponents;

// Manager
// 管理器（Manager）
const static auto& getManager = privilegeSetLogin;
const static auto& headManager = privilegeSetLogin;
const static auto& patchManager = privilegeSetConfigureManager;
const static auto& postManager = privilegeSetConfigureManager;
const static auto& putManager = privilegeSetConfigureManager;
const static auto& deleteManager = privilegeSetConfigureManager;

// ManagerCollection
// 管理器集合（ManagerCollection）
const static auto& getManagerCollection = privilegeSetLogin;
const static auto& headManagerCollection = privilegeSetLogin;
const static auto& patchManagerCollection = privilegeSetConfigureManager;
const static auto& postManagerCollection = privilegeSetConfigureManager;
const static auto& putManagerCollection = privilegeSetConfigureManager;
const static auto& deleteManagerCollection = privilegeSetConfigureManager;

// ManagerAccount
// 管理器账户（ManagerAccount）
const static auto& getManagerAccount = privilegeSetConfigureManagerOrConfigureUsersOrConfigureSelf;
const static auto& headManagerAccount = privilegeSetLogin;
const static auto& patchManagerAccount = privilegeSetConfigureUsers;
const static auto& postManagerAccount = privilegeSetConfigureUsers;
const static auto& putManagerAccount = privilegeSetConfigureUsers;
const static auto& deleteManagerAccount = privilegeSetConfigureUsers;

// ManagerAccountCollection
// 管理器账户集合（ManagerAccountCollection）
const static auto& getManagerAccountCollection = privilegeSetLogin;
const static auto& headManagerAccountCollection = privilegeSetLogin;
const static auto& patchManagerAccountCollection = privilegeSetConfigureUsers;
const static auto& putManagerAccountCollection = privilegeSetConfigureUsers;
const static auto& deleteManagerAccountCollection = privilegeSetConfigureUsers;
const static auto& postManagerAccountCollection = privilegeSetConfigureUsers;

// ManagerDiagnosticData
// 管理器诊断数据（ManagerDiagnosticData）
const static auto& getManagerDiagnosticData = privilegeSetLogin;
const static auto& headManagerDiagnosticData = privilegeSetLogin;
const static auto& patchManagerDiagnosticData = privilegeSetConfigureManager;
const static auto& postManagerDiagnosticData = privilegeSetConfigureManager;
const static auto& deleteManagerDiagnosticData = privilegeSetConfigureManager;
const static auto& putManagerDiagnosticData = privilegeSetConfigureManager;

// ManagerNetworkProtocol
// 管理器网络协议（ManagerNetworkProtocol）
const static auto& getManagerNetworkProtocol = privilegeSetLogin;
const static auto& headManagerNetworkProtocol = privilegeSetLogin;
const static auto& patchManagerNetworkProtocol = privilegeSetConfigureManager;
const static auto& postManagerNetworkProtocol = privilegeSetConfigureManager;
const static auto& putManagerNetworkProtocol = privilegeSetConfigureManager;
const static auto& deleteManagerNetworkProtocol = privilegeSetConfigureManager;

// MediaController
// 介质控制器（MediaController）
const static auto& getMediaController = privilegeSetLogin;
const static auto& headMediaController = privilegeSetLogin;
const static auto& patchMediaController = privilegeSetConfigureComponents;
const static auto& postMediaController = privilegeSetConfigureComponents;
const static auto& putMediaController = privilegeSetConfigureComponents;
const static auto& deleteMediaController = privilegeSetConfigureComponents;

// MediaControllerCollection
// 介质控制器集合（MediaControllerCollection）
const static auto& getMediaControllerCollection = privilegeSetLogin;
const static auto& headMediaControllerCollection = privilegeSetLogin;
const static auto& patchMediaControllerCollection = privilegeSetConfigureComponents;
const static auto& postMediaControllerCollection = privilegeSetConfigureComponents;
const static auto& putMediaControllerCollection = privilegeSetConfigureComponents;
const static auto& deleteMediaControllerCollection = privilegeSetConfigureComponents;

// Memory
// 内存（Memory）
const static auto& getMemory = privilegeSetLogin;
const static auto& headMemory = privilegeSetLogin;
const static auto& patchMemory = privilegeSetConfigureComponents;
const static auto& postMemory = privilegeSetConfigureComponents;
const static auto& putMemory = privilegeSetConfigureComponents;
const static auto& deleteMemory = privilegeSetConfigureComponents;

// MemoryCollection
// 内存集合（MemoryCollection）
const static auto& getMemoryCollection = privilegeSetLogin;
const static auto& headMemoryCollection = privilegeSetLogin;
const static auto& patchMemoryCollection = privilegeSetConfigureComponents;
const static auto& postMemoryCollection = privilegeSetConfigureComponents;
const static auto& putMemoryCollection = privilegeSetConfigureComponents;
const static auto& deleteMemoryCollection = privilegeSetConfigureComponents;

// MemoryChunks
// 内存块（MemoryChunks）
const static auto& getMemoryChunks = privilegeSetLogin;
const static auto& headMemoryChunks = privilegeSetLogin;
const static auto& patchMemoryChunks = privilegeSetConfigureComponents;
const static auto& postMemoryChunks = privilegeSetConfigureComponents;
const static auto& putMemoryChunks = privilegeSetConfigureComponents;
const static auto& deleteMemoryChunks = privilegeSetConfigureComponents;

// MemoryChunksCollection
// 内存块集合（MemoryChunksCollection）
const static auto& getMemoryChunksCollection = privilegeSetLogin;
const static auto& headMemoryChunksCollection = privilegeSetLogin;
const static auto& patchMemoryChunksCollection = privilegeSetConfigureComponents;
const static auto& postMemoryChunksCollection = privilegeSetConfigureComponents;
const static auto& putMemoryChunksCollection = privilegeSetConfigureComponents;
const static auto& deleteMemoryChunksCollection = privilegeSetConfigureComponents;

// MemoryDomain
// 内存域（MemoryDomain）
const static auto& getMemoryDomain = privilegeSetLogin;
const static auto& headMemoryDomain = privilegeSetLogin;
const static auto& patchMemoryDomain = privilegeSetConfigureComponents;
const static auto& postMemoryDomain = privilegeSetConfigureComponents;
const static auto& putMemoryDomain = privilegeSetConfigureComponents;
const static auto& deleteMemoryDomain = privilegeSetConfigureComponents;

// MemoryDomainCollection
// 内存域集合（MemoryDomainCollection）
const static auto& getMemoryDomainCollection = privilegeSetLogin;
const static auto& headMemoryDomainCollection = privilegeSetLogin;
const static auto& patchMemoryDomainCollection = privilegeSetConfigureComponents;
const static auto& postMemoryDomainCollection = privilegeSetConfigureComponents;
const static auto& putMemoryDomainCollection = privilegeSetConfigureComponents;
const static auto& deleteMemoryDomainCollection = privilegeSetConfigureComponents;

// MemoryMetrics
// 内存度量（MemoryMetrics）
const static auto& getMemoryMetrics = privilegeSetLogin;
const static auto& headMemoryMetrics = privilegeSetLogin;
const static auto& patchMemoryMetrics = privilegeSetConfigureComponents;
const static auto& postMemoryMetrics = privilegeSetConfigureComponents;
const static auto& putMemoryMetrics = privilegeSetConfigureComponents;
const static auto& deleteMemoryMetrics = privilegeSetConfigureComponents;

// MemoryRegion
// 内存区域（MemoryRegion）
const static auto& getMemoryRegion = privilegeSetLogin;
const static auto& headMemoryRegion = privilegeSetLogin;
const static auto& patchMemoryRegion = privilegeSetConfigureComponents;
const static auto& postMemoryRegion = privilegeSetConfigureComponents;
const static auto& putMemoryRegion = privilegeSetConfigureComponents;
const static auto& deleteMemoryRegion = privilegeSetConfigureComponents;

// MemoryRegionCollection
// 内存区域集合（MemoryRegionCollection）
const static auto& getMemoryRegionCollection = privilegeSetLogin;
const static auto& headMemoryRegionCollection = privilegeSetLogin;
const static auto& patchMemoryRegionCollection = privilegeSetConfigureComponents;
const static auto& postMemoryRegionCollection = privilegeSetConfigureComponents;
const static auto& putMemoryRegionCollection = privilegeSetConfigureComponents;
const static auto& deleteMemoryRegionCollection = privilegeSetConfigureComponents;

// MessageRegistry
// 消息注册表（MessageRegistry）
const static auto& getMessageRegistry = privilegeSetLogin;
const static auto& headMessageRegistry = privilegeSetLogin;
const static auto& patchMessageRegistry = privilegeSetConfigureManager;
const static auto& postMessageRegistry = privilegeSetConfigureManager;
const static auto& putMessageRegistry = privilegeSetConfigureManager;
const static auto& deleteMessageRegistry = privilegeSetConfigureManager;

// MessageRegistryFile
// 消息注册表文件（MessageRegistryFile）
const static auto& getMessageRegistryFile = privilegeSetLogin;
const static auto& headMessageRegistryFile = privilegeSetLogin;
const static auto& patchMessageRegistryFile = privilegeSetConfigureManager;
const static auto& postMessageRegistryFile = privilegeSetConfigureManager;
const static auto& putMessageRegistryFile = privilegeSetConfigureManager;
const static auto& deleteMessageRegistryFile = privilegeSetConfigureManager;

// MessageRegistryFileCollection
// 消息注册表文件集合（MessageRegistryFileCollection）
const static auto& getMessageRegistryFileCollection = privilegeSetLogin;
const static auto& headMessageRegistryFileCollection = privilegeSetLogin;
const static auto& patchMessageRegistryFileCollection = privilegeSetConfigureManager;
const static auto& postMessageRegistryFileCollection = privilegeSetConfigureManager;
const static auto& putMessageRegistryFileCollection = privilegeSetConfigureManager;
const static auto& deleteMessageRegistryFileCollection = privilegeSetConfigureManager;

// MetricDefinition
// 度量定义（MetricDefinition）
const static auto& getMetricDefinition = privilegeSetLogin;
const static auto& headMetricDefinition = privilegeSetLogin;
const static auto& patchMetricDefinition = privilegeSetConfigureManager;
const static auto& putMetricDefinition = privilegeSetConfigureManager;
const static auto& deleteMetricDefinition = privilegeSetConfigureManager;
const static auto& postMetricDefinition = privilegeSetConfigureManager;

// MetricDefinitionCollection
// 度量定义集合（MetricDefinitionCollection）
const static auto& getMetricDefinitionCollection = privilegeSetLogin;
const static auto& headMetricDefinitionCollection = privilegeSetLogin;
const static auto& patchMetricDefinitionCollection = privilegeSetConfigureManager;
const static auto& putMetricDefinitionCollection = privilegeSetConfigureManager;
const static auto& deleteMetricDefinitionCollection = privilegeSetConfigureManager;
const static auto& postMetricDefinitionCollection = privilegeSetConfigureManager;

// MetricReport
// 度量报告（MetricReport）
const static auto& getMetricReport = privilegeSetLogin;
const static auto& headMetricReport = privilegeSetLogin;
const static auto& patchMetricReport = privilegeSetConfigureManager;
const static auto& putMetricReport = privilegeSetConfigureManager;
const static auto& deleteMetricReport = privilegeSetConfigureManager;
const static auto& postMetricReport = privilegeSetConfigureManager;

// MetricReportCollection
// 度量报告集合（MetricReportCollection）
const static auto& getMetricReportCollection = privilegeSetLogin;
const static auto& headMetricReportCollection = privilegeSetLogin;
const static auto& patchMetricReportCollection = privilegeSetConfigureManager;
const static auto& putMetricReportCollection = privilegeSetConfigureManager;
const static auto& deleteMetricReportCollection = privilegeSetConfigureManager;
const static auto& postMetricReportCollection = privilegeSetConfigureManager;

// MetricReportDefinition
// 度量报告定义（MetricReportDefinition）
const static auto& getMetricReportDefinition = privilegeSetLogin;
const static auto& headMetricReportDefinition = privilegeSetLogin;
const static auto& patchMetricReportDefinition = privilegeSetConfigureManager;
const static auto& putMetricReportDefinition = privilegeSetConfigureManager;
const static auto& deleteMetricReportDefinition = privilegeSetConfigureManager;
const static auto& postMetricReportDefinition = privilegeSetConfigureManager;

// MetricReportDefinitionCollection
// 度量报告定义集合（MetricReportDefinitionCollection）
const static auto& getMetricReportDefinitionCollection = privilegeSetLogin;
const static auto& headMetricReportDefinitionCollection = privilegeSetLogin;
const static auto& patchMetricReportDefinitionCollection = privilegeSetConfigureManager;
const static auto& putMetricReportDefinitionCollection = privilegeSetConfigureManager;
const static auto& deleteMetricReportDefinitionCollection = privilegeSetConfigureManager;
const static auto& postMetricReportDefinitionCollection = privilegeSetConfigureManager;

// NetworkAdapter
// 网络适配器（NetworkAdapter）
const static auto& getNetworkAdapter = privilegeSetLogin;
const static auto& headNetworkAdapter = privilegeSetLogin;
const static auto& patchNetworkAdapter = privilegeSetConfigureComponents;
const static auto& postNetworkAdapter = privilegeSetConfigureComponents;
const static auto& putNetworkAdapter = privilegeSetConfigureComponents;
const static auto& deleteNetworkAdapter = privilegeSetConfigureComponents;

// NetworkAdapterCollection
// 网络适配器集合（NetworkAdapterCollection）
const static auto& getNetworkAdapterCollection = privilegeSetLogin;
const static auto& headNetworkAdapterCollection = privilegeSetLogin;
const static auto& patchNetworkAdapterCollection = privilegeSetConfigureComponents;
const static auto& postNetworkAdapterCollection = privilegeSetConfigureComponents;
const static auto& putNetworkAdapterCollection = privilegeSetConfigureComponents;
const static auto& deleteNetworkAdapterCollection = privilegeSetConfigureComponents;

// NetworkAdapterMetrics
// 网络适配器度量（NetworkAdapterMetrics）
const static auto& getNetworkAdapterMetrics = privilegeSetLogin;
const static auto& headNetworkAdapterMetrics = privilegeSetLogin;
const static auto& patchNetworkAdapterMetrics = privilegeSetConfigureManager;
const static auto& putNetworkAdapterMetrics = privilegeSetConfigureManager;
const static auto& deleteNetworkAdapterMetrics = privilegeSetConfigureManager;
const static auto& postNetworkAdapterMetrics = privilegeSetConfigureManager;

// NetworkDeviceFunction
// 网络设备功能（NetworkDeviceFunction）
const static auto& getNetworkDeviceFunction = privilegeSetLogin;
const static auto& headNetworkDeviceFunction = privilegeSetLogin;
const static auto& patchNetworkDeviceFunction = privilegeSetConfigureComponents;
const static auto& postNetworkDeviceFunction = privilegeSetConfigureComponents;
const static auto& putNetworkDeviceFunction = privilegeSetConfigureComponents;
const static auto& deleteNetworkDeviceFunction = privilegeSetConfigureComponents;

// NetworkDeviceFunctionCollection
// 网络设备功能集合（NetworkDeviceFunctionCollection）
const static auto& getNetworkDeviceFunctionCollection = privilegeSetLogin;
const static auto& headNetworkDeviceFunctionCollection = privilegeSetLogin;
const static auto& patchNetworkDeviceFunctionCollection = privilegeSetConfigureComponents;
const static auto& postNetworkDeviceFunctionCollection = privilegeSetConfigureComponents;
const static auto& putNetworkDeviceFunctionCollection = privilegeSetConfigureComponents;
const static auto& deleteNetworkDeviceFunctionCollection = privilegeSetConfigureComponents;

// NetworkDeviceFunctionMetrics
// 网络设备功能度量（NetworkDeviceFunctionMetrics）
const static auto& getNetworkDeviceFunctionMetrics = privilegeSetLogin;
const static auto& headNetworkDeviceFunctionMetrics = privilegeSetLogin;
const static auto& patchNetworkDeviceFunctionMetrics = privilegeSetConfigureManager;
const static auto& putNetworkDeviceFunctionMetrics = privilegeSetConfigureManager;
const static auto& deleteNetworkDeviceFunctionMetrics = privilegeSetConfigureManager;
const static auto& postNetworkDeviceFunctionMetrics = privilegeSetConfigureManager;

// NetworkInterface
// 网络接口（NetworkInterface）
const static auto& getNetworkInterface = privilegeSetLogin;
const static auto& headNetworkInterface = privilegeSetLogin;
const static auto& patchNetworkInterface = privilegeSetConfigureComponents;
const static auto& postNetworkInterface = privilegeSetConfigureComponents;
const static auto& putNetworkInterface = privilegeSetConfigureComponents;
const static auto& deleteNetworkInterface = privilegeSetConfigureComponents;

// NetworkInterfaceCollection
// 网络接口集合（NetworkInterfaceCollection）
const static auto& getNetworkInterfaceCollection = privilegeSetLogin;
const static auto& headNetworkInterfaceCollection = privilegeSetLogin;
const static auto& patchNetworkInterfaceCollection = privilegeSetConfigureComponents;
const static auto& postNetworkInterfaceCollection = privilegeSetConfigureComponents;
const static auto& putNetworkInterfaceCollection = privilegeSetConfigureComponents;
const static auto& deleteNetworkInterfaceCollection = privilegeSetConfigureComponents;

// NetworkPort
// 网络端口（NetworkPort）
const static auto& getNetworkPort = privilegeSetLogin;
const static auto& headNetworkPort = privilegeSetLogin;
const static auto& patchNetworkPort = privilegeSetConfigureComponents;
const static auto& postNetworkPort = privilegeSetConfigureComponents;
const static auto& putNetworkPort = privilegeSetConfigureComponents;
const static auto& deleteNetworkPort = privilegeSetConfigureComponents;

// NetworkPortCollection
// 网络端口集合（NetworkPortCollection）
const static auto& getNetworkPortCollection = privilegeSetLogin;
const static auto& headNetworkPortCollection = privilegeSetLogin;
const static auto& patchNetworkPortCollection = privilegeSetConfigureComponents;
const static auto& postNetworkPortCollection = privilegeSetConfigureComponents;
const static auto& putNetworkPortCollection = privilegeSetConfigureComponents;
const static auto& deleteNetworkPortCollection = privilegeSetConfigureComponents;

// OperatingConfig
// 运行配置（OperatingConfig）
const static auto& getOperatingConfig = privilegeSetLogin;
const static auto& headOperatingConfig = privilegeSetLogin;
const static auto& patchOperatingConfig = privilegeSetConfigureComponents;
const static auto& postOperatingConfig = privilegeSetConfigureComponents;
const static auto& putOperatingConfig = privilegeSetConfigureComponents;
const static auto& deleteOperatingConfig = privilegeSetConfigureComponents;

// OperatingConfigCollection
// 运行配置集合（OperatingConfigCollection）
const static auto& getOperatingConfigCollection = privilegeSetLogin;
const static auto& headOperatingConfigCollection = privilegeSetLogin;
const static auto& patchOperatingConfigCollection = privilegeSetConfigureComponents;
const static auto& postOperatingConfigCollection = privilegeSetConfigureComponents;
const static auto& putOperatingConfigCollection = privilegeSetConfigureComponents;
const static auto& deleteOperatingConfigCollection = privilegeSetConfigureComponents;

// OperatingSystem
// 操作系统（OperatingSystem）
const static auto& getOperatingSystem = privilegeSetLogin;
const static auto& headOperatingSystem = privilegeSetLogin;
const static auto& patchOperatingSystem = privilegeSetConfigureComponents;
const static auto& postOperatingSystem = privilegeSetConfigureComponents;
const static auto& putOperatingSystem = privilegeSetConfigureComponents;
const static auto& deleteOperatingSystem = privilegeSetConfigureComponents;

// OutboundConnection
// 出站连接（OutboundConnection）
const static auto& getOutboundConnection = privilegeSetLogin;
const static auto& headOutboundConnection = privilegeSetLogin;
const static auto& patchOutboundConnection = privilegeSetConfigureManager;
const static auto& putOutboundConnection = privilegeSetConfigureManager;
const static auto& deleteOutboundConnection = privilegeSetConfigureManager;
const static auto& postOutboundConnection = privilegeSetConfigureManager;

// OutboundConnectionCollection
// 出站连接集合（OutboundConnectionCollection）
const static auto& getOutboundConnectionCollection = privilegeSetLogin;
const static auto& headOutboundConnectionCollection = privilegeSetLogin;
const static auto& patchOutboundConnectionCollection = privilegeSetConfigureManager;
const static auto& putOutboundConnectionCollection = privilegeSetConfigureManager;
const static auto& deleteOutboundConnectionCollection = privilegeSetConfigureManager;
const static auto& postOutboundConnectionCollection = privilegeSetConfigureManager;

// Outlet
// 电源插座（Outlet）
const static auto& getOutlet = privilegeSetLogin;
const static auto& headOutlet = privilegeSetLogin;
const static auto& patchOutlet = privilegeSetConfigureComponents;
const static auto& postOutlet = privilegeSetConfigureComponents;
const static auto& putOutlet = privilegeSetConfigureComponents;
const static auto& deleteOutlet = privilegeSetConfigureComponents;

// OutletCollection
// 电源插座集合（OutletCollection）
const static auto& getOutletCollection = privilegeSetLogin;
const static auto& headOutletCollection = privilegeSetLogin;
const static auto& patchOutletCollection = privilegeSetConfigureComponents;
const static auto& postOutletCollection = privilegeSetConfigureComponents;
const static auto& putOutletCollection = privilegeSetConfigureComponents;
const static auto& deleteOutletCollection = privilegeSetConfigureComponents;

// OutletGroup
// 电源插座组（OutletGroup）
const static auto& getOutletGroup = privilegeSetLogin;
const static auto& headOutletGroup = privilegeSetLogin;
const static auto& patchOutletGroup = privilegeSetConfigureComponents;
const static auto& postOutletGroup = privilegeSetConfigureComponents;
const static auto& putOutletGroup = privilegeSetConfigureComponents;
const static auto& deleteOutletGroup = privilegeSetConfigureComponents;

// OutletGroupCollection
// 电源插座组集合（OutletGroupCollection）
const static auto& getOutletGroupCollection = privilegeSetLogin;
const static auto& headOutletGroupCollection = privilegeSetLogin;
const static auto& patchOutletGroupCollection = privilegeSetConfigureComponents;
const static auto& postOutletGroupCollection = privilegeSetConfigureComponents;
const static auto& putOutletGroupCollection = privilegeSetConfigureComponents;
const static auto& deleteOutletGroupCollection = privilegeSetConfigureComponents;

// PCIeDevice
// PCIe 设备（PCIeDevice）
const static auto& getPCIeDevice = privilegeSetLogin;
const static auto& headPCIeDevice = privilegeSetLogin;
const static auto& patchPCIeDevice = privilegeSetConfigureComponents;
const static auto& postPCIeDevice = privilegeSetConfigureComponents;
const static auto& putPCIeDevice = privilegeSetConfigureComponents;
const static auto& deletePCIeDevice = privilegeSetConfigureComponents;

// PCIeDeviceCollection
// PCIe 设备集合（PCIeDeviceCollection）
const static auto& getPCIeDeviceCollection = privilegeSetLogin;
const static auto& headPCIeDeviceCollection = privilegeSetLogin;
const static auto& patchPCIeDeviceCollection = privilegeSetConfigureComponents;
const static auto& postPCIeDeviceCollection = privilegeSetConfigureComponents;
const static auto& putPCIeDeviceCollection = privilegeSetConfigureComponents;
const static auto& deletePCIeDeviceCollection = privilegeSetConfigureComponents;

// PCIeFunction
// PCIe 功能（PCIeFunction）
const static auto& getPCIeFunction = privilegeSetLogin;
const static auto& headPCIeFunction = privilegeSetLogin;
const static auto& patchPCIeFunction = privilegeSetConfigureComponents;
const static auto& postPCIeFunction = privilegeSetConfigureComponents;
const static auto& putPCIeFunction = privilegeSetConfigureComponents;
const static auto& deletePCIeFunction = privilegeSetConfigureComponents;

// PCIeFunctionCollection
// PCIe 功能集合（PCIeFunctionCollection）
const static auto& getPCIeFunctionCollection = privilegeSetLogin;
const static auto& headPCIeFunctionCollection = privilegeSetLogin;
const static auto& patchPCIeFunctionCollection = privilegeSetConfigureComponents;
const static auto& postPCIeFunctionCollection = privilegeSetConfigureComponents;
const static auto& putPCIeFunctionCollection = privilegeSetConfigureComponents;
const static auto& deletePCIeFunctionCollection = privilegeSetConfigureComponents;

// PCIeSlots
// PCIe 插槽（PCIeSlots）
const static auto& getPCIeSlots = privilegeSetLogin;
const static auto& headPCIeSlots = privilegeSetLogin;
const static auto& patchPCIeSlots = privilegeSetConfigureComponents;
const static auto& postPCIeSlots = privilegeSetConfigureComponents;
const static auto& putPCIeSlots = privilegeSetConfigureComponents;
const static auto& deletePCIeSlots = privilegeSetConfigureComponents;

// Port
// 端口（Port）
const static auto& getPort = privilegeSetLogin;
const static auto& headPort = privilegeSetLogin;
const static auto& patchPort = privilegeSetConfigureComponents;
const static auto& postPort = privilegeSetConfigureComponents;
const static auto& putPort = privilegeSetConfigureComponents;
const static auto& deletePort = privilegeSetConfigureComponents;

// PortCollection
// 端口集合（PortCollection）
const static auto& getPortCollection = privilegeSetLogin;
const static auto& headPortCollection = privilegeSetLogin;
const static auto& patchPortCollection = privilegeSetConfigureComponents;
const static auto& postPortCollection = privilegeSetConfigureComponents;
const static auto& putPortCollection = privilegeSetConfigureComponents;
const static auto& deletePortCollection = privilegeSetConfigureComponents;

// PortMetrics
// 端口度量（PortMetrics）
const static auto& getPortMetrics = privilegeSetLogin;
const static auto& headPortMetrics = privilegeSetLogin;
const static auto& patchPortMetrics = privilegeSetConfigureComponents;
const static auto& postPortMetrics = privilegeSetConfigureComponents;
const static auto& putPortMetrics = privilegeSetConfigureComponents;
const static auto& deletePortMetrics = privilegeSetConfigureComponents;

// Power
// 电源（Power）
const static auto& getPower = privilegeSetLogin;
const static auto& headPower = privilegeSetLogin;
const static auto& patchPower = privilegeSetConfigureManager;
const static auto& putPower = privilegeSetConfigureManager;
const static auto& deletePower = privilegeSetConfigureManager;
const static auto& postPower = privilegeSetConfigureManager;

// PowerDistribution
// 配电（PowerDistribution）
const static auto& getPowerDistribution = privilegeSetLogin;
const static auto& headPowerDistribution = privilegeSetLogin;
const static auto& patchPowerDistribution = privilegeSetConfigureComponents;
const static auto& postPowerDistribution = privilegeSetConfigureComponents;
const static auto& putPowerDistribution = privilegeSetConfigureComponents;
const static auto& deletePowerDistribution = privilegeSetConfigureComponents;

// PowerDistributionCollection
// 配电集合（PowerDistributionCollection）
const static auto& getPowerDistributionCollection = privilegeSetLogin;
const static auto& headPowerDistributionCollection = privilegeSetLogin;
const static auto& patchPowerDistributionCollection = privilegeSetConfigureComponents;
const static auto& postPowerDistributionCollection = privilegeSetConfigureComponents;
const static auto& putPowerDistributionCollection = privilegeSetConfigureComponents;
const static auto& deletePowerDistributionCollection = privilegeSetConfigureComponents;

// PowerDistributionMetrics
// 配电度量（PowerDistributionMetrics）
const static auto& getPowerDistributionMetrics = privilegeSetLogin;
const static auto& headPowerDistributionMetrics = privilegeSetLogin;
const static auto& patchPowerDistributionMetrics = privilegeSetConfigureComponents;
const static auto& postPowerDistributionMetrics = privilegeSetConfigureComponents;
const static auto& putPowerDistributionMetrics = privilegeSetConfigureComponents;
const static auto& deletePowerDistributionMetrics = privilegeSetConfigureComponents;

// PowerDomain
// 电源域（PowerDomain）
const static auto& getPowerDomain = privilegeSetLogin;
const static auto& headPowerDomain = privilegeSetLogin;
const static auto& patchPowerDomain = privilegeSetConfigureManager;
const static auto& putPowerDomain = privilegeSetConfigureManager;
const static auto& deletePowerDomain = privilegeSetConfigureManager;
const static auto& postPowerDomain = privilegeSetConfigureManager;

// PowerDomainCollection
// 电源域集合（PowerDomainCollection）
const static auto& getPowerDomainCollection = privilegeSetLogin;
const static auto& headPowerDomainCollection = privilegeSetLogin;
const static auto& patchPowerDomainCollection = privilegeSetConfigureManager;
const static auto& putPowerDomainCollection = privilegeSetConfigureManager;
const static auto& deletePowerDomainCollection = privilegeSetConfigureManager;
const static auto& postPowerDomainCollection = privilegeSetConfigureManager;

// PowerEquipment
// 电源设备（PowerEquipment）
const static auto& getPowerEquipment = privilegeSetLogin;
const static auto& headPowerEquipment = privilegeSetLogin;
const static auto& patchPowerEquipment = privilegeSetConfigureManager;
const static auto& putPowerEquipment = privilegeSetConfigureManager;
const static auto& deletePowerEquipment = privilegeSetConfigureManager;
const static auto& postPowerEquipment = privilegeSetConfigureManager;

// PowerSubsystem
// 电源子系统（PowerSubsystem）
const static auto& getPowerSubsystem = privilegeSetLogin;
const static auto& headPowerSubsystem = privilegeSetLogin;
const static auto& patchPowerSubsystem = privilegeSetConfigureManager;
const static auto& putPowerSubsystem = privilegeSetConfigureManager;
const static auto& deletePowerSubsystem = privilegeSetConfigureManager;
const static auto& postPowerSubsystem = privilegeSetConfigureManager;

// PowerSupply
// 电源供应器（PowerSupply）
const static auto& getPowerSupply = privilegeSetLogin;
const static auto& headPowerSupply = privilegeSetLogin;
const static auto& patchPowerSupply = privilegeSetConfigureManager;
const static auto& putPowerSupply = privilegeSetConfigureManager;
const static auto& deletePowerSupply = privilegeSetConfigureManager;
const static auto& postPowerSupply = privilegeSetConfigureManager;

// PowerSupplyCollection
// 电源供应器集合（PowerSupplyCollection）
const static auto& getPowerSupplyCollection = privilegeSetLogin;
const static auto& headPowerSupplyCollection = privilegeSetLogin;
const static auto& patchPowerSupplyCollection = privilegeSetConfigureManager;
const static auto& putPowerSupplyCollection = privilegeSetConfigureManager;
const static auto& deletePowerSupplyCollection = privilegeSetConfigureManager;
const static auto& postPowerSupplyCollection = privilegeSetConfigureManager;

// PowerSupplyMetrics
// 电源供应器度量（PowerSupplyMetrics）
const static auto& getPowerSupplyMetrics = privilegeSetLogin;
const static auto& headPowerSupplyMetrics = privilegeSetLogin;
const static auto& patchPowerSupplyMetrics = privilegeSetConfigureManager;
const static auto& putPowerSupplyMetrics = privilegeSetConfigureManager;
const static auto& deletePowerSupplyMetrics = privilegeSetConfigureManager;
const static auto& postPowerSupplyMetrics = privilegeSetConfigureManager;

// PrivilegeRegistry
// 权限注册表（PrivilegeRegistry）
const static auto& getPrivilegeRegistry = privilegeSetLogin;
const static auto& headPrivilegeRegistry = privilegeSetLogin;
const static auto& patchPrivilegeRegistry = privilegeSetConfigureManager;
const static auto& postPrivilegeRegistry = privilegeSetConfigureManager;
const static auto& putPrivilegeRegistry = privilegeSetConfigureManager;
const static auto& deletePrivilegeRegistry = privilegeSetConfigureManager;

// Processor
// 处理器（Processor）
const static auto& getProcessor = privilegeSetLogin;
const static auto& headProcessor = privilegeSetLogin;
const static auto& patchProcessor = privilegeSetConfigureComponents;
const static auto& putProcessor = privilegeSetConfigureComponents;
const static auto& deleteProcessor = privilegeSetConfigureComponents;
const static auto& postProcessor = privilegeSetConfigureComponents;

// ProcessorCollection
// 处理器集合（ProcessorCollection）
const static auto& getProcessorCollection = privilegeSetLogin;
const static auto& headProcessorCollection = privilegeSetLogin;
const static auto& patchProcessorCollection = privilegeSetConfigureComponents;
const static auto& putProcessorCollection = privilegeSetConfigureComponents;
const static auto& deleteProcessorCollection = privilegeSetConfigureComponents;
const static auto& postProcessorCollection = privilegeSetConfigureComponents;

// ProcessorMetrics
// 处理器度量（ProcessorMetrics）
const static auto& getProcessorMetrics = privilegeSetLogin;
const static auto& headProcessorMetrics = privilegeSetLogin;
const static auto& patchProcessorMetrics = privilegeSetConfigureComponents;
const static auto& putProcessorMetrics = privilegeSetConfigureComponents;
const static auto& deleteProcessorMetrics = privilegeSetConfigureComponents;
const static auto& postProcessorMetrics = privilegeSetConfigureComponents;

// Pump
// 泵（Pump）
const static auto& getPump = privilegeSetLogin;
const static auto& headPump = privilegeSetLogin;
const static auto& patchPump = privilegeSetConfigureComponents;
const static auto& putPump = privilegeSetConfigureComponents;
const static auto& deletePump = privilegeSetConfigureComponents;
const static auto& postPump = privilegeSetConfigureComponents;

// PumpCollection
// 泵集合（PumpCollection）
const static auto& getPumpCollection = privilegeSetLogin;
const static auto& headPumpCollection = privilegeSetLogin;
const static auto& patchPumpCollection = privilegeSetConfigureComponents;
const static auto& putPumpCollection = privilegeSetConfigureComponents;
const static auto& deletePumpCollection = privilegeSetConfigureComponents;
const static auto& postPumpCollection = privilegeSetConfigureComponents;

// RegisteredClient
// 已注册客户端（RegisteredClient）
const static auto& getRegisteredClient = privilegeSetLogin;
const static auto& headRegisteredClient = privilegeSetLogin;
const static auto& patchRegisteredClient = privilegeSetConfigureManagerOrConfigureSelf;
const static auto& postRegisteredClient = privilegeSetConfigureManagerOrConfigureSelf;
const static auto& putRegisteredClient = privilegeSetConfigureManagerOrConfigureSelf;
const static auto& deleteRegisteredClient = privilegeSetConfigureManagerOrConfigureSelf;

// RegisteredClientCollection
// 已注册客户端集合（RegisteredClientCollection）
const static auto& getRegisteredClientCollection = privilegeSetLogin;
const static auto& headRegisteredClientCollection = privilegeSetLogin;
const static auto& patchRegisteredClientCollection = privilegeSetConfigureManagerOrConfigureComponents;
const static auto& postRegisteredClientCollection = privilegeSetConfigureManagerOrConfigureComponents;
const static auto& putRegisteredClientCollection = privilegeSetConfigureManagerOrConfigureComponents;
const static auto& deleteRegisteredClientCollection = privilegeSetConfigureManagerOrConfigureComponents;

// Reservoir
// 储液器（Reservoir）
const static auto& getReservoir = privilegeSetLogin;
const static auto& headReservoir = privilegeSetLogin;
const static auto& patchReservoir = privilegeSetConfigureComponents;
const static auto& putReservoir = privilegeSetConfigureComponents;
const static auto& deleteReservoir = privilegeSetConfigureComponents;
const static auto& postReservoir = privilegeSetConfigureComponents;

// ReservoirCollection
// 储液器集合（ReservoirCollection）
const static auto& getReservoirCollection = privilegeSetLogin;
const static auto& headReservoirCollection = privilegeSetLogin;
const static auto& patchReservoirCollection = privilegeSetConfigureComponents;
const static auto& putReservoirCollection = privilegeSetConfigureComponents;
const static auto& deleteReservoirCollection = privilegeSetConfigureComponents;
const static auto& postReservoirCollection = privilegeSetConfigureComponents;

// ResourceBlock
// 资源块（ResourceBlock）
const static auto& getResourceBlock = privilegeSetLogin;
const static auto& headResourceBlock = privilegeSetLogin;
const static auto& patchResourceBlock = privilegeSetConfigureComponents;
const static auto& putResourceBlock = privilegeSetConfigureComponents;
const static auto& deleteResourceBlock = privilegeSetConfigureComponents;
const static auto& postResourceBlock = privilegeSetConfigureComponents;

// ResourceBlockCollection
// 资源块集合（ResourceBlockCollection）
const static auto& getResourceBlockCollection = privilegeSetLogin;
const static auto& headResourceBlockCollection = privilegeSetLogin;
const static auto& patchResourceBlockCollection = privilegeSetConfigureComponents;
const static auto& putResourceBlockCollection = privilegeSetConfigureComponents;
const static auto& deleteResourceBlockCollection = privilegeSetConfigureComponents;
const static auto& postResourceBlockCollection = privilegeSetConfigureComponents;

// Role
// 角色（Role）
const static auto& getRole = privilegeSetLogin;
const static auto& headRole = privilegeSetLogin;
const static auto& patchRole = privilegeSetConfigureManager;
const static auto& putRole = privilegeSetConfigureManager;
const static auto& deleteRole = privilegeSetConfigureManager;
const static auto& postRole = privilegeSetConfigureManager;

// RoleCollection
// 角色集合（RoleCollection）
const static auto& getRoleCollection = privilegeSetLogin;
const static auto& headRoleCollection = privilegeSetLogin;
const static auto& patchRoleCollection = privilegeSetConfigureManager;
const static auto& putRoleCollection = privilegeSetConfigureManager;
const static auto& deleteRoleCollection = privilegeSetConfigureManager;
const static auto& postRoleCollection = privilegeSetConfigureManager;

// RouteEntry
// 路由条目（RouteEntry）
const static auto& getRouteEntry = privilegeSetLogin;
const static auto& headRouteEntry = privilegeSetLogin;
const static auto& patchRouteEntry = privilegeSetConfigureComponents;
const static auto& putRouteEntry = privilegeSetConfigureComponents;
const static auto& deleteRouteEntry = privilegeSetConfigureComponents;
const static auto& postRouteEntry = privilegeSetConfigureComponents;

// RouteEntryCollection
// 路由条目集合（RouteEntryCollection）
const static auto& getRouteEntryCollection = privilegeSetLogin;
const static auto& headRouteEntryCollection = privilegeSetLogin;
const static auto& patchRouteEntryCollection = privilegeSetConfigureComponents;
const static auto& putRouteEntryCollection = privilegeSetConfigureComponents;
const static auto& deleteRouteEntryCollection = privilegeSetConfigureComponents;
const static auto& postRouteEntryCollection = privilegeSetConfigureComponents;

// RouteSetEntry
// 路由集条目（RouteSetEntry）
const static auto& getRouteSetEntry = privilegeSetLogin;
const static auto& headRouteSetEntry = privilegeSetLogin;
const static auto& patchRouteSetEntry = privilegeSetConfigureComponents;
const static auto& putRouteSetEntry = privilegeSetConfigureComponents;
const static auto& deleteRouteSetEntry = privilegeSetConfigureComponents;
const static auto& postRouteSetEntry = privilegeSetConfigureComponents;

// RouteSetEntryCollection
// 路由集条目集合（RouteSetEntryCollection）
const static auto& getRouteSetEntryCollection = privilegeSetLogin;
const static auto& headRouteSetEntryCollection = privilegeSetLogin;
const static auto& patchRouteSetEntryCollection = privilegeSetConfigureComponents;
const static auto& putRouteSetEntryCollection = privilegeSetConfigureComponents;
const static auto& deleteRouteSetEntryCollection = privilegeSetConfigureComponents;
const static auto& postRouteSetEntryCollection = privilegeSetConfigureComponents;

// SecureBoot
// 安全启动（SecureBoot）
const static auto& getSecureBoot = privilegeSetLogin;
const static auto& headSecureBoot = privilegeSetLogin;
const static auto& patchSecureBoot = privilegeSetConfigureComponents;
const static auto& postSecureBoot = privilegeSetConfigureComponents;
const static auto& putSecureBoot = privilegeSetConfigureComponents;
const static auto& deleteSecureBoot = privilegeSetConfigureComponents;

// SecureBootDatabase
// 安全启动数据库（SecureBootDatabase）
const static auto& getSecureBootDatabase = privilegeSetLogin;
const static auto& headSecureBootDatabase = privilegeSetLogin;
const static auto& patchSecureBootDatabase = privilegeSetConfigureComponents;
const static auto& postSecureBootDatabase = privilegeSetConfigureComponents;
const static auto& putSecureBootDatabase = privilegeSetConfigureComponents;
const static auto& deleteSecureBootDatabase = privilegeSetConfigureComponents;

// SecureBootDatabaseCollection
// 安全启动数据库集合（SecureBootDatabaseCollection）
const static auto& getSecureBootDatabaseCollection = privilegeSetLogin;
const static auto& headSecureBootDatabaseCollection = privilegeSetLogin;
const static auto& patchSecureBootDatabaseCollection = privilegeSetConfigureComponents;
const static auto& postSecureBootDatabaseCollection = privilegeSetConfigureComponents;
const static auto& putSecureBootDatabaseCollection = privilegeSetConfigureComponents;
const static auto& deleteSecureBootDatabaseCollection = privilegeSetConfigureComponents;

// SecurityPolicy
// 安全策略（SecurityPolicy）
const static auto& getSecurityPolicy = privilegeSetLogin;
const static auto& headSecurityPolicy = privilegeSetLogin;
const static auto& patchSecurityPolicy = privilegeSetConfigureManager;
const static auto& putSecurityPolicy = privilegeSetConfigureManager;
const static auto& deleteSecurityPolicy = privilegeSetConfigureManager;
const static auto& postSecurityPolicy = privilegeSetConfigureManager;

// Sensor
// 传感器（Sensor）
const static auto& getSensor = privilegeSetLogin;
const static auto& headSensor = privilegeSetLogin;
const static auto& patchSensor = privilegeSetConfigureComponents;
const static auto& postSensor = privilegeSetConfigureComponents;
const static auto& putSensor = privilegeSetConfigureComponents;
const static auto& deleteSensor = privilegeSetConfigureComponents;

// SensorCollection
// 传感器集合（SensorCollection）
const static auto& getSensorCollection = privilegeSetLogin;
const static auto& headSensorCollection = privilegeSetLogin;
const static auto& patchSensorCollection = privilegeSetConfigureComponents;
const static auto& postSensorCollection = privilegeSetConfigureComponents;
const static auto& putSensorCollection = privilegeSetConfigureComponents;
const static auto& deleteSensorCollection = privilegeSetConfigureComponents;

// SerialInterface
// 串行接口（SerialInterface）
const static auto& getSerialInterface = privilegeSetLogin;
const static auto& headSerialInterface = privilegeSetLogin;
const static auto& patchSerialInterface = privilegeSetConfigureManager;
const static auto& putSerialInterface = privilegeSetConfigureManager;
const static auto& deleteSerialInterface = privilegeSetConfigureManager;
const static auto& postSerialInterface = privilegeSetConfigureManager;

// SerialInterfaceCollection
// 串行接口集合（SerialInterfaceCollection）
const static auto& getSerialInterfaceCollection = privilegeSetLogin;
const static auto& headSerialInterfaceCollection = privilegeSetLogin;
const static auto& patchSerialInterfaceCollection = privilegeSetConfigureManager;
const static auto& putSerialInterfaceCollection = privilegeSetConfigureManager;
const static auto& deleteSerialInterfaceCollection = privilegeSetConfigureManager;
const static auto& postSerialInterfaceCollection = privilegeSetConfigureManager;

// ServiceConditions
// 服务状况（ServiceConditions）
const static auto& getServiceConditions = privilegeSetLogin;
const static auto& headServiceConditions = privilegeSetLogin;
const static auto& patchServiceConditions = privilegeSetConfigureManager;
const static auto& putServiceConditions = privilegeSetConfigureManager;
const static auto& deleteServiceConditions = privilegeSetConfigureManager;
const static auto& postServiceConditions = privilegeSetConfigureManager;

// ServiceRoot
// 服务根（ServiceRoot）
const static auto& getServiceRoot = privilegeSetLoginOrNoAuth;
const static auto& headServiceRoot = privilegeSetLoginOrNoAuth;
const static auto& patchServiceRoot = privilegeSetConfigureManager;
const static auto& putServiceRoot = privilegeSetConfigureManager;
const static auto& deleteServiceRoot = privilegeSetConfigureManager;
const static auto& postServiceRoot = privilegeSetConfigureManager;

// Session
// 会话（Session）
const static auto& getSession = privilegeSetConfigureManagerOrConfigureSelf;
const static auto& headSession = privilegeSetConfigureManagerOrConfigureSelf;
const static auto& patchSession = privilegeSetConfigureManager;
const static auto& putSession = privilegeSetConfigureManager;
const static auto& deleteSession = privilegeSetConfigureManagerOrConfigureSelf;
const static auto& postSession = privilegeSetConfigureManager;

// SessionCollection
// 会话集合（SessionCollection）
const static auto& getSessionCollection = privilegeSetLogin;
const static auto& headSessionCollection = privilegeSetLogin;
const static auto& patchSessionCollection = privilegeSetConfigureManager;
const static auto& putSessionCollection = privilegeSetConfigureManager;
const static auto& deleteSessionCollection = privilegeSetConfigureManager;
const static auto& postSessionCollection = privilegeSetLogin;

// SessionService
// 会话服务（SessionService）
const static auto& getSessionService = privilegeSetLogin;
const static auto& headSessionService = privilegeSetLogin;
const static auto& patchSessionService = privilegeSetConfigureManager;
const static auto& putSessionService = privilegeSetConfigureManager;
const static auto& deleteSessionService = privilegeSetConfigureManager;
const static auto& postSessionService = privilegeSetConfigureManager;

// Signature
// 签名（Signature）
const static auto& getSignature = privilegeSetLogin;
const static auto& headSignature = privilegeSetLogin;
const static auto& patchSignature = privilegeSetConfigureComponents;
const static auto& postSignature = privilegeSetConfigureComponents;
const static auto& putSignature = privilegeSetConfigureComponents;
const static auto& deleteSignature = privilegeSetConfigureComponents;

// SignatureCollection
// 签名集合（SignatureCollection）
const static auto& getSignatureCollection = privilegeSetLogin;
const static auto& headSignatureCollection = privilegeSetLogin;
const static auto& patchSignatureCollection = privilegeSetConfigureComponents;
const static auto& postSignatureCollection = privilegeSetConfigureComponents;
const static auto& putSignatureCollection = privilegeSetConfigureComponents;
const static auto& deleteSignatureCollection = privilegeSetConfigureComponents;

// SimpleStorage
// 简单存储（SimpleStorage）
const static auto& getSimpleStorage = privilegeSetLogin;
const static auto& headSimpleStorage = privilegeSetLogin;
const static auto& patchSimpleStorage = privilegeSetConfigureComponents;
const static auto& postSimpleStorage = privilegeSetConfigureComponents;
const static auto& putSimpleStorage = privilegeSetConfigureComponents;
const static auto& deleteSimpleStorage = privilegeSetConfigureComponents;

// SimpleStorageCollection
// 简单存储集合（SimpleStorageCollection）
const static auto& getSimpleStorageCollection = privilegeSetLogin;
const static auto& headSimpleStorageCollection = privilegeSetLogin;
const static auto& patchSimpleStorageCollection = privilegeSetConfigureComponents;
const static auto& postSimpleStorageCollection = privilegeSetConfigureComponents;
const static auto& putSimpleStorageCollection = privilegeSetConfigureComponents;
const static auto& deleteSimpleStorageCollection = privilegeSetConfigureComponents;

// SoftwareInventory
// 软件清单（SoftwareInventory）
const static auto& getSoftwareInventory = privilegeSetLogin;
const static auto& headSoftwareInventory = privilegeSetLogin;
const static auto& patchSoftwareInventory = privilegeSetConfigureComponents;
const static auto& postSoftwareInventory = privilegeSetConfigureComponents;
const static auto& putSoftwareInventory = privilegeSetConfigureComponents;
const static auto& deleteSoftwareInventory = privilegeSetConfigureComponents;

// SoftwareInventoryCollection
// 软件清单集合（SoftwareInventoryCollection）
const static auto& getSoftwareInventoryCollection = privilegeSetLogin;
const static auto& headSoftwareInventoryCollection = privilegeSetLogin;
const static auto& patchSoftwareInventoryCollection = privilegeSetConfigureComponents;
const static auto& postSoftwareInventoryCollection = privilegeSetConfigureComponents;
const static auto& putSoftwareInventoryCollection = privilegeSetConfigureComponents;
const static auto& deleteSoftwareInventoryCollection = privilegeSetConfigureComponents;

// Storage
// 存储（Storage）
const static auto& getStorage = privilegeSetLogin;
const static auto& headStorage = privilegeSetLogin;
const static auto& patchStorage = privilegeSetConfigureComponents;
const static auto& postStorage = privilegeSetConfigureComponents;
const static auto& putStorage = privilegeSetConfigureComponents;
const static auto& deleteStorage = privilegeSetConfigureComponents;

// StorageCollection
// 存储集合（StorageCollection）
const static auto& getStorageCollection = privilegeSetLogin;
const static auto& headStorageCollection = privilegeSetLogin;
const static auto& patchStorageCollection = privilegeSetConfigureComponents;
const static auto& postStorageCollection = privilegeSetConfigureComponents;
const static auto& putStorageCollection = privilegeSetConfigureComponents;
const static auto& deleteStorageCollection = privilegeSetConfigureComponents;

// StorageController
// 存储控制器（StorageController）
const static auto& getStorageController = privilegeSetLogin;
const static auto& headStorageController = privilegeSetLogin;
const static auto& patchStorageController = privilegeSetConfigureComponents;
const static auto& postStorageController = privilegeSetConfigureComponents;
const static auto& putStorageController = privilegeSetConfigureComponents;
const static auto& deleteStorageController = privilegeSetConfigureComponents;

// StorageControllerCollection
// 存储控制器集合（StorageControllerCollection）
const static auto& getStorageControllerCollection = privilegeSetLogin;
const static auto& headStorageControllerCollection = privilegeSetLogin;
const static auto& patchStorageControllerCollection = privilegeSetConfigureComponents;
const static auto& postStorageControllerCollection = privilegeSetConfigureComponents;
const static auto& putStorageControllerCollection = privilegeSetConfigureComponents;
const static auto& deleteStorageControllerCollection = privilegeSetConfigureComponents;

// StorageControllerMetrics
// 存储控制器度量（StorageControllerMetrics）
const static auto& getStorageControllerMetrics = privilegeSetLogin;
const static auto& headStorageControllerMetrics = privilegeSetLogin;
const static auto& patchStorageControllerMetrics = privilegeSetConfigureComponents;
const static auto& postStorageControllerMetrics = privilegeSetConfigureComponents;
const static auto& putStorageControllerMetrics = privilegeSetConfigureComponents;
const static auto& deleteStorageControllerMetrics = privilegeSetConfigureComponents;

// Switch
// 交换机（Switch）
const static auto& getSwitch = privilegeSetLogin;
const static auto& headSwitch = privilegeSetLogin;
const static auto& patchSwitch = privilegeSetConfigureComponents;
const static auto& postSwitch = privilegeSetConfigureComponents;
const static auto& putSwitch = privilegeSetConfigureComponents;
const static auto& deleteSwitch = privilegeSetConfigureComponents;

// SwitchCollection
// 交换机集合（SwitchCollection）
const static auto& getSwitchCollection = privilegeSetLogin;
const static auto& headSwitchCollection = privilegeSetLogin;
const static auto& patchSwitchCollection = privilegeSetConfigureComponents;
const static auto& postSwitchCollection = privilegeSetConfigureComponents;
const static auto& putSwitchCollection = privilegeSetConfigureComponents;
const static auto& deleteSwitchCollection = privilegeSetConfigureComponents;

// SwitchMetrics
// 交换机度量（SwitchMetrics）
const static auto& getSwitchMetrics = privilegeSetLogin;
const static auto& headSwitchMetrics = privilegeSetLogin;
const static auto& patchSwitchMetrics = privilegeSetConfigureComponents;
const static auto& postSwitchMetrics = privilegeSetConfigureComponents;
const static auto& putSwitchMetrics = privilegeSetConfigureComponents;
const static auto& deleteSwitchMetrics = privilegeSetConfigureComponents;

// Task
// 任务（Task）
const static auto& getTask = privilegeSetLogin;
const static auto& headTask = privilegeSetLogin;
const static auto& patchTask = privilegeSetConfigureManager;
const static auto& putTask = privilegeSetConfigureManager;
const static auto& deleteTask = privilegeSetConfigureManager;
const static auto& postTask = privilegeSetConfigureManager;

// TaskCollection
// 任务集合（TaskCollection）
const static auto& getTaskCollection = privilegeSetLogin;
const static auto& headTaskCollection = privilegeSetLogin;
const static auto& patchTaskCollection = privilegeSetConfigureManager;
const static auto& putTaskCollection = privilegeSetConfigureManager;
const static auto& deleteTaskCollection = privilegeSetConfigureManager;
const static auto& postTaskCollection = privilegeSetConfigureManager;

// TaskService
// 任务服务（TaskService）
const static auto& getTaskService = privilegeSetLogin;
const static auto& headTaskService = privilegeSetLogin;
const static auto& patchTaskService = privilegeSetConfigureManager;
const static auto& putTaskService = privilegeSetConfigureManager;
const static auto& deleteTaskService = privilegeSetConfigureManager;
const static auto& postTaskService = privilegeSetConfigureManager;

// TelemetryService
// 遥测服务（TelemetryService）
const static auto& getTelemetryService = privilegeSetLogin;
const static auto& headTelemetryService = privilegeSetLogin;
const static auto& patchTelemetryService = privilegeSetConfigureManager;
const static auto& putTelemetryService = privilegeSetConfigureManager;
const static auto& deleteTelemetryService = privilegeSetConfigureManager;
const static auto& postTelemetryService = privilegeSetConfigureManager;

// Thermal
// 热管理（Thermal）
const static auto& getThermal = privilegeSetLogin;
const static auto& headThermal = privilegeSetLogin;
const static auto& patchThermal = privilegeSetConfigureManager;
const static auto& putThermal = privilegeSetConfigureManager;
const static auto& deleteThermal = privilegeSetConfigureManager;
const static auto& postThermal = privilegeSetConfigureManager;

// ThermalEquipment
// 热管理设备（ThermalEquipment）
const static auto& getThermalEquipment = privilegeSetLogin;
const static auto& headThermalEquipment = privilegeSetLogin;
const static auto& patchThermalEquipment = privilegeSetConfigureManager;
const static auto& putThermalEquipment = privilegeSetConfigureManager;
const static auto& deleteThermalEquipment = privilegeSetConfigureManager;
const static auto& postThermalEquipment = privilegeSetConfigureManager;

// ThermalMetrics
// 热度量（ThermalMetrics）
const static auto& getThermalMetrics = privilegeSetLogin;
const static auto& headThermalMetrics = privilegeSetLogin;
const static auto& patchThermalMetrics = privilegeSetConfigureManager;
const static auto& putThermalMetrics = privilegeSetConfigureManager;
const static auto& deleteThermalMetrics = privilegeSetConfigureManager;
const static auto& postThermalMetrics = privilegeSetConfigureManager;

// ThermalSubsystem
// 热管理子系统（ThermalSubsystem）
const static auto& getThermalSubsystem = privilegeSetLogin;
const static auto& headThermalSubsystem = privilegeSetLogin;
const static auto& patchThermalSubsystem = privilegeSetConfigureManager;
const static auto& putThermalSubsystem = privilegeSetConfigureManager;
const static auto& deleteThermalSubsystem = privilegeSetConfigureManager;
const static auto& postThermalSubsystem = privilegeSetConfigureManager;

// Triggers
// 触发器（Triggers）
const static auto& getTriggers = privilegeSetLogin;
const static auto& headTriggers = privilegeSetLogin;
const static auto& patchTriggers = privilegeSetConfigureManager;
const static auto& putTriggers = privilegeSetConfigureManager;
const static auto& deleteTriggers = privilegeSetConfigureManager;
const static auto& postTriggers = privilegeSetConfigureManager;

// TriggersCollection
// 触发器集合（TriggersCollection）
const static auto& getTriggersCollection = privilegeSetLogin;
const static auto& headTriggersCollection = privilegeSetLogin;
const static auto& patchTriggersCollection = privilegeSetConfigureManager;
const static auto& putTriggersCollection = privilegeSetConfigureManager;
const static auto& deleteTriggersCollection = privilegeSetConfigureManager;
const static auto& postTriggersCollection = privilegeSetConfigureManager;

// TrustedComponent
// 可信组件（TrustedComponent）
const static auto& getTrustedComponent = privilegeSetLogin;
const static auto& headTrustedComponent = privilegeSetLogin;
const static auto& patchTrustedComponent = privilegeSetConfigureManager;
const static auto& putTrustedComponent = privilegeSetConfigureManager;
const static auto& deleteTrustedComponent = privilegeSetConfigureManager;
const static auto& postTrustedComponent = privilegeSetConfigureManager;

// TrustedComponentCollection
// 可信组件集合（TrustedComponentCollection）
const static auto& getTrustedComponentCollection = privilegeSetLogin;
const static auto& headTrustedComponentCollection = privilegeSetLogin;
const static auto& patchTrustedComponentCollection = privilegeSetConfigureManager;
const static auto& putTrustedComponentCollection = privilegeSetConfigureManager;
const static auto& deleteTrustedComponentCollection = privilegeSetConfigureManager;
const static auto& postTrustedComponentCollection = privilegeSetConfigureManager;

// UpdateService
// 更新服务（UpdateService）
const static auto& getUpdateService = privilegeSetLogin;
const static auto& headUpdateService = privilegeSetLogin;
const static auto& patchUpdateService = privilegeSetConfigureComponents;
const static auto& postUpdateService = privilegeSetConfigureComponents;
const static auto& putUpdateService = privilegeSetConfigureComponents;
const static auto& deleteUpdateService = privilegeSetConfigureComponents;

// USBController
// USB 控制器（USBController）
const static auto& getUSBController = privilegeSetLogin;
const static auto& headUSBController = privilegeSetLogin;
const static auto& patchUSBController = privilegeSetConfigureComponents;
const static auto& putUSBController = privilegeSetConfigureComponents;
const static auto& deleteUSBController = privilegeSetConfigureComponents;
const static auto& postUSBController = privilegeSetConfigureComponents;

// USBControllerCollection
// USB 控制器集合（USBControllerCollection）
const static auto& getUSBControllerCollection = privilegeSetLogin;
const static auto& headUSBControllerCollection = privilegeSetLogin;
const static auto& patchUSBControllerCollection = privilegeSetConfigureComponents;
const static auto& putUSBControllerCollection = privilegeSetConfigureComponents;
const static auto& deleteUSBControllerCollection = privilegeSetConfigureComponents;
const static auto& postUSBControllerCollection = privilegeSetConfigureComponents;

// VCATEntry
// VCAT 条目（VCATEntry）
const static auto& getVCATEntry = privilegeSetLogin;
const static auto& headVCATEntry = privilegeSetLogin;
const static auto& patchVCATEntry = privilegeSetConfigureComponents;
const static auto& putVCATEntry = privilegeSetConfigureComponents;
const static auto& deleteVCATEntry = privilegeSetConfigureComponents;
const static auto& postVCATEntry = privilegeSetConfigureComponents;

// VCATEntryCollection
// VCAT 条目集合（VCATEntryCollection）
const static auto& getVCATEntryCollection = privilegeSetLogin;
const static auto& headVCATEntryCollection = privilegeSetLogin;
const static auto& patchVCATEntryCollection = privilegeSetConfigureComponents;
const static auto& putVCATEntryCollection = privilegeSetConfigureComponents;
const static auto& deleteVCATEntryCollection = privilegeSetConfigureComponents;
const static auto& postVCATEntryCollection = privilegeSetConfigureComponents;

// VLanNetworkInterface
// VLAN 网络接口（VLanNetworkInterface）
const static auto& getVLanNetworkInterface = privilegeSetLogin;
const static auto& headVLanNetworkInterface = privilegeSetLogin;
const static auto& patchVLanNetworkInterface = privilegeSetConfigureManager;
const static auto& putVLanNetworkInterface = privilegeSetConfigureManager;
const static auto& deleteVLanNetworkInterface = privilegeSetConfigureManager;
const static auto& postVLanNetworkInterface = privilegeSetConfigureManager;

// VLanNetworkInterfaceCollection
// VLAN 网络接口集合（VLanNetworkInterfaceCollection）
const static auto& getVLanNetworkInterfaceCollection = privilegeSetLogin;
const static auto& headVLanNetworkInterfaceCollection = privilegeSetLogin;
const static auto& patchVLanNetworkInterfaceCollection = privilegeSetConfigureManager;
const static auto& putVLanNetworkInterfaceCollection = privilegeSetConfigureManager;
const static auto& deleteVLanNetworkInterfaceCollection = privilegeSetConfigureManager;
const static auto& postVLanNetworkInterfaceCollection = privilegeSetConfigureManager;

// VirtualMedia
// 虚拟介质（VirtualMedia）
const static auto& getVirtualMedia = privilegeSetLogin;
const static auto& headVirtualMedia = privilegeSetLogin;
const static auto& patchVirtualMedia = privilegeSetConfigureManager;
const static auto& putVirtualMedia = privilegeSetConfigureManager;
const static auto& deleteVirtualMedia = privilegeSetConfigureManager;
const static auto& postVirtualMedia = privilegeSetConfigureManager;

// VirtualMediaCollection
// 虚拟介质集合（VirtualMediaCollection）
const static auto& getVirtualMediaCollection = privilegeSetLogin;
const static auto& headVirtualMediaCollection = privilegeSetLogin;
const static auto& patchVirtualMediaCollection = privilegeSetConfigureManager;
const static auto& putVirtualMediaCollection = privilegeSetConfigureManager;
const static auto& deleteVirtualMediaCollection = privilegeSetConfigureManager;
const static auto& postVirtualMediaCollection = privilegeSetConfigureManager;

// Volume
// 卷（Volume）
const static auto& getVolume = privilegeSetLogin;
const static auto& headVolume = privilegeSetLogin;
const static auto& patchVolume = privilegeSetConfigureComponents;
const static auto& postVolume = privilegeSetConfigureComponents;
const static auto& putVolume = privilegeSetConfigureComponents;
const static auto& deleteVolume = privilegeSetConfigureComponents;

// VolumeCollection
// 卷集合（VolumeCollection）
const static auto& getVolumeCollection = privilegeSetLogin;
const static auto& headVolumeCollection = privilegeSetLogin;
const static auto& patchVolumeCollection = privilegeSetConfigureComponents;
const static auto& postVolumeCollection = privilegeSetConfigureComponents;
const static auto& putVolumeCollection = privilegeSetConfigureComponents;
const static auto& deleteVolumeCollection = privilegeSetConfigureComponents;

// Zone
// 区域（Zone）
const static auto& getZone = privilegeSetLogin;
const static auto& headZone = privilegeSetLogin;
const static auto& patchZone = privilegeSetConfigureComponents;
const static auto& postZone = privilegeSetConfigureComponents;
const static auto& putZone = privilegeSetConfigureComponents;
const static auto& deleteZone = privilegeSetConfigureComponents;

// ZoneCollection
// 区域集合（ZoneCollection）
const static auto& getZoneCollection = privilegeSetLogin;
const static auto& headZoneCollection = privilegeSetLogin;
const static auto& patchZoneCollection = privilegeSetConfigureComponents;
const static auto& postZoneCollection = privilegeSetConfigureComponents;
const static auto& putZoneCollection = privilegeSetConfigureComponents;
const static auto& deleteZoneCollection = privilegeSetConfigureComponents;

} // namespace redfish::privileges
// clang-format on
