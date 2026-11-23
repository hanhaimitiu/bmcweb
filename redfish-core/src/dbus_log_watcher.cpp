#include "dbus_log_watcher.hpp"

#include "dbus_singleton.hpp"
#include "dbus_utility.hpp"
#include "event_logs_object_type.hpp"
#include "event_service_manager.hpp"
#include "logging.hpp"
#include "telemetry_readings.hpp"
#include "utils/dbus_event_log_entry.hpp"
#include "utils/time_utils.hpp"

#include <sdbusplus/bus/match.hpp>
#include <sdbusplus/message.hpp>
#include <sdbusplus/message/native_types.hpp>

#include <algorithm>
#include <optional>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

namespace redfish
{

bool DbusEventLogMonitor::eventLogObjectFromDBus(
    const dbus::utility::DBusPropertiesMap& map, EventLogObjectsType& event,
    std::string_view tz)
{
    std::optional<DbusEventLogEntry> optEntry =
        fillDbusEventLogEntryFromPropertyMap(map);

    if (!optEntry.has_value())
    {
        BMCWEB_LOG_ERROR(
            "Could not construct event log entry from dbus properties");
        return false;
    }
    DbusEventLogEntry& entry = optEntry.value();
    event.id = std::to_string(entry.Id);
    event.timestamp =
        redfish::time_utils::getDateTimeUintMs(entry.Timestamp, tz);

    // This dbus property is not documented to contain the Redfish Message Id,
    // but can be used as such. As a temporary solution that is sufficient,
    // the event filtering code will drop the event anyways if event.messageId
    // is not valid.
    //
    // This will need resolved before
    // experimental-redfish-dbus-log-subscription is stabilized
    // 该 dbus 属性的文档并未说明其包含 Redfish Message Id，
    // 但可以这样使用。作为临时方案这已经足够，
    // 因为如果 event.messageId 无效，事件过滤代码无论如何都会丢弃该事件。
    // 在
    // experimental-redfish-dbus-log-subscription 稳定之前需要解决这个问题
    event.messageId = entry.Message;

    // The order of 'AdditionalData' is not what's specified in an e.g.
    // busctl call to create the Event Log Entry. So it cannot be used
    // to map to the message args. Leaving this branch here for it to be
    // implemented when the mapping is available
    // 'AdditionalData' 的顺序与例如 busctl 创建
    // Event Log Entry 时指定的顺序不同。因此不能用它来
    // 映射到消息参数。这里保留这个分支，以便在有映射方案
    // 时实现它

    return true;
}

static void dbusEventLogMatchHandlerSingleEntry(
    const dbus::utility::DBusPropertiesMap& map)
{
    std::vector<EventLogObjectsType> eventRecords;
    EventLogObjectsType& event = eventRecords.emplace_back();
    bool success = DbusEventLogMonitor::eventLogObjectFromDBus(map, event);
    if (!success)
    {
        BMCWEB_LOG_ERROR("Could not parse event log entry from dbus");
        return;
    }

    BMCWEB_LOG_DEBUG("Found Event Log Entry Id={}, Timestamp={}, Message={}",
                     event.id, event.timestamp, event.messageId);
    EventServiceManager::sendEventsToSubs(eventRecords);
}

static void onDbusEventLogCreated(sdbusplus::message_t& msg)
{
    BMCWEB_LOG_DEBUG("Handling new DBus Event Log Entry");

    sdbusplus::object_path objectPath;
    dbus::utility::DBusInterfacesMap interfaces;

    msg.read(objectPath, interfaces);

    for (auto& pair : interfaces)
    {
        BMCWEB_LOG_DEBUG("Found dbus interface {}", pair.first);
        if (pair.first == "xyz.openbmc_project.Logging.Entry")
        {
            const dbus::utility::DBusPropertiesMap& map = pair.second;
            dbusEventLogMatchHandlerSingleEntry(map);
        }
    }
}

const std::string propertiesMatchString =
    sdbusplus::match_rules::type::signal() +
    sdbusplus::match_rules::sender("xyz.openbmc_project.Logging") +
    sdbusplus::match_rules::interface("org.freedesktop.DBus.ObjectManager") +
    sdbusplus::match_rules::path("/xyz/openbmc_project/logging") +
    sdbusplus::match_rules::member("InterfacesAdded");

DbusEventLogMonitor::DbusEventLogMonitor() :
    dbusEventLogMonitor(*crow::connections::systemBus, propertiesMatchString,
                        onDbusEventLogCreated)

{}

static void getReadingsForReport(sdbusplus::message_t& msg)
{
    sdbusplus::object_path path(msg.get_path());
    std::string id = path.filename();
    if (id.empty())
    {
        BMCWEB_LOG_ERROR("Failed to get Id from path");
        return;
    }

    std::string interface;
    dbus::utility::DBusPropertiesMap props;
    std::vector<std::string> invalidProps;
    msg.read(interface, props, invalidProps);

    auto found = std::ranges::find_if(props, [](const auto& x) {
        return x.first == "Readings";
    });
    if (found == props.end())
    {
        BMCWEB_LOG_INFO("Failed to get Readings from Report properties");
        return;
    }

    const telemetry::TimestampReadings* readings =
        std::get_if<telemetry::TimestampReadings>(&found->second);
    if (readings == nullptr)
    {
        BMCWEB_LOG_INFO("Failed to get Readings from Report properties");
        return;
    }
    EventServiceManager::sendTelemetryReportToSubs(id, *readings);
}

const std::string telemetryMatchStr =
    "type='signal',member='PropertiesChanged',"
    "interface='org.freedesktop.DBus.Properties',"
    "arg0=xyz.openbmc_project.Telemetry.Report";

DbusTelemetryMonitor::DbusTelemetryMonitor() :
    matchTelemetryMonitor(*crow::connections::systemBus, telemetryMatchStr,
                          getReadingsForReport)
{}
} // namespace redfish
