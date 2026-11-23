// SPDX-License-Identifier: Apache-2.0
// SPDX-FileCopyrightText: Copyright OpenBMC Authors
// SPDX-FileCopyrightText: Copyright 2018 Intel Corporation
#pragma once

#include "bmcweb_config.h"

#include "app.hpp"
#include "async_resp.hpp"
#include "credential_pipe.hpp"
#include "dbus_singleton.hpp"
#include "dbus_utility.hpp"
#include "error_messages.hpp"
#include "generated/enums/virtual_media.hpp"
#include "http_request.hpp"
#include "logging.hpp"
#include "query.hpp"
#include "registries/privilege_registry.hpp"
#include "utils/json_utils.hpp"

#include <boost/beast/http/verb.hpp>
#include <boost/system/result.hpp>
#include <boost/url/format.hpp>
#include <boost/url/parse.hpp>
#include <boost/url/url_view.hpp>
#include <boost/url/url_view_base.hpp>
#include <sdbusplus/message/native_types.hpp>

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <functional>
#include <memory>
#include <optional>
#include <ranges>
#include <string>
#include <string_view>
#include <utility>
#include <variant>

namespace redfish
{

enum class VmMode
{
    Invalid,
    Legacy,
    Proxy
};

inline VmMode parseObjectPathAndGetMode(const sdbusplus::object_path& itemPath,
                                        const std::string& resName)
{
    std::string thisPath = itemPath.filename();
    BMCWEB_LOG_DEBUG("Filename: {}, ThisPath: {}", itemPath.str, thisPath);

    if (thisPath.empty())
    {
        return VmMode::Invalid;
    }

    if (thisPath != resName)
    {
        return VmMode::Invalid;
    }

    auto mode = itemPath.parent_path();
    auto type = mode.parent_path();

    if (mode.filename().empty() || type.filename().empty())
    {
        return VmMode::Invalid;
    }

    if (type.filename() != "VirtualMedia")
    {
        return VmMode::Invalid;
    }
    std::string modeStr = mode.filename();
    if (modeStr == "Legacy")
    {
        return VmMode::Legacy;
    }
    if (modeStr == "Proxy")
    {
        return VmMode::Proxy;
    }
    return VmMode::Invalid;
}

using CheckItemHandler =
    std::function<void(const std::string& service, const std::string& resName,
                       const std::shared_ptr<bmcweb::AsyncResp>&,
                       const std::pair<sdbusplus::object_path,
                                       dbus::utility::DBusInterfacesMap>&)>;

inline void findAndParseObject(
    const std::string& service, const std::string& resName,
    const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
    CheckItemHandler&& handler)
{
    sdbusplus::object_path path("/xyz/openbmc_project/VirtualMedia");
    dbus::utility::getManagedObjects(
        service, path,
        // ast-grep-ignore: long-lambda
        // ast-grep-ignore: long-lambda（ast-grep 工具指令：忽略过长的 lambda 表达式）
        [service, resName, asyncResp, handler = std::move(handler)](
            const boost::system::error_code& ec,
            const dbus::utility::ManagedObjectType& subtree) {
            if (ec)
            {
                BMCWEB_LOG_DEBUG("DBUS response error");

                return;
            }

            for (const auto& item : subtree)
            {
                VmMode mode = parseObjectPathAndGetMode(item.first, resName);
                if (mode != VmMode::Invalid)
                {
                    handler(service, resName, asyncResp, item);
                    return;
                }
            }

            BMCWEB_LOG_DEBUG("Parent item not found");
            messages::resourceNotFound(asyncResp->res, "VirtualMedia", resName);
        });
}

/**
 * @brief Function extracts transfer protocol name from URI.
 */
/* @brief 从 URI 中提取传输协议名称的函数。 */
inline virtual_media::TransferProtocolType getTransferProtocolTypeFromUri(
    const std::string& imageUri)
{
    boost::system::result<boost::urls::url_view> url =
        boost::urls::parse_uri(imageUri);
    if (url)
    {
        std::string_view scheme = url->scheme();
        if (scheme == "smb")
        {
            return virtual_media::TransferProtocolType::CIFS;
        }
        if (scheme == "https")
        {
            return virtual_media::TransferProtocolType::HTTPS;
        }
    }

    return virtual_media::TransferProtocolType::Invalid;
}

/**
 * @brief Read all known properties from VM object interfaces
 */
/* @brief 从 VM 对象接口读取所有已知属性 */
inline void vmParseInterfaceObject(
    const dbus::utility::DBusInterfacesMap& interfaces,
    const std::shared_ptr<bmcweb::AsyncResp>& asyncResp)
{
    for (const auto& [interface, values] : interfaces)
    {
        if (interface == "xyz.openbmc_project.VirtualMedia.MountPoint")
        {
            for (const auto& [property, value] : values)
            {
                if (property == "EndpointId")
                {
                    const std::string* endpointIdValue =
                        std::get_if<std::string>(&value);
                    if (endpointIdValue == nullptr)
                    {
                        continue;
                    }
                    if (!endpointIdValue->empty())
                    {
                        // Proxy mode
                        // 代理（Proxy）模式
                        asyncResp->res
                            .jsonValue["Oem"]["OpenBMC"]["WebSocketEndpoint"] =
                            *endpointIdValue;
                        asyncResp->res.jsonValue["TransferProtocolType"] =
                            virtual_media::TransferProtocolType::OEM;
                    }
                }
                if (property == "ImageURL")
                {
                    const std::string* imageUrlValue =
                        std::get_if<std::string>(&value);
                    if (imageUrlValue != nullptr && !imageUrlValue->empty())
                    {
                        std::filesystem::path filePath = *imageUrlValue;
                        if (!filePath.has_filename())
                        {
                            // this will handle https share, which not
                            // necessarily has to have filename given.
                            // 这将处理 https 共享，它不
                            // 一定需要给出文件名。
                            asyncResp->res.jsonValue["ImageName"] = "";
                        }
                        else
                        {
                            asyncResp->res.jsonValue["ImageName"] =
                                filePath.filename();
                        }

                        asyncResp->res.jsonValue["Image"] = *imageUrlValue;
                        virtual_media::TransferProtocolType transferProtocol =
                            getTransferProtocolTypeFromUri(*imageUrlValue);
                        if (transferProtocol !=
                            virtual_media::TransferProtocolType::Invalid)
                        {
                            asyncResp->res.jsonValue["TransferProtocolType"] =
                                transferProtocol;
                        }

                        asyncResp->res.jsonValue["ConnectedVia"] =
                            virtual_media::ConnectedVia::URI;
                    }
                }
                if (property == "WriteProtected")
                {
                    const bool* writeProtectedValue = std::get_if<bool>(&value);
                    if (writeProtectedValue != nullptr)
                    {
                        asyncResp->res.jsonValue["WriteProtected"] =
                            *writeProtectedValue;
                    }
                }
            }
        }
        if (interface == "xyz.openbmc_project.VirtualMedia.Process")
        {
            for (const auto& [property, value] : values)
            {
                if (property == "Active")
                {
                    const bool* activeValue = std::get_if<bool>(&value);
                    if (activeValue == nullptr)
                    {
                        BMCWEB_LOG_DEBUG("Value Active not found");
                        return;
                    }
                    asyncResp->res.jsonValue["Inserted"] = *activeValue;

                    if (*activeValue)
                    {
                        asyncResp->res.jsonValue["ConnectedVia"] =
                            virtual_media::ConnectedVia::Applet;
                    }
                }
            }
        }
    }
}

/**
 * @brief Fill template for Virtual Media Item.
 */
/* @brief 填充 Virtual Media Item 的模板。 */
inline nlohmann::json vmItemTemplate(const std::string& name,
                                     const std::string& resName)
{
    nlohmann::json item;
    item["@odata.id"] = boost::urls::format(
        "/redfish/v1/Managers/{}/VirtualMedia/{}", name, resName);

    item["@odata.type"] = "#VirtualMedia.v1_3_0.VirtualMedia";
    item["Name"] = "Virtual Removable Media";
    item["Id"] = resName;
    item["WriteProtected"] = true;
    item["ConnectedVia"] = virtual_media::ConnectedVia::NotConnected;
    item["MediaTypes"] = nlohmann::json::array_t({"CD", "USBStick"});
    item["TransferMethod"] = virtual_media::TransferMethod::Stream;
    item["Oem"]["OpenBMC"]["@odata.type"] =
        "#OpenBMCVirtualMedia.v1_0_0.VirtualMedia";
    item["Oem"]["OpenBMC"]["@odata.id"] = boost::urls::format(
        "/redfish/v1/Managers/{}/VirtualMedia/{}#/Oem/OpenBMC", name, resName);

    return item;
}

/**
 *  @brief Fills collection data
 */
/*  @brief 填充集合数据 */
inline void getVmResourceList(std::shared_ptr<bmcweb::AsyncResp> asyncResp,
                              const std::string& service,
                              const std::string& name)
{
    BMCWEB_LOG_DEBUG("Get available Virtual Media resources.");
    sdbusplus::object_path objPath("/xyz/openbmc_project/VirtualMedia");
    dbus::utility::getManagedObjects(
        service, objPath,
        // ast-grep-ignore: long-lambda
        // ast-grep-ignore: long-lambda（ast-grep 工具指令：忽略过长的 lambda 表达式）
        [name, asyncResp{std::move(asyncResp)}](
            const boost::system::error_code& ec,
            const dbus::utility::ManagedObjectType& subtree) {
            if (ec)
            {
                BMCWEB_LOG_DEBUG("DBUS response error");
                return;
            }
            nlohmann::json& members = asyncResp->res.jsonValue["Members"];
            members = nlohmann::json::array();

            for (const auto& object : subtree)
            {
                nlohmann::json item;
                std::string path = object.first.filename();
                if (path.empty())
                {
                    continue;
                }

                item["@odata.id"] = boost::urls::format(
                    "/redfish/v1/Managers/{}/VirtualMedia/{}", name, path);
                members.emplace_back(std::move(item));
            }
            asyncResp->res.jsonValue["Members@odata.count"] = members.size();
        });
}

inline void afterGetVmData(
    const std::string& name, const std::string& /*service*/,
    const std::string& resName,
    const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
    const std::pair<sdbusplus::object_path, dbus::utility::DBusInterfacesMap>&
        item)
{
    VmMode mode = parseObjectPathAndGetMode(item.first, resName);
    if (mode == VmMode::Invalid)
    {
        return;
    }

    asyncResp->res.jsonValue = vmItemTemplate(name, resName);

    // Check if dbus path is Legacy type
    // 检查 dbus 路径是否为 Legacy 类型
    if (mode == VmMode::Legacy)
    {
        asyncResp->res.jsonValue["Actions"]["#VirtualMedia.InsertMedia"]
                                ["target"] = boost::urls::format(
            "/redfish/v1/Managers/{}/VirtualMedia/{}/Actions/VirtualMedia.InsertMedia",
            name, resName);
    }

    vmParseInterfaceObject(item.second, asyncResp);

    asyncResp->res.jsonValue["Actions"]["#VirtualMedia.EjectMedia"]
                            ["target"] = boost::urls::format(
        "/redfish/v1/Managers/{}/VirtualMedia/{}/Actions/VirtualMedia.EjectMedia",
        name, resName);
}

/**
 *  @brief Fills data for specific resource
 */
/*  @brief 填充特定资源的数据 */
inline void getVmData(const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
                      const std::string& service, const std::string& name,
                      const std::string& resName)
{
    BMCWEB_LOG_DEBUG("Get Virtual Media resource data.");

    findAndParseObject(service, resName, asyncResp,
                       std::bind_front(afterGetVmData, name));
}

/**
 * @brief Transfer protocols supported for InsertMedia action.
 *
 */
/* @brief InsertMedia 动作支持的传输协议。 */
enum class TransferProtocol
{
    https,
    smb,
    invalid
};

/**
 * @brief Function extracts transfer protocol type from URI.
 *
 */
/* @brief 从 URI 中提取传输协议类型的函数。 */
inline std::optional<TransferProtocol> getTransferProtocolFromUri(
    const boost::urls::url_view_base& imageUri)
{
    std::string_view scheme = imageUri.scheme();
    if (scheme == "smb")
    {
        return TransferProtocol::smb;
    }
    if (scheme == "https")
    {
        return TransferProtocol::https;
    }
    if (!scheme.empty())
    {
        return TransferProtocol::invalid;
    }

    return {};
}

/**
 * @brief Function convert transfer protocol from string param.
 *
 */
/* @brief 从字符串参数转换传输协议的函数。 */
inline std::optional<TransferProtocol> getTransferProtocolFromParam(
    const std::optional<std::string>& transferProtocolType)
{
    if (!transferProtocolType)
    {
        return {};
    }

    if (*transferProtocolType == "CIFS")
    {
        return TransferProtocol::smb;
    }

    if (*transferProtocolType == "HTTPS")
    {
        return TransferProtocol::https;
    }

    return TransferProtocol::invalid;
}

/**
 * @brief Function extends URI with transfer protocol type.
 *
 */
/* @brief 用传输协议类型扩展 URI 的函数。 */
inline std::string getUriWithTransferProtocol(
    const std::string& imageUri, const TransferProtocol& transferProtocol)
{
    if (transferProtocol == TransferProtocol::smb)
    {
        return "smb://" + imageUri;
    }

    if (transferProtocol == TransferProtocol::https)
    {
        return "https://" + imageUri;
    }

    return imageUri;
}

struct InsertMediaActionParams
{
    std::optional<std::string> imageUrl;
    std::optional<std::string> userName;
    std::optional<std::string> password;
    std::optional<std::string> transferMethod;
    std::optional<std::string> transferProtocolType;
    std::optional<bool> writeProtected = true;
    std::optional<bool> inserted;
};

/**
 * @brief Function transceives data with dbus directly.
 *
 * All BMC state properties will be retrieved before sending reset request.
 */
/* @brief 该函数直接与 dbus 收发数据。
 * 在发送复位请求之前将检索所有 BMC 状态属性。 */
inline void doMountVmLegacy(const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
                            const std::string& service, const std::string& name,
                            const std::string& imageUrl, bool rw,
                            std::string&& userName, std::string&& password)
{
    if (userName.contains('\0'))
    {
        messages::actionParameterValueFormatError(
            asyncResp->res, userName, "Username", "VirtualMedia.InsertMedia");
        return;
    }
    constexpr uint64_t limit = 512;
    if (userName.size() > limit)
    {
        messages::stringValueTooLong(asyncResp->res, userName, limit);
        return;
    }
    if (password.contains('\0'))
    {
        messages::actionParameterValueFormatError(
            asyncResp->res, password, "Password", "VirtualMedia.InsertMedia");
        return;
    }
    if (password.size() > limit)
    {
        messages::stringValueTooLong(asyncResp->res, password, limit);
        return;
    }
    // Open pipe
    // 打开管道（pipe）
    std::shared_ptr<CredentialsPipe> secretPipe =
        std::make_shared<CredentialsPipe>(
            crow::connections::systemBus->get_io_context());
    int fd = secretPipe->releaseFd();

    // Pass secret over pipe
    // 通过管道传递密钥（secret）
    secretPipe->asyncWrite(
        std::move(userName), std::move(password),
        [asyncResp,
         secretPipe](const boost::system::error_code& ec, std::size_t) {
            if (ec)
            {
                BMCWEB_LOG_ERROR("Failed to pass secret: {}", ec);
                messages::internalError(asyncResp->res);
            }
        });

    sdbusplus::message::unix_fd unixFd(fd);

    sdbusplus::object_path path("/xyz/openbmc_project/VirtualMedia/Legacy");
    path /= name;
    dbus::utility::async_method_call(
        asyncResp,
        // ast-grep-ignore: long-lambda
        // ast-grep-ignore: long-lambda（ast-grep 工具指令：忽略过长的 lambda 表达式）
        [asyncResp](const boost::system::error_code& ec, bool success) {
            if (ec)
            {
                BMCWEB_LOG_ERROR("Bad D-Bus request error: {}", ec);
                messages::internalError(asyncResp->res);
                return;
            }
            if (!success)
            {
                BMCWEB_LOG_ERROR("Service responded with error");
                messages::internalError(asyncResp->res);
            }
        },
        service, path.str, "xyz.openbmc_project.VirtualMedia.Legacy", "Mount",
        imageUrl, rw, unixFd);
}

/**
 * @brief Function validate parameters of insert media request.
 *
 */
/* @brief 校验 InsertMedia 请求参数的函数。 */
inline void validateParams(const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
                           const std::string& service,
                           const std::string& resName,
                           InsertMediaActionParams& actionParams)
{
    BMCWEB_LOG_DEBUG("Validation started");
    // required param imageUrl must not be empty
    // 必填参数 imageUrl 不能为空
    if (!actionParams.imageUrl)
    {
        BMCWEB_LOG_ERROR("Request action parameter Image is empty.");

        messages::propertyValueFormatError(asyncResp->res, "<empty>", "Image");

        return;
    }

    // optional param inserted must be true
    // 可选参数 inserted 必须为 true
    if (actionParams.inserted && !*actionParams.inserted)
    {
        BMCWEB_LOG_ERROR(
            "Request action optional parameter Inserted must be true.");

        messages::actionParameterNotSupported(asyncResp->res, "Inserted",
                                              "InsertMedia");

        return;
    }

    // optional param transferMethod must be stream
    // 可选参数 transferMethod 必须为 stream
    if (actionParams.transferMethod &&
        (*actionParams.transferMethod != "Stream"))
    {
        BMCWEB_LOG_ERROR("Request action optional parameter "
                         "TransferMethod must be Stream.");

        messages::actionParameterNotSupported(asyncResp->res, "TransferMethod",
                                              "InsertMedia");

        return;
    }
    boost::system::result<boost::urls::url_view> url =
        boost::urls::parse_uri(*actionParams.imageUrl);
    if (!url)
    {
        messages::actionParameterValueFormatError(
            asyncResp->res, *actionParams.imageUrl, "Image", "InsertMedia");
        return;
    }
    std::optional<TransferProtocol> uriTransferProtocolType =
        getTransferProtocolFromUri(*url);

    std::optional<TransferProtocol> paramTransferProtocolType =
        getTransferProtocolFromParam(actionParams.transferProtocolType);

    // ImageUrl does not contain valid protocol type
    // ImageUrl 不包含有效的协议类型
    if (uriTransferProtocolType &&
        *uriTransferProtocolType == TransferProtocol::invalid)
    {
        BMCWEB_LOG_ERROR("Request action parameter ImageUrl must "
                         "contain specified protocol type from list: "
                         "(smb, https).");

        messages::resourceAtUriInUnknownFormat(asyncResp->res, *url);

        return;
    }

    // transferProtocolType should contain value from list
    // transferProtocolType 应包含列表中的值
    if (paramTransferProtocolType &&
        *paramTransferProtocolType == TransferProtocol::invalid)
    {
        BMCWEB_LOG_ERROR("Request action parameter TransferProtocolType "
                         "must be provided with value from list: "
                         "(CIFS, HTTPS).");

        messages::propertyValueNotInList(
            asyncResp->res, actionParams.transferProtocolType.value_or(""),
            "TransferProtocolType");
        return;
    }

    // valid transfer protocol not provided either with URI nor param
    // URI 和参数中都没有提供有效的传输协议
    if (!uriTransferProtocolType && !paramTransferProtocolType)
    {
        BMCWEB_LOG_ERROR("Request action parameter ImageUrl must "
                         "contain specified protocol type or param "
                         "TransferProtocolType must be provided.");

        messages::resourceAtUriInUnknownFormat(asyncResp->res, *url);

        return;
    }

    // valid transfer protocol provided both with URI and param
    // URI 和参数中都提供了有效的传输协议
    if (paramTransferProtocolType && uriTransferProtocolType)
    {
        // check if protocol is the same for URI and param
        // 检查 URI 和参数中的协议是否相同
        if (*paramTransferProtocolType != *uriTransferProtocolType)
        {
            BMCWEB_LOG_ERROR("Request action parameter "
                             "TransferProtocolType must  contain the "
                             "same protocol type as protocol type "
                             "provided with param imageUrl.");

            messages::actionParameterValueTypeError(
                asyncResp->res, actionParams.transferProtocolType.value_or(""),
                "TransferProtocolType", "InsertMedia");

            return;
        }
    }

    // validation passed, add protocol to URI if needed
    // 校验通过，如需要则将协议添加到 URI
    if (!uriTransferProtocolType && paramTransferProtocolType)
    {
        actionParams.imageUrl = getUriWithTransferProtocol(
            *actionParams.imageUrl, *paramTransferProtocolType);
    }

    if (!actionParams.userName)
    {
        actionParams.userName = "";
    }

    if (!actionParams.password)
    {
        actionParams.password = "";
    }

    doMountVmLegacy(asyncResp, service, resName, *actionParams.imageUrl,
                    !(actionParams.writeProtected.value_or(false)),
                    std::move(*actionParams.userName),
                    std::move(*actionParams.password));
}

/**
 * @brief Function transceives data with dbus directly.
 *
 * All BMC state properties will be retrieved before sending reset request.
 */
/* @brief 该函数直接与 dbus 收发数据。
 * 在发送复位请求之前将检索所有 BMC 状态属性。 */
inline void doEjectAction(const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
                          const std::string& service, const std::string& name,
                          bool legacy)
{
    // Legacy mount requires parameter with image
    // Legacy 挂载需要带 image 的参数
    if (legacy)
    {
        dbus::utility::async_method_call(
            asyncResp,
            [asyncResp](const boost::system::error_code& ec) {
                if (ec)
                {
                    BMCWEB_LOG_ERROR("Bad D-Bus request error: {}", ec);

                    messages::internalError(asyncResp->res);
                    return;
                }
            },
            service, "/xyz/openbmc_project/VirtualMedia/Legacy/" + name,
            "xyz.openbmc_project.VirtualMedia.Legacy", "Unmount");
    }
    else // proxy
    {
        dbus::utility::async_method_call(
            asyncResp,
            [asyncResp](const boost::system::error_code& ec) {
                if (ec)
                {
                    BMCWEB_LOG_ERROR("Bad D-Bus request error: {}", ec);

                    messages::internalError(asyncResp->res);
                    return;
                }
            },
            service, "/xyz/openbmc_project/VirtualMedia/Proxy/" + name,
            "xyz.openbmc_project.VirtualMedia.Proxy", "Unmount");
    }
}

inline void handleManagersVirtualMediaActionInsertPost(
    crow::App& app, const crow::Request& req,
    const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
    const std::string& name, const std::string& resName)
{
    if (!redfish::setUpRedfishRoute(app, req, asyncResp))
    {
        return;
    }

    constexpr std::string_view action = "VirtualMedia.InsertMedia";
    if (name != BMCWEB_REDFISH_MANAGER_URI_NAME)
    {
        messages::resourceNotFound(asyncResp->res, action, resName);

        return;
    }
    InsertMediaActionParams actionParams;

    // Read obligatory parameters (url of image)
    // 读取必填参数（image 的 url）
    if (!json_util::readJsonAction(                                    //
            req, asyncResp->res,                                       //
            "Image", actionParams.imageUrl,                            //
            "Inserted", actionParams.inserted,                         //
            "Password", actionParams.password,                         //
            "TransferMethod", actionParams.transferMethod,             //
            "TransferProtocolType", actionParams.transferProtocolType, //
            "UserName", actionParams.userName,                         //
            "WriteProtected", actionParams.writeProtected              //
            ))
    {
        return;
    }

    dbus::utility::getDbusObject(
        "/xyz/openbmc_project/VirtualMedia", {},
        // ast-grep-ignore: long-lambda
        // ast-grep-ignore: long-lambda（ast-grep 工具指令：忽略过长的 lambda 表达式）
        [asyncResp, action, actionParams,
         resName](const boost::system::error_code& ec,
                  const dbus::utility::MapperGetObject& getObjectType) mutable {
            if (ec)
            {
                BMCWEB_LOG_ERROR("ObjectMapper::GetObject call failed: {}", ec);
                messages::resourceNotFound(asyncResp->res, action, resName);

                return;
            }

            std::string service = getObjectType.begin()->first;
            BMCWEB_LOG_DEBUG("GetObjectType: {}", service);

            sdbusplus::object_path path("/xyz/openbmc_project/VirtualMedia");
            dbus::utility::getManagedObjects(
                service, path,
                // ast-grep-ignore: long-lambda
                // ast-grep-ignore: long-lambda（ast-grep 工具指令：忽略过长的 lambda 表达式）
                [service, resName, action, actionParams, asyncResp](
                    const boost::system::error_code& ec2,
                    const dbus::utility::ManagedObjectType& subtree) mutable {
                    if (ec2)
                    {
                        // Not possible in proxy mode
                        // 在代理（proxy）模式下不可行
                        BMCWEB_LOG_DEBUG("InsertMedia not "
                                         "allowed in proxy mode");
                        messages::resourceNotFound(asyncResp->res, action,
                                                   resName);

                        return;
                    }
                    for (const auto& object : subtree)
                    {
                        VmMode mode =
                            parseObjectPathAndGetMode(object.first, resName);
                        if (mode == VmMode::Legacy)
                        {
                            validateParams(asyncResp, service, resName,
                                           actionParams);

                            return;
                        }
                    }
                    BMCWEB_LOG_DEBUG("Parent item not found");
                    messages::resourceNotFound(asyncResp->res, "VirtualMedia",
                                               resName);
                });
        });
}

inline void handleManagersVirtualMediaActionEject(
    crow::App& app, const crow::Request& req,
    const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
    const std::string& managerName, const std::string& resName)
{
    if (!redfish::setUpRedfishRoute(app, req, asyncResp))
    {
        return;
    }

    constexpr std::string_view action = "VirtualMedia.EjectMedia";
    if (managerName != BMCWEB_REDFISH_MANAGER_URI_NAME)
    {
        messages::resourceNotFound(asyncResp->res, action, resName);

        return;
    }

    dbus::utility::getDbusObject(
        "/xyz/openbmc_project/VirtualMedia", {},
        // ast-grep-ignore: long-lambda
        // ast-grep-ignore: long-lambda（ast-grep 工具指令：忽略过长的 lambda 表达式）
        [asyncResp, action,
         resName](const boost::system::error_code& ec2,
                  const dbus::utility::MapperGetObject& getObjectType) {
            if (ec2)
            {
                BMCWEB_LOG_ERROR("ObjectMapper::GetObject call failed: {}",
                                 ec2);
                messages::internalError(asyncResp->res);

                return;
            }
            std::string service = getObjectType.begin()->first;
            BMCWEB_LOG_DEBUG("GetObjectType: {}", service);

            sdbusplus::object_path path("/xyz/openbmc_project/VirtualMedia");
            dbus::utility::getManagedObjects(
                service, path,
                // ast-grep-ignore: long-lambda
                // ast-grep-ignore: long-lambda（ast-grep 工具指令：忽略过长的 lambda 表达式）
                [resName, service, action,
                 asyncResp](const boost::system::error_code& ec,
                            const dbus::utility::ManagedObjectType& subtree) {
                    if (ec)
                    {
                        BMCWEB_LOG_ERROR("ObjectMapper : No Service found");
                        messages::resourceNotFound(asyncResp->res, action,
                                                   resName);
                        return;
                    }

                    for (const auto& object : subtree)
                    {
                        VmMode mode =
                            parseObjectPathAndGetMode(object.first, resName);
                        if (mode != VmMode::Invalid)
                        {
                            doEjectAction(asyncResp, service, resName,
                                          mode == VmMode::Legacy);
                            return;
                        }
                    }
                    BMCWEB_LOG_DEBUG("Parent item not found");
                    messages::resourceNotFound(asyncResp->res, "VirtualMedia",
                                               resName);
                });
        });
}

inline void handleManagersVirtualMediaCollectionGet(
    crow::App& app, const crow::Request& req,
    const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
    const std::string& name)
{
    if (!redfish::setUpRedfishRoute(app, req, asyncResp))
    {
        return;
    }
    if (name != BMCWEB_REDFISH_MANAGER_URI_NAME)
    {
        messages::resourceNotFound(asyncResp->res, "VirtualMedia", name);

        return;
    }

    asyncResp->res.jsonValue["@odata.type"] =
        "#VirtualMediaCollection.VirtualMediaCollection";
    asyncResp->res.jsonValue["Name"] = "Virtual Media Services";
    asyncResp->res.jsonValue["@odata.id"] =
        boost::urls::format("/redfish/v1/Managers/{}/VirtualMedia", name);

    dbus::utility::getDbusObject(
        "/xyz/openbmc_project/VirtualMedia", {},
        // ast-grep-ignore: long-lambda
        // ast-grep-ignore: long-lambda（ast-grep 工具指令：忽略过长的 lambda 表达式）
        [asyncResp, name](const boost::system::error_code& ec,
                          const dbus::utility::MapperGetObject& getObjectType) {
            if (ec)
            {
                BMCWEB_LOG_ERROR("ObjectMapper::GetObject call failed: {}", ec);
                messages::internalError(asyncResp->res);

                return;
            }
            std::string service = getObjectType.begin()->first;
            BMCWEB_LOG_DEBUG("GetObjectType: {}", service);

            getVmResourceList(asyncResp, service, name);
        });
}

inline void handleVirtualMediaGet(
    crow::App& app, const crow::Request& req,
    const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
    const std::string& name, const std::string& resName)
{
    if (!redfish::setUpRedfishRoute(app, req, asyncResp))
    {
        return;
    }
    if (name != BMCWEB_REDFISH_MANAGER_URI_NAME)
    {
        messages::resourceNotFound(asyncResp->res, "VirtualMedia", resName);

        return;
    }

    dbus::utility::getDbusObject(
        "/xyz/openbmc_project/VirtualMedia", {},
        // ast-grep-ignore: long-lambda
        // ast-grep-ignore: long-lambda（ast-grep 工具指令：忽略过长的 lambda 表达式）
        [asyncResp, name,
         resName](const boost::system::error_code& ec,
                  const dbus::utility::MapperGetObject& getObjectType) {
            if (ec)
            {
                BMCWEB_LOG_ERROR("ObjectMapper::GetObject call failed: {}", ec);
                messages::internalError(asyncResp->res);

                return;
            }
            std::string service = getObjectType.begin()->first;
            BMCWEB_LOG_DEBUG("GetObjectType: {}", service);

            getVmData(asyncResp, service, name, resName);
        });
}

inline void requestNBDVirtualMediaRoutes(App& app)
{
    BMCWEB_ROUTE(
        app,
        "/redfish/v1/Managers/<str>/VirtualMedia/<str>/Actions/VirtualMedia.InsertMedia/")
        .privileges(redfish::privileges::postVirtualMedia)
        .methods(boost::beast::http::verb::post)(std::bind_front(
            handleManagersVirtualMediaActionInsertPost, std::ref(app)));

    BMCWEB_ROUTE(
        app,
        "/redfish/v1/Managers/<str>/VirtualMedia/<str>/Actions/VirtualMedia.EjectMedia/")
        .privileges(redfish::privileges::postVirtualMedia)
        .methods(boost::beast::http::verb::post)(std::bind_front(
            handleManagersVirtualMediaActionEject, std::ref(app)));

    BMCWEB_ROUTE(app, "/redfish/v1/Managers/<str>/VirtualMedia/")
        .privileges(redfish::privileges::getVirtualMediaCollection)
        .methods(boost::beast::http::verb::get)(std::bind_front(
            handleManagersVirtualMediaCollectionGet, std::ref(app)));

    BMCWEB_ROUTE(app, "/redfish/v1/Managers/<str>/VirtualMedia/<str>/")
        .privileges(redfish::privileges::getVirtualMedia)
        .methods(boost::beast::http::verb::get)(
            std::bind_front(handleVirtualMediaGet, std::ref(app)));
}

} // namespace redfish
