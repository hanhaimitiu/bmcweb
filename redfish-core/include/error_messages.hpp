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
/*                  请先阅读此警告
 * 这是一个自动生成的头文件，其中包含针对 Redfish DMTF 定义的
 * 消息的定义。
 * 除运行 parse_registries.py 脚本外，请勿修改此注册表。
 * 本文件中包含的定义归 DMTF 所有。
 * 对这些文件的任何修改，
 * 都应先推送到 DMTF
 * github 组织中的相应注册表。 */
// These generated headers are a superset of what is needed.
// clang sees them as an error, so ignore
// NOLINTBEGIN(misc-include-cleaner)
// 这些生成的头文件是所需内容的超集。
// clang 会将其视为错误，因此忽略。
// NOLINTBEGIN(misc-include-cleaner)
#include "http_response.hpp"

#include <boost/url/url_view_base.hpp>
#include <nlohmann/json.hpp>

#include <cstdint>
#include <source_location>
#include <string_view>
// NOLINTEND(misc-include-cleaner)

namespace redfish
{

namespace messages
{
/**
 * @brief Formats Success message into JSON
 * Message body: "The request completed successfully."
 *
 *
 * @returns Message Success formatted to JSON */
/* @brief 将 Success 消息格式化为 JSON
 * 消息正文："请求已成功完成。"
 * @returns 格式化为 JSON 的 Success 消息 */
nlohmann::json::object_t success();

void success(crow::Response& res);

/**
 * @brief Formats GeneralError message into JSON
 * Message body: "A general error has occurred.  See Resolution for information
 * on how to resolve the error, or @Message.ExtendedInfo if Resolution is not
 * provided."
 *
 *
 * @returns Message GeneralError formatted to JSON */
/* @brief 将 GeneralError 消息格式化为 JSON
 * 消息正文："发生了常规错误。请参阅 Resolution
 * 了解如何解决该错误；如果未提供 Resolution
 * ，请参阅 @Message.ExtendedInfo。"
 * @returns 格式化为 JSON 的 GeneralError 消息 */
nlohmann::json::object_t generalError();

void generalError(crow::Response& res);

/**
 * @brief Formats Created message into JSON
 * Message body: "The resource was created successfully."
 *
 *
 * @returns Message Created formatted to JSON */
/* @brief 将 Created 消息格式化为 JSON
 * 消息正文："资源已成功创建。"
 * @returns 格式化为 JSON 的 Created 消息 */
nlohmann::json::object_t created();

void created(crow::Response& res);

/**
 * @brief Formats NoOperation message into JSON
 * Message body: "The request body submitted contain no data to act upon and no
 * changes to the resource took place."
 *
 *
 * @returns Message NoOperation formatted to JSON */
/* @brief 将 NoOperation 消息格式化为 JSON
 * 消息正文："提交的请求正文没有可执行的数据，资源未发生任何更改。
 * "
 * @returns 格式化为 JSON 的 NoOperation 消息 */
nlohmann::json::object_t noOperation();

void noOperation(crow::Response& res);

/**
 * @brief Formats PropertyDuplicate message into JSON
 * Message body: "The property <arg1> was duplicated in the request."
 *
 * @param[in] arg1 Parameter of message that will replace %1 in its body.
 *
 * @returns Message PropertyDuplicate formatted to JSON */
/* @brief 将 PropertyDuplicate 消息格式化为 JSON
 * 消息正文："请求中属性 <arg1> 重复出现。"
 * @param[in] arg1 消息参数，将替换正文中的 %1。
 * @returns 格式化为 JSON 的 PropertyDuplicate 消息 */
nlohmann::json::object_t propertyDuplicate(std::string_view arg1);

void propertyDuplicate(crow::Response& res, std::string_view arg1);

/**
 * @brief Formats PropertyUnknown message into JSON
 * Message body: "The property <arg1> is not in the list of valid properties for
 * the resource."
 *
 * @param[in] arg1 Parameter of message that will replace %1 in its body.
 *
 * @returns Message PropertyUnknown formatted to JSON */
/* @brief 将 PropertyUnknown 消息格式化为 JSON
 * 消息正文："属性 <arg1>
 * 不在该资源的有效属性列表中。"
 * @param[in] arg1 消息参数，将替换正文中的 %1。
 * @returns 格式化为 JSON 的 PropertyUnknown 消息 */
nlohmann::json::object_t propertyUnknown(std::string_view arg1);

void propertyUnknown(crow::Response& res, std::string_view arg1);

/**
 * @brief Formats PropertyValueTypeError message into JSON
 * Message body: "The value <arg1> for the property <arg2> is not a type that
 * the property can accept."
 *
 * @param[in] arg1 Parameter of message that will replace %1 in its body.
 * @param[in] arg2 Parameter of message that will replace %2 in its body.
 *
 * @returns Message PropertyValueTypeError formatted to JSON */
/* @brief 将 PropertyValueTypeError 消息格式化为 JSON
 * 消息正文："属性 <arg2> 的值 <arg1>
 * 的类型不是该属性可接受的类型。"
 * @param[in] arg1 消息参数，将替换正文中的 %1。
 * @param[in] arg2 消息参数，将替换正文中的 %2。
 * @returns 格式化为 JSON 的 PropertyValueTypeError 消息 */
nlohmann::json::object_t propertyValueTypeError(const nlohmann::json& arg1,
                                                std::string_view arg2);

void propertyValueTypeError(crow::Response& res, const nlohmann::json& arg1,
                            std::string_view arg2);

/**
 * @brief Formats PropertyValueFormatError message into JSON
 * Message body: "The value <arg1> for the property <arg2> is not a format that
 * the property can accept."
 *
 * @param[in] arg1 Parameter of message that will replace %1 in its body.
 * @param[in] arg2 Parameter of message that will replace %2 in its body.
 *
 * @returns Message PropertyValueFormatError formatted to JSON */
/* @brief 将 PropertyValueFormatError 消息格式化为 JSON
 * 消息正文："属性 <arg2> 的值 <arg1>
 * 的格式不是该属性可接受的格式。"
 * @param[in] arg1 消息参数，将替换正文中的 %1。
 * @param[in] arg2 消息参数，将替换正文中的 %2。
 * @returns 格式化为 JSON 的 PropertyValueFormatError 消息 */
nlohmann::json::object_t propertyValueFormatError(const nlohmann::json& arg1,
                                                  std::string_view arg2);

void propertyValueFormatError(crow::Response& res, const nlohmann::json& arg1,
                              std::string_view arg2);

/**
 * @brief Formats PropertyValueNotInList message into JSON
 * Message body: "The value <arg1> for the property <arg2> is not in the list of
 * acceptable values."
 *
 * @param[in] arg1 Parameter of message that will replace %1 in its body.
 * @param[in] arg2 Parameter of message that will replace %2 in its body.
 *
 * @returns Message PropertyValueNotInList formatted to JSON */
/* @brief 将 PropertyValueNotInList 消息格式化为 JSON
 * 消息正文："属性 <arg2> 的值 <arg1>
 * 不在可接受的值列表中。"
 * @param[in] arg1 消息参数，将替换正文中的 %1。
 * @param[in] arg2 消息参数，将替换正文中的 %2。
 * @returns 格式化为 JSON 的 PropertyValueNotInList 消息 */
nlohmann::json::object_t propertyValueNotInList(const nlohmann::json& arg1,
                                                std::string_view arg2);

void propertyValueNotInList(crow::Response& res, const nlohmann::json& arg1,
                            std::string_view arg2);

/**
 * @brief Formats PropertyValueOutOfRange message into JSON
 * Message body: "The value <arg1> for the property <arg2> is not in the
 * supported range of acceptable values."
 *
 * @param[in] arg1 Parameter of message that will replace %1 in its body.
 * @param[in] arg2 Parameter of message that will replace %2 in its body.
 *
 * @returns Message PropertyValueOutOfRange formatted to JSON */
/* @brief 将 PropertyValueOutOfRange 消息格式化为 JSON
 * 消息正文："属性 <arg2> 的值 <arg1>
 * 不在支持的可接受值范围内。"
 * @param[in] arg1 消息参数，将替换正文中的 %1。
 * @param[in] arg2 消息参数，将替换正文中的 %2。
 * @returns 格式化为 JSON 的 PropertyValueOutOfRange 消息 */
nlohmann::json::object_t propertyValueOutOfRange(const nlohmann::json& arg1,
                                                 std::string_view arg2);

void propertyValueOutOfRange(crow::Response& res, const nlohmann::json& arg1,
                             std::string_view arg2);

/**
 * @brief Formats PropertyValueError message into JSON
 * Message body: "The value provided for the property <arg1> is not valid."
 *
 * @param[in] arg1 Parameter of message that will replace %1 in its body.
 *
 * @returns Message PropertyValueError formatted to JSON */
/* @brief 将 PropertyValueError 消息格式化为 JSON
 * 消息正文："为属性 <arg1> 提供的值无效。"
 * @param[in] arg1 消息参数，将替换正文中的 %1。
 * @returns 格式化为 JSON 的 PropertyValueError 消息 */
nlohmann::json::object_t propertyValueError(std::string_view arg1);

void propertyValueError(crow::Response& res, std::string_view arg1);

/**
 * @brief Formats PropertyNotWritable message into JSON
 * Message body: "The property <arg1> is a read-only property and cannot be
 * assigned a value."
 *
 * @param[in] arg1 Parameter of message that will replace %1 in its body.
 *
 * @returns Message PropertyNotWritable formatted to JSON */
/* @brief 将 PropertyNotWritable 消息格式化为 JSON
 * 消息正文："属性 <arg1>
 * 是只读属性，不能赋值。"
 * @param[in] arg1 消息参数，将替换正文中的 %1。
 * @returns 格式化为 JSON 的 PropertyNotWritable 消息 */
nlohmann::json::object_t propertyNotWritable(std::string_view arg1);

void propertyNotWritable(crow::Response& res, std::string_view arg1);

/**
 * @brief Formats PropertyNotUpdated message into JSON
 * Message body: "The property <arg1> was not updated due to an internal service
 * error.  The service is still operational."
 *
 * @param[in] arg1 Parameter of message that will replace %1 in its body.
 *
 * @returns Message PropertyNotUpdated formatted to JSON */
/* @brief 将 PropertyNotUpdated 消息格式化为 JSON
 * 消息正文："由于内部服务错误，属性 <arg1>
 * 未更新。服务仍在正常运行。"
 * @param[in] arg1 消息参数，将替换正文中的 %1。
 * @returns 格式化为 JSON 的 PropertyNotUpdated 消息 */
nlohmann::json::object_t propertyNotUpdated(std::string_view arg1);

void propertyNotUpdated(crow::Response& res, std::string_view arg1);

/**
 * @brief Formats PropertyMissing message into JSON
 * Message body: "The property <arg1> is a required property and must be
 * included in the request."
 *
 * @param[in] arg1 Parameter of message that will replace %1 in its body.
 *
 * @returns Message PropertyMissing formatted to JSON */
/* @brief 将 PropertyMissing 消息格式化为 JSON
 * 消息正文："属性 <arg1>
 * 是必填属性，必须包含在请求中。"
 * @param[in] arg1 消息参数，将替换正文中的 %1。
 * @returns 格式化为 JSON 的 PropertyMissing 消息 */
nlohmann::json::object_t propertyMissing(std::string_view arg1);

void propertyMissing(crow::Response& res, std::string_view arg1);

/**
 * @brief Formats MalformedJSON message into JSON
 * Message body: "The request body submitted was malformed JSON and could not be
 * parsed by the receiving service."
 *
 *
 * @returns Message MalformedJSON formatted to JSON */
/* @brief 将 MalformedJSON 消息格式化为 JSON
 * 消息正文："提交的请求正文是格式错误的
 *  JSON，接收方服务无法解析。"
 * @returns 格式化为 JSON 的 MalformedJSON 消息 */
nlohmann::json::object_t malformedJSON();

void malformedJSON(crow::Response& res);

/**
 * @brief Formats InvalidJSON message into JSON
 * Message body: "The request body submitted is invalid JSON starting at line
 * <arg1> and could not be parsed by the receiving service."
 *
 * @param[in] arg1 Parameter of message that will replace %1 in its body.
 *
 * @returns Message InvalidJSON formatted to JSON */
/* @brief 将 InvalidJSON 消息格式化为 JSON
 * 消息正文："提交的请求正文从第 <arg1> 行开始是无效的
 *  JSON，接收方服务无法解析。"
 * @param[in] arg1 消息参数，将替换正文中的 %1。
 * @returns 格式化为 JSON 的 InvalidJSON 消息 */
nlohmann::json::object_t invalidJSON(uint64_t arg1);

void invalidJSON(crow::Response& res, uint64_t arg1);

/**
 * @brief Formats EmptyJSON message into JSON
 * Message body: "The request body submitted contained an empty JSON object and
 * the service is unable to process it."
 *
 *
 * @returns Message EmptyJSON formatted to JSON */
/* @brief 将 EmptyJSON 消息格式化为 JSON
 * 消息正文："提交的请求正文包含空的
 *  JSON 对象，服务无法处理。"
 * @returns 格式化为 JSON 的 EmptyJSON 消息 */
nlohmann::json::object_t emptyJSON();

void emptyJSON(crow::Response& res);

/**
 * @brief Formats ActionNotSupported message into JSON
 * Message body: "The action <arg1> is not supported by the resource."
 *
 * @param[in] arg1 Parameter of message that will replace %1 in its body.
 *
 * @returns Message ActionNotSupported formatted to JSON */
/* @brief 将 ActionNotSupported 消息格式化为 JSON
 * 消息正文："资源不支持操作 <arg1>。"
 * @param[in] arg1 消息参数，将替换正文中的 %1。
 * @returns 格式化为 JSON 的 ActionNotSupported 消息 */
nlohmann::json::object_t actionNotSupported(std::string_view arg1);

void actionNotSupported(crow::Response& res, std::string_view arg1);

/**
 * @brief Formats ActionParameterMissing message into JSON
 * Message body: "The action <arg1> requires the parameter <arg2> to be present
 * in the request body."
 *
 * @param[in] arg1 Parameter of message that will replace %1 in its body.
 * @param[in] arg2 Parameter of message that will replace %2 in its body.
 *
 * @returns Message ActionParameterMissing formatted to JSON */
/* @brief 将 ActionParameterMissing 消息格式化为 JSON
 * 消息正文："操作 <arg1> 要求请求正文中包含参数
 *  <arg2>。"
 * @param[in] arg1 消息参数，将替换正文中的 %1。
 * @param[in] arg2 消息参数，将替换正文中的 %2。
 * @returns 格式化为 JSON 的 ActionParameterMissing 消息 */
nlohmann::json::object_t actionParameterMissing(std::string_view arg1,
                                                std::string_view arg2);

void actionParameterMissing(crow::Response& res, std::string_view arg1,
                            std::string_view arg2);

/**
 * @brief Formats ActionParameterDuplicate message into JSON
 * Message body: "The action <arg1> was submitted with more than one value for
 * the parameter <arg2>."
 *
 * @param[in] arg1 Parameter of message that will replace %1 in its body.
 * @param[in] arg2 Parameter of message that will replace %2 in its body.
 *
 * @returns Message ActionParameterDuplicate formatted to JSON */
/* @brief 将 ActionParameterDuplicate 消息格式化为 JSON
 * 消息正文："提交操作 <arg1> 时，参数
 *  <arg2> 提供了多个值。"
 * @param[in] arg1 消息参数，将替换正文中的 %1。
 * @param[in] arg2 消息参数，将替换正文中的 %2。
 * @returns 格式化为 JSON 的 ActionParameterDuplicate 消息 */
nlohmann::json::object_t actionParameterDuplicate(std::string_view arg1,
                                                  std::string_view arg2);

void actionParameterDuplicate(crow::Response& res, std::string_view arg1,
                              std::string_view arg2);

/**
 * @brief Formats ActionParameterUnknown message into JSON
 * Message body: "The action <arg1> was submitted with the invalid parameter
 * <arg2>."
 *
 * @param[in] arg1 Parameter of message that will replace %1 in its body.
 * @param[in] arg2 Parameter of message that will replace %2 in its body.
 *
 * @returns Message ActionParameterUnknown formatted to JSON */
/* @brief 将 ActionParameterUnknown 消息格式化为 JSON
 * 消息正文："提交操作 <arg1>
 * 时携带了无效参数 <arg2>。"
 * @param[in] arg1 消息参数，将替换正文中的 %1。
 * @param[in] arg2 消息参数，将替换正文中的 %2。
 * @returns 格式化为 JSON 的 ActionParameterUnknown 消息 */
nlohmann::json::object_t actionParameterUnknown(std::string_view arg1,
                                                std::string_view arg2);

void actionParameterUnknown(crow::Response& res, std::string_view arg1,
                            std::string_view arg2);

/**
 * @brief Formats ActionParameterValueTypeError message into JSON
 * Message body: "The value <arg1> for the parameter <arg2> in the action <arg3>
 * is not a type that the parameter can accept."
 *
 * @param[in] arg1 Parameter of message that will replace %1 in its body.
 * @param[in] arg2 Parameter of message that will replace %2 in its body.
 * @param[in] arg3 Parameter of message that will replace %3 in its body.
 *
 * @returns Message ActionParameterValueTypeError formatted to JSON */
/* @brief 将 ActionParameterValueTypeError 消息格式化为 JSON
 * 消息正文："操作 <arg3> 中参数 <arg2>
 * 的值 <arg1> 的类型不是该参数可接受的类型。"
 * @param[in] arg1 消息参数，将替换正文中的 %1。
 * @param[in] arg2 消息参数，将替换正文中的 %2。
 * @param[in] arg3 消息参数，将替换正文中的 %3。
 * @returns 格式化为 JSON 的 ActionParameterValueTypeError 消息 */
nlohmann::json::object_t actionParameterValueTypeError(
    const nlohmann::json& arg1, std::string_view arg2, std::string_view arg3);

void actionParameterValueTypeError(
    crow::Response& res, const nlohmann::json& arg1, std::string_view arg2,
    std::string_view arg3);

/**
 * @brief Formats ActionParameterValueFormatError message into JSON
 * Message body: "The value <arg1> for the parameter <arg2> in the action <arg3>
 * is not a format that the parameter can accept."
 *
 * @param[in] arg1 Parameter of message that will replace %1 in its body.
 * @param[in] arg2 Parameter of message that will replace %2 in its body.
 * @param[in] arg3 Parameter of message that will replace %3 in its body.
 *
 * @returns Message ActionParameterValueFormatError formatted to JSON */
/* @brief 将 ActionParameterValueFormatError 消息格式化为 JSON
 * 消息正文："操作 <arg3> 中参数 <arg2>
 * 的值 <arg1> 的格式不是该参数可接受的格式。"
 * @param[in] arg1 消息参数，将替换正文中的 %1。
 * @param[in] arg2 消息参数，将替换正文中的 %2。
 * @param[in] arg3 消息参数，将替换正文中的 %3。
 * @returns 格式化为 JSON 的 ActionParameterValueFormatError 消息 */
nlohmann::json::object_t actionParameterValueFormatError(
    const nlohmann::json& arg1, std::string_view arg2, std::string_view arg3);

void actionParameterValueFormatError(
    crow::Response& res, const nlohmann::json& arg1, std::string_view arg2,
    std::string_view arg3);

/**
 * @brief Formats ActionParameterValueNotInList message into JSON
 * Message body: "The value <arg1> for the parameter <arg2> in the action <arg3>
 * is not in the list of acceptable values."
 *
 * @param[in] arg1 Parameter of message that will replace %1 in its body.
 * @param[in] arg2 Parameter of message that will replace %2 in its body.
 * @param[in] arg3 Parameter of message that will replace %3 in its body.
 *
 * @returns Message ActionParameterValueNotInList formatted to JSON */
/* @brief 将 ActionParameterValueNotInList 消息格式化为 JSON
 * 消息正文："操作 <arg3> 中参数 <arg2>
 * 的值 <arg1> 不在可接受的值列表中。"
 * @param[in] arg1 消息参数，将替换正文中的 %1。
 * @param[in] arg2 消息参数，将替换正文中的 %2。
 * @param[in] arg3 消息参数，将替换正文中的 %3。
 * @returns 格式化为 JSON 的 ActionParameterValueNotInList 消息 */
nlohmann::json::object_t actionParameterValueNotInList(
    std::string_view arg1, std::string_view arg2, std::string_view arg3);

void actionParameterValueNotInList(crow::Response& res, std::string_view arg1,
                                   std::string_view arg2,
                                   std::string_view arg3);

/**
 * @brief Formats ActionParameterValueOutOfRange message into JSON
 * Message body: "The value <arg1> for the parameter <arg2> in the action <arg3>
 * is not in the supported range of acceptable values."
 *
 * @param[in] arg1 Parameter of message that will replace %1 in its body.
 * @param[in] arg2 Parameter of message that will replace %2 in its body.
 * @param[in] arg3 Parameter of message that will replace %3 in its body.
 *
 * @returns Message ActionParameterValueOutOfRange formatted to JSON */
/* @brief 将 ActionParameterValueOutOfRange 消息格式化为 JSON
 * 消息正文："操作 <arg3> 中参数 <arg2>
 * 的值 <arg1> 不在支持的可接受值范围内。"
 * @param[in] arg1 消息参数，将替换正文中的 %1。
 * @param[in] arg2 消息参数，将替换正文中的 %2。
 * @param[in] arg3 消息参数，将替换正文中的 %3。
 * @returns 格式化为 JSON 的 ActionParameterValueOutOfRange 消息 */
nlohmann::json::object_t actionParameterValueOutOfRange(
    std::string_view arg1, std::string_view arg2, std::string_view arg3);

void actionParameterValueOutOfRange(crow::Response& res, std::string_view arg1,
                                    std::string_view arg2,
                                    std::string_view arg3);

/**
 * @brief Formats ActionParameterValueError message into JSON
 * Message body: "The value for the parameter <arg1> in the action <arg2> is
 * invalid."
 *
 * @param[in] arg1 Parameter of message that will replace %1 in its body.
 * @param[in] arg2 Parameter of message that will replace %2 in its body.
 *
 * @returns Message ActionParameterValueError formatted to JSON */
/* @brief 将 ActionParameterValueError 消息格式化为 JSON
 * 消息正文："操作 <arg2>
 * 中参数 <arg1> 的值无效。"
 * @param[in] arg1 消息参数，将替换正文中的 %1。
 * @param[in] arg2 消息参数，将替换正文中的 %2。
 * @returns 格式化为 JSON 的 ActionParameterValueError 消息 */
nlohmann::json::object_t actionParameterValueError(const nlohmann::json& arg1,
                                                   std::string_view arg2);

void actionParameterValueError(crow::Response& res, const nlohmann::json& arg1,
                               std::string_view arg2);

/**
 * @brief Formats ActionParameterNotSupported message into JSON
 * Message body: "The parameter <arg1> for the action <arg2> is not supported on
 * the target resource."
 *
 * @param[in] arg1 Parameter of message that will replace %1 in its body.
 * @param[in] arg2 Parameter of message that will replace %2 in its body.
 *
 * @returns Message ActionParameterNotSupported formatted to JSON */
/* @brief 将 ActionParameterNotSupported 消息格式化为 JSON
 * 消息正文："目标资源不支持操作 <arg2>
 * 的参数 <arg1>。"
 * @param[in] arg1 消息参数，将替换正文中的 %1。
 * @param[in] arg2 消息参数，将替换正文中的 %2。
 * @returns 格式化为 JSON 的 ActionParameterNotSupported 消息 */
nlohmann::json::object_t actionParameterNotSupported(std::string_view arg1,
                                                     std::string_view arg2);

void actionParameterNotSupported(crow::Response& res, std::string_view arg1,
                                 std::string_view arg2);

/**
 * @brief Formats ArraySizeTooLong message into JSON
 * Message body: "The array provided for property <arg1> exceeds the size limit
 * <arg2>."
 *
 * @param[in] arg1 Parameter of message that will replace %1 in its body.
 * @param[in] arg2 Parameter of message that will replace %2 in its body.
 *
 * @returns Message ArraySizeTooLong formatted to JSON */
/* @brief 将 ArraySizeTooLong 消息格式化为 JSON
 * 消息正文："为属性 <arg1> 提供的数组超出了大小限制
 *  <arg2>。"
 * @param[in] arg1 消息参数，将替换正文中的 %1。
 * @param[in] arg2 消息参数，将替换正文中的 %2。
 * @returns 格式化为 JSON 的 ArraySizeTooLong 消息 */
nlohmann::json::object_t arraySizeTooLong(std::string_view arg1, uint64_t arg2);

void arraySizeTooLong(crow::Response& res, std::string_view arg1,
                      uint64_t arg2);

/**
 * @brief Formats ArraySizeTooShort message into JSON
 * Message body: "The array provided for property <arg1> is under the minimum
 * size limit <arg2>."
 *
 * @param[in] arg1 Parameter of message that will replace %1 in its body.
 * @param[in] arg2 Parameter of message that will replace %2 in its body.
 *
 * @returns Message ArraySizeTooShort formatted to JSON */
/* @brief 将 ArraySizeTooShort 消息格式化为 JSON
 * 消息正文："为属性 <arg1> 提供的数组低于最小大小限制
 *  <arg2>。"
 * @param[in] arg1 消息参数，将替换正文中的 %1。
 * @param[in] arg2 消息参数，将替换正文中的 %2。
 * @returns 格式化为 JSON 的 ArraySizeTooShort 消息 */
nlohmann::json::object_t arraySizeTooShort(std::string_view arg1,
                                           uint64_t arg2);

void arraySizeTooShort(crow::Response& res, std::string_view arg1,
                       uint64_t arg2);

/**
 * @brief Formats QueryParameterValueTypeError message into JSON
 * Message body: "The value <arg1> for the query parameter <arg2> is not a type
 * that the parameter can accept."
 *
 * @param[in] arg1 Parameter of message that will replace %1 in its body.
 * @param[in] arg2 Parameter of message that will replace %2 in its body.
 *
 * @returns Message QueryParameterValueTypeError formatted to JSON */
/* @brief 将 QueryParameterValueTypeError 消息格式化为 JSON
 * 消息正文："查询参数 <arg2> 的值 <arg1>
 * 的类型不是该参数可接受的类型。"
 * @param[in] arg1 消息参数，将替换正文中的 %1。
 * @param[in] arg2 消息参数，将替换正文中的 %2。
 * @returns 格式化为 JSON 的 QueryParameterValueTypeError 消息 */
nlohmann::json::object_t queryParameterValueTypeError(
    const nlohmann::json& arg1, std::string_view arg2);

void queryParameterValueTypeError(
    crow::Response& res, const nlohmann::json& arg1, std::string_view arg2);

/**
 * @brief Formats QueryParameterValueFormatError message into JSON
 * Message body: "The value <arg1> for the parameter <arg2> is not a format that
 * the parameter can accept."
 *
 * @param[in] arg1 Parameter of message that will replace %1 in its body.
 * @param[in] arg2 Parameter of message that will replace %2 in its body.
 *
 * @returns Message QueryParameterValueFormatError formatted to JSON */
/* @brief 将 QueryParameterValueFormatError 消息格式化为 JSON
 * 消息正文："参数 <arg2> 的值 <arg1>
 * 的格式不是该参数可接受的格式。"
 * @param[in] arg1 消息参数，将替换正文中的 %1。
 * @param[in] arg2 消息参数，将替换正文中的 %2。
 * @returns 格式化为 JSON 的 QueryParameterValueFormatError 消息 */
nlohmann::json::object_t queryParameterValueFormatError(
    const nlohmann::json& arg1, std::string_view arg2);

void queryParameterValueFormatError(
    crow::Response& res, const nlohmann::json& arg1, std::string_view arg2);

/**
 * @brief Formats QueryParameterValueError message into JSON
 * Message body: "The value for the parameter <arg1> is invalid."
 *
 * @param[in] arg1 Parameter of message that will replace %1 in its body.
 *
 * @returns Message QueryParameterValueError formatted to JSON */
/* @brief 将 QueryParameterValueError 消息格式化为 JSON
 * 消息正文："参数 <arg1> 的值无效。"
 * @param[in] arg1 消息参数，将替换正文中的 %1。
 * @returns 格式化为 JSON 的 QueryParameterValueError 消息 */
nlohmann::json::object_t queryParameterValueError(std::string_view arg1);

void queryParameterValueError(crow::Response& res, std::string_view arg1);

/**
 * @brief Formats QueryParameterOutOfRange message into JSON
 * Message body: "The value <arg1> for the query parameter <arg2> is out of
 * range <arg3>."
 *
 * @param[in] arg1 Parameter of message that will replace %1 in its body.
 * @param[in] arg2 Parameter of message that will replace %2 in its body.
 * @param[in] arg3 Parameter of message that will replace %3 in its body.
 *
 * @returns Message QueryParameterOutOfRange formatted to JSON */
/* @brief 将 QueryParameterOutOfRange 消息格式化为 JSON
 * 消息正文："查询参数 <arg2> 的值
 *  <arg1> 超出范围 <arg3>。"
 * @param[in] arg1 消息参数，将替换正文中的 %1。
 * @param[in] arg2 消息参数，将替换正文中的 %2。
 * @param[in] arg3 消息参数，将替换正文中的 %3。
 * @returns 格式化为 JSON 的 QueryParameterOutOfRange 消息 */
nlohmann::json::object_t queryParameterOutOfRange(
    std::string_view arg1, std::string_view arg2, std::string_view arg3);

void queryParameterOutOfRange(crow::Response& res, std::string_view arg1,
                              std::string_view arg2, std::string_view arg3);

/**
 * @brief Formats QueryNotSupportedOnResource message into JSON
 * Message body: "Querying is not supported on the requested resource."
 *
 *
 * @returns Message QueryNotSupportedOnResource formatted to JSON */
/* @brief 将 QueryNotSupportedOnResource 消息格式化为 JSON
 * 消息正文："所请求的资源不支持查询。"
 * @returns 格式化为 JSON 的 QueryNotSupportedOnResource 消息 */
nlohmann::json::object_t queryNotSupportedOnResource();

void queryNotSupportedOnResource(crow::Response& res);

/**
 * @brief Formats QueryNotSupportedOnOperation message into JSON
 * Message body: "Querying is not supported with the requested operation."
 *
 *
 * @returns Message QueryNotSupportedOnOperation formatted to JSON */
/* @brief 将 QueryNotSupportedOnOperation 消息格式化为 JSON
 * 消息正文："所请求的操作不支持查询。"
 * @returns 格式化为 JSON 的 QueryNotSupportedOnOperation 消息 */
nlohmann::json::object_t queryNotSupportedOnOperation();

void queryNotSupportedOnOperation(crow::Response& res);

/**
 * @brief Formats QueryNotSupported message into JSON
 * Message body: "Querying is not supported by the implementation."
 *
 *
 * @returns Message QueryNotSupported formatted to JSON */
/* @brief 将 QueryNotSupported 消息格式化为 JSON
 * 消息正文："该实现不支持查询。"
 * @returns 格式化为 JSON 的 QueryNotSupported 消息 */
nlohmann::json::object_t queryNotSupported();

void queryNotSupported(crow::Response& res);

/**
 * @brief Formats QueryCombinationInvalid message into JSON
 * Message body: "Two or more query parameters in the request cannot be used
 * together."
 *
 *
 * @returns Message QueryCombinationInvalid formatted to JSON */
/* @brief 将 QueryCombinationInvalid 消息格式化为 JSON
 * 消息正文："请求中的两个或更多查询参数不能同时使用。
 * "
 * @returns 格式化为 JSON 的 QueryCombinationInvalid 消息 */
nlohmann::json::object_t queryCombinationInvalid();

void queryCombinationInvalid(crow::Response& res);

/**
 * @brief Formats QueryParameterUnsupported message into JSON
 * Message body: "Query parameter <arg1> is not supported."
 *
 * @param[in] arg1 Parameter of message that will replace %1 in its body.
 *
 * @returns Message QueryParameterUnsupported formatted to JSON */
/* @brief 将 QueryParameterUnsupported 消息格式化为 JSON
 * 消息正文："不支持查询参数 <arg1>。"
 * @param[in] arg1 消息参数，将替换正文中的 %1。
 * @returns 格式化为 JSON 的 QueryParameterUnsupported 消息 */
nlohmann::json::object_t queryParameterUnsupported(std::string_view arg1);

void queryParameterUnsupported(crow::Response& res, std::string_view arg1);

/**
 * @brief Formats SessionLimitExceeded message into JSON
 * Message body: "The session establishment failed due to the number of
 * simultaneous sessions exceeding the limit of the implementation."
 *
 *
 * @returns Message SessionLimitExceeded formatted to JSON */
/* @brief 将 SessionLimitExceeded 消息格式化为 JSON
 * 消息正文："由于并发会话数超过实现限制，会话建立失败。
 * "
 * @returns 格式化为 JSON 的 SessionLimitExceeded 消息 */
nlohmann::json::object_t sessionLimitExceeded();

void sessionLimitExceeded(crow::Response& res);

/**
 * @brief Formats EventSubscriptionLimitExceeded message into JSON
 * Message body: "The event subscription failed due to the number of
 * simultaneous subscriptions exceeding the limit of the implementation."
 *
 *
 * @returns Message EventSubscriptionLimitExceeded formatted to JSON */
/* @brief 将 EventSubscriptionLimitExceeded 消息格式化为 JSON
 * 消息正文："由于并发订阅数超过实现限制，事件订阅失败。
 * "
 * @returns 格式化为 JSON 的 EventSubscriptionLimitExceeded 消息 */
nlohmann::json::object_t eventSubscriptionLimitExceeded();

void eventSubscriptionLimitExceeded(crow::Response& res);

/**
 * @brief Formats ResourceCannotBeDeleted message into JSON
 * Message body: "The delete request failed because the resource requested
 * cannot be deleted."
 *
 *
 * @returns Message ResourceCannotBeDeleted formatted to JSON */
/* @brief 将 ResourceCannotBeDeleted 消息格式化为 JSON
 * 消息正文："删除请求失败，因为所请求的资源无法删除。
 * "
 * @returns 格式化为 JSON 的 ResourceCannotBeDeleted 消息 */
nlohmann::json::object_t resourceCannotBeDeleted();

void resourceCannotBeDeleted(crow::Response& res);

/**
 * @brief Formats ResourceInUse message into JSON
 * Message body: "The change to the requested resource failed because the
 * resource is in use or in transition."
 *
 *
 * @returns Message ResourceInUse formatted to JSON */
/* @brief 将 ResourceInUse 消息格式化为 JSON
 * 消息正文："对请求资源的更改失败，因为该资源正在使用中或处于转换状态。
 * "
 * @returns 格式化为 JSON 的 ResourceInUse 消息 */
nlohmann::json::object_t resourceInUse();

void resourceInUse(crow::Response& res);

/**
 * @brief Formats ResourceAlreadyExists message into JSON
 * Message body: "The requested resource of type <arg1> with the property <arg2>
 * with the value <arg3> already exists."
 *
 * @param[in] arg1 Parameter of message that will replace %1 in its body.
 * @param[in] arg2 Parameter of message that will replace %2 in its body.
 * @param[in] arg3 Parameter of message that will replace %3 in its body.
 *
 * @returns Message ResourceAlreadyExists formatted to JSON */
/* @brief 将 ResourceAlreadyExists 消息格式化为 JSON
 * 消息正文："属性 <arg2> 值为 <arg3>
 * 的 <arg1> 类型资源已存在。"
 * @param[in] arg1 消息参数，将替换正文中的 %1。
 * @param[in] arg2 消息参数，将替换正文中的 %2。
 * @param[in] arg3 消息参数，将替换正文中的 %3。
 * @returns 格式化为 JSON 的 ResourceAlreadyExists 消息 */
nlohmann::json::object_t resourceAlreadyExists(
    std::string_view arg1, std::string_view arg2, std::string_view arg3);

void resourceAlreadyExists(crow::Response& res, std::string_view arg1,
                           std::string_view arg2, std::string_view arg3);

/**
 * @brief Formats ResourceNotFound message into JSON
 * Message body: "The requested resource of type <arg1> named <arg2> was not
 * found."
 *
 * @param[in] arg1 Parameter of message that will replace %1 in its body.
 * @param[in] arg2 Parameter of message that will replace %2 in its body.
 *
 * @returns Message ResourceNotFound formatted to JSON */
/* @brief 将 ResourceNotFound 消息格式化为 JSON
 * 消息正文："未找到名为 <arg2>
 * 的 <arg1> 类型请求资源。"
 * @param[in] arg1 消息参数，将替换正文中的 %1。
 * @param[in] arg2 消息参数，将替换正文中的 %2。
 * @returns 格式化为 JSON 的 ResourceNotFound 消息 */
nlohmann::json::object_t resourceNotFound(std::string_view arg1,
                                          std::string_view arg2);

void resourceNotFound(crow::Response& res, std::string_view arg1,
                      std::string_view arg2);

/**
 * @brief Formats PayloadTooLarge message into JSON
 * Message body: "The supplied payload exceeds the maximum size supported by the
 * service."
 *
 *
 * @returns Message PayloadTooLarge formatted to JSON */
/* @brief 将 PayloadTooLarge 消息格式化为 JSON
 * 消息正文："提供的有效载荷超出了服务支持的最大大小。
 * "
 * @returns 格式化为 JSON 的 PayloadTooLarge 消息 */
nlohmann::json::object_t payloadTooLarge();

void payloadTooLarge(crow::Response& res);

/**
 * @brief Formats InsufficientStorage message into JSON
 * Message body: "Insufficient storage or memory available to complete the
 * request."
 *
 *
 * @returns Message InsufficientStorage formatted to JSON */
/* @brief 将 InsufficientStorage 消息格式化为 JSON
 * 消息正文："可用的存储空间或内存不足，无法完成请求。
 * "
 * @returns 格式化为 JSON 的 InsufficientStorage 消息 */
nlohmann::json::object_t insufficientStorage();

void insufficientStorage(crow::Response& res);

/**
 * @brief Formats MissingOrMalformedPart message into JSON
 * Message body: "The multipart request contains malformed parts or is missing
 * required parts."
 *
 *
 * @returns Message MissingOrMalformedPart formatted to JSON */
/* @brief 将 MissingOrMalformedPart 消息格式化为 JSON
 * 消息正文："multipart 请求包含格式错误的部件或缺少必需的部件。
 * "
 * @returns 格式化为 JSON 的 MissingOrMalformedPart 消息 */
nlohmann::json::object_t missingOrMalformedPart();

void missingOrMalformedPart(crow::Response& res);

/**
 * @brief Formats InvalidURI message into JSON
 * Message body: "The URI <arg1> was not found."
 *
 * @param[in] arg1 Parameter of message that will replace %1 in its body.
 *
 * @returns Message InvalidURI formatted to JSON */
/* @brief 将 InvalidURI 消息格式化为 JSON
 * 消息正文："未找到 URI <arg1>。"
 * @param[in] arg1 消息参数，将替换正文中的 %1。
 * @returns 格式化为 JSON 的 InvalidURI 消息 */
nlohmann::json::object_t invalidURI(std::string_view arg1);

void invalidURI(crow::Response& res, std::string_view arg1);

/**
 * @brief Formats CreateFailedMissingReqProperties message into JSON
 * Message body: "The create operation failed because the required property
 * <arg1> was missing from the request."
 *
 * @param[in] arg1 Parameter of message that will replace %1 in its body.
 *
 * @returns Message CreateFailedMissingReqProperties formatted to JSON */
/* @brief 将 CreateFailedMissingReqProperties 消息格式化为 JSON
 * 消息正文："创建操作失败，因为请求中缺少必需的属性
 *  <arg1>。"
 * @param[in] arg1 消息参数，将替换正文中的 %1。
 * @returns 格式化为 JSON 的 CreateFailedMissingReqProperties 消息 */
nlohmann::json::object_t createFailedMissingReqProperties(
    std::string_view arg1);

void createFailedMissingReqProperties(crow::Response& res,
                                      std::string_view arg1);

/**
 * @brief Formats CreateLimitReachedForResource message into JSON
 * Message body: "The create operation failed because the resource has reached
 * the limit of possible resources."
 *
 *
 * @returns Message CreateLimitReachedForResource formatted to JSON */
/* @brief 将 CreateLimitReachedForResource 消息格式化为 JSON
 * 消息正文："创建操作失败，因为该资源已达到可创建的资源数量上限。
 * "
 * @returns 格式化为 JSON 的 CreateLimitReachedForResource 消息 */
nlohmann::json::object_t createLimitReachedForResource();

void createLimitReachedForResource(crow::Response& res);

/**
 * @brief Formats ServiceShuttingDown message into JSON
 * Message body: "The operation failed because the service is shutting down and
 * can no longer take incoming requests."
 *
 *
 * @returns Message ServiceShuttingDown formatted to JSON */
/* @brief 将 ServiceShuttingDown 消息格式化为 JSON
 * 消息正文："操作失败，因为服务正在关闭，无法再接收传入请求。
 * "
 * @returns 格式化为 JSON 的 ServiceShuttingDown 消息 */
nlohmann::json::object_t serviceShuttingDown();

void serviceShuttingDown(crow::Response& res);

/**
 * @brief Formats ServiceInUnknownState message into JSON
 * Message body: "The operation failed because the service is in an unknown
 * state and can no longer take incoming requests."
 *
 *
 * @returns Message ServiceInUnknownState formatted to JSON */
/* @brief 将 ServiceInUnknownState 消息格式化为 JSON
 * 消息正文："操作失败，因为服务处于未知状态，无法再接收传入请求。
 * "
 * @returns 格式化为 JSON 的 ServiceInUnknownState 消息 */
nlohmann::json::object_t serviceInUnknownState();

void serviceInUnknownState(crow::Response& res);

/**
 * @brief Formats NoValidSession message into JSON
 * Message body: "There is no valid session established with the
 * implementation."
 *
 *
 * @returns Message NoValidSession formatted to JSON */
/* @brief 将 NoValidSession 消息格式化为 JSON
 * 消息正文："与该实现之间未建立有效的会话。
 * "
 * @returns 格式化为 JSON 的 NoValidSession 消息 */
nlohmann::json::object_t noValidSession();

void noValidSession(crow::Response& res);

/**
 * @brief Formats InsufficientPrivilege message into JSON
 * Message body: "There are insufficient privileges for the account or
 * credentials associated with the current session to perform the requested
 * operation."
 *
 *
 * @returns Message InsufficientPrivilege formatted to JSON */
/* @brief 将 InsufficientPrivilege 消息格式化为 JSON
 * 消息正文："与当前会话关联的账户或凭据权限不足，无法执行所请求的操作。
 * "
 * @returns 格式化为 JSON 的 InsufficientPrivilege 消息 */
nlohmann::json::object_t insufficientPrivilege();

void insufficientPrivilege(crow::Response& res);

/**
 * @brief Formats AccountModified message into JSON
 * Message body: "The account was successfully modified."
 *
 *
 * @returns Message AccountModified formatted to JSON */
/* @brief 将 AccountModified 消息格式化为 JSON
 * 消息正文："账户已成功修改。"
 * @returns 格式化为 JSON 的 AccountModified 消息 */
nlohmann::json::object_t accountModified();

void accountModified(crow::Response& res);

/**
 * @brief Formats AccountNotModified message into JSON
 * Message body: "The account modification request failed."
 *
 *
 * @returns Message AccountNotModified formatted to JSON */
/* @brief 将 AccountNotModified 消息格式化为 JSON
 * 消息正文："账户修改请求失败。"
 * @returns 格式化为 JSON 的 AccountNotModified 消息 */
nlohmann::json::object_t accountNotModified();

void accountNotModified(crow::Response& res);

/**
 * @brief Formats AccountRemoved message into JSON
 * Message body: "The account was successfully removed."
 *
 *
 * @returns Message AccountRemoved formatted to JSON */
/* @brief 将 AccountRemoved 消息格式化为 JSON
 * 消息正文："账户已成功删除。"
 * @returns 格式化为 JSON 的 AccountRemoved 消息 */
nlohmann::json::object_t accountRemoved();

void accountRemoved(crow::Response& res);

/**
 * @brief Formats AccountForSessionNoLongerExists message into JSON
 * Message body: "The account for the current session was removed, and so the
 * current session was removed as well."
 *
 *
 * @returns Message AccountForSessionNoLongerExists formatted to JSON */
/* @brief 将 AccountForSessionNoLongerExists 消息格式化为 JSON
 * 消息正文："当前会话的账户已被删除，因此当前会话也已删除。
 * "
 * @returns 格式化为 JSON 的 AccountForSessionNoLongerExists 消息 */
nlohmann::json::object_t accountForSessionNoLongerExists();

void accountForSessionNoLongerExists(crow::Response& res);

/**
 * @brief Formats InvalidObject message into JSON
 * Message body: "The object at <arg1> is invalid."
 *
 * @param[in] arg1 Parameter of message that will replace %1 in its body.
 *
 * @returns Message InvalidObject formatted to JSON */
/* @brief 将 InvalidObject 消息格式化为 JSON
 * 消息正文："<arg1> 处的对象无效。"
 * @param[in] arg1 消息参数，将替换正文中的 %1。
 * @returns 格式化为 JSON 的 InvalidObject 消息 */
nlohmann::json::object_t invalidObject(const boost::urls::url_view_base& arg1);

void invalidObject(crow::Response& res, const boost::urls::url_view_base& arg1);

/**
 * @brief Formats InternalError message into JSON
 * Message body: "The request failed due to an internal service error.  The
 * service is still operational."
 *
 *
 * @returns Message InternalError formatted to JSON */
/* @brief 将 InternalError 消息格式化为 JSON
 * 消息正文："请求因内部服务错误而失败。服务仍在正常运行。
 * "
 * @returns 格式化为 JSON 的 InternalError 消息 */
nlohmann::json::object_t internalError();

void internalError(crow::Response& res, std::source_location location =
                                            std::source_location::current());

/**
 * @brief Formats UnrecognizedRequestBody message into JSON
 * Message body: "The service detected a malformed request body that it was
 * unable to interpret."
 *
 *
 * @returns Message UnrecognizedRequestBody formatted to JSON */
/* @brief 将 UnrecognizedRequestBody 消息格式化为 JSON
 * 消息正文："服务检测到格式错误、无法解析的请求正文。
 * "
 * @returns 格式化为 JSON 的 UnrecognizedRequestBody 消息 */
nlohmann::json::object_t unrecognizedRequestBody();

void unrecognizedRequestBody(crow::Response& res);

/**
 * @brief Formats ResourceMissingAtURI message into JSON
 * Message body: "The resource at the URI <arg1> was not found."
 *
 * @param[in] arg1 Parameter of message that will replace %1 in its body.
 *
 * @returns Message ResourceMissingAtURI formatted to JSON */
/* @brief 将 ResourceMissingAtURI 消息格式化为 JSON
 * 消息正文："未找到 URI <arg1> 处的资源。"
 * @param[in] arg1 消息参数，将替换正文中的 %1。
 * @returns 格式化为 JSON 的 ResourceMissingAtURI 消息 */
nlohmann::json::object_t resourceMissingAtURI(
    const boost::urls::url_view_base& arg1);

void resourceMissingAtURI(crow::Response& res,
                          const boost::urls::url_view_base& arg1);

/**
 * @brief Formats ResourceAtUriInUnknownFormat message into JSON
 * Message body: "The resource at <arg1> is in a format not recognized by the
 * service."
 *
 * @param[in] arg1 Parameter of message that will replace %1 in its body.
 *
 * @returns Message ResourceAtUriInUnknownFormat formatted to JSON */
/* @brief 将 ResourceAtUriInUnknownFormat 消息格式化为 JSON
 * 消息正文："<arg1> 处的资源采用了服务无法识别的格式。
 * "
 * @param[in] arg1 消息参数，将替换正文中的 %1。
 * @returns 格式化为 JSON 的 ResourceAtUriInUnknownFormat 消息 */
nlohmann::json::object_t resourceAtUriInUnknownFormat(
    const boost::urls::url_view_base& arg1);

void resourceAtUriInUnknownFormat(crow::Response& res,
                                  const boost::urls::url_view_base& arg1);

/**
 * @brief Formats ResourceAtUriUnauthorized message into JSON
 * Message body: "While accessing the resource at <arg1>, the service received
 * an authorization error <arg2>."
 *
 * @param[in] arg1 Parameter of message that will replace %1 in its body.
 * @param[in] arg2 Parameter of message that will replace %2 in its body.
 *
 * @returns Message ResourceAtUriUnauthorized formatted to JSON */
/* @brief 将 ResourceAtUriUnauthorized 消息格式化为 JSON
 * 消息正文："访问 <arg1> 处的资源时，服务收到授权错误
 *  <arg2>。"
 * @param[in] arg1 消息参数，将替换正文中的 %1。
 * @param[in] arg2 消息参数，将替换正文中的 %2。
 * @returns 格式化为 JSON 的 ResourceAtUriUnauthorized 消息 */
nlohmann::json::object_t resourceAtUriUnauthorized(
    const boost::urls::url_view_base& arg1, std::string_view arg2);

void resourceAtUriUnauthorized(crow::Response& res,
                               const boost::urls::url_view_base& arg1,
                               std::string_view arg2);

/**
 * @brief Formats CouldNotEstablishConnection message into JSON
 * Message body: "The service failed to establish a connection with the URI
 * <arg1>."
 *
 * @param[in] arg1 Parameter of message that will replace %1 in its body.
 *
 * @returns Message CouldNotEstablishConnection formatted to JSON */
/* @brief 将 CouldNotEstablishConnection 消息格式化为 JSON
 * 消息正文："服务无法与 URI <arg1>
 * 建立连接。"
 * @param[in] arg1 消息参数，将替换正文中的 %1。
 * @returns 格式化为 JSON 的 CouldNotEstablishConnection 消息 */
nlohmann::json::object_t couldNotEstablishConnection(
    const boost::urls::url_view_base& arg1);

void couldNotEstablishConnection(crow::Response& res,
                                 const boost::urls::url_view_base& arg1);

/**
 * @brief Formats SourceDoesNotSupportProtocol message into JSON
 * Message body: "The other end of the connection at <arg1> does not support the
 * specified protocol <arg2>."
 *
 * @param[in] arg1 Parameter of message that will replace %1 in its body.
 * @param[in] arg2 Parameter of message that will replace %2 in its body.
 *
 * @returns Message SourceDoesNotSupportProtocol formatted to JSON */
/* @brief 将 SourceDoesNotSupportProtocol 消息格式化为 JSON
 * 消息正文："<arg1> 处的连接另一端不支持指定的协议
 *  <arg2>。"
 * @param[in] arg1 消息参数，将替换正文中的 %1。
 * @param[in] arg2 消息参数，将替换正文中的 %2。
 * @returns 格式化为 JSON 的 SourceDoesNotSupportProtocol 消息 */
nlohmann::json::object_t sourceDoesNotSupportProtocol(
    const boost::urls::url_view_base& arg1, std::string_view arg2);

void sourceDoesNotSupportProtocol(crow::Response& res,
                                  const boost::urls::url_view_base& arg1,
                                  std::string_view arg2);

/**
 * @brief Formats AccessDenied message into JSON
 * Message body: "While attempting to establish a connection to <arg1>, the
 * service denied access."
 *
 * @param[in] arg1 Parameter of message that will replace %1 in its body.
 *
 * @returns Message AccessDenied formatted to JSON */
/* @brief 将 AccessDenied 消息格式化为 JSON
 * 消息正文："尝试建立到 <arg1>
 * 的连接时，服务拒绝了访问。"
 * @param[in] arg1 消息参数，将替换正文中的 %1。
 * @returns 格式化为 JSON 的 AccessDenied 消息 */
nlohmann::json::object_t accessDenied(const boost::urls::url_view_base& arg1);

void accessDenied(crow::Response& res, const boost::urls::url_view_base& arg1);

/**
 * @brief Formats ServiceTemporarilyUnavailable message into JSON
 * Message body: "The service is temporarily unavailable.  Retry in <arg1>
 * seconds."
 *
 * @param[in] arg1 Parameter of message that will replace %1 in its body.
 *
 * @returns Message ServiceTemporarilyUnavailable formatted to JSON */
/* @brief 将 ServiceTemporarilyUnavailable 消息格式化为 JSON
 * 消息正文："服务暂时不可用。请在
 *  <arg1> 秒后重试。"
 * @param[in] arg1 消息参数，将替换正文中的 %1。
 * @returns 格式化为 JSON 的 ServiceTemporarilyUnavailable 消息 */
nlohmann::json::object_t serviceTemporarilyUnavailable(std::string_view arg1);

void serviceTemporarilyUnavailable(crow::Response& res, std::string_view arg1);

/**
 * @brief Formats InvalidIndex message into JSON
 * Message body: "The index <arg1> is not a valid offset into the array."
 *
 * @param[in] arg1 Parameter of message that will replace %1 in its body.
 *
 * @returns Message InvalidIndex formatted to JSON */
/* @brief 将 InvalidIndex 消息格式化为 JSON
 * 消息正文："索引 <arg1> 不是该数组的有效偏移量。"
 * @param[in] arg1 消息参数，将替换正文中的 %1。
 * @returns 格式化为 JSON 的 InvalidIndex 消息 */
nlohmann::json::object_t invalidIndex(uint64_t arg1);

void invalidIndex(crow::Response& res, uint64_t arg1);

/**
 * @brief Formats PropertyValueModified message into JSON
 * Message body: "The property <arg1> was assigned the value <arg2> due to
 * modification by the service."
 *
 * @param[in] arg1 Parameter of message that will replace %1 in its body.
 * @param[in] arg2 Parameter of message that will replace %2 in its body.
 *
 * @returns Message PropertyValueModified formatted to JSON */
/* @brief 将 PropertyValueModified 消息格式化为 JSON
 * 消息正文："由于服务的修改，属性 <arg1>
 * 被赋值为 <arg2>。"
 * @param[in] arg1 消息参数，将替换正文中的 %1。
 * @param[in] arg2 消息参数，将替换正文中的 %2。
 * @returns 格式化为 JSON 的 PropertyValueModified 消息 */
nlohmann::json::object_t propertyValueModified(std::string_view arg1,
                                               const nlohmann::json& arg2);

void propertyValueModified(crow::Response& res, std::string_view arg1,
                           const nlohmann::json& arg2);

/**
 * @brief Formats ResourceInStandby message into JSON
 * Message body: "The request could not be performed because the resource is in
 * standby."
 *
 *
 * @returns Message ResourceInStandby formatted to JSON */
/* @brief 将 ResourceInStandby 消息格式化为 JSON
 * 消息正文："由于资源处于待机状态，无法执行该请求。
 * "
 * @returns 格式化为 JSON 的 ResourceInStandby 消息 */
nlohmann::json::object_t resourceInStandby();

void resourceInStandby(crow::Response& res);

/**
 * @brief Formats ResourceExhaustion message into JSON
 * Message body: "The resource <arg1> was unable to satisfy the request due to
 * unavailability of resources."
 *
 * @param[in] arg1 Parameter of message that will replace %1 in its body.
 *
 * @returns Message ResourceExhaustion formatted to JSON */
/* @brief 将 ResourceExhaustion 消息格式化为 JSON
 * 消息正文："由于资源不可用，资源
 *  <arg1> 无法满足请求。"
 * @param[in] arg1 消息参数，将替换正文中的 %1。
 * @returns 格式化为 JSON 的 ResourceExhaustion 消息 */
nlohmann::json::object_t resourceExhaustion(std::string_view arg1);

void resourceExhaustion(crow::Response& res, std::string_view arg1);

/**
 * @brief Formats StringValueTooLong message into JSON
 * Message body: "The string <arg1> exceeds the length limit <arg2>."
 *
 * @param[in] arg1 Parameter of message that will replace %1 in its body.
 * @param[in] arg2 Parameter of message that will replace %2 in its body.
 *
 * @returns Message StringValueTooLong formatted to JSON */
/* @brief 将 StringValueTooLong 消息格式化为 JSON
 * 消息正文："字符串 <arg1> 超出了长度限制 <arg2>。"
 * @param[in] arg1 消息参数，将替换正文中的 %1。
 * @param[in] arg2 消息参数，将替换正文中的 %2。
 * @returns 格式化为 JSON 的 StringValueTooLong 消息 */
nlohmann::json::object_t stringValueTooLong(std::string_view arg1,
                                            uint64_t arg2);

void stringValueTooLong(crow::Response& res, std::string_view arg1,
                        uint64_t arg2);

/**
 * @brief Formats StringValueTooShort message into JSON
 * Message body: "The string <arg1> was under the minimum required length
 * <arg2>."
 *
 * @param[in] arg1 Parameter of message that will replace %1 in its body.
 * @param[in] arg2 Parameter of message that will replace %2 in its body.
 *
 * @returns Message StringValueTooShort formatted to JSON */
/* @brief 将 StringValueTooShort 消息格式化为 JSON
 * 消息正文："字符串 <arg1> 低于要求的最小长度
 *  <arg2>。"
 * @param[in] arg1 消息参数，将替换正文中的 %1。
 * @param[in] arg2 消息参数，将替换正文中的 %2。
 * @returns 格式化为 JSON 的 StringValueTooShort 消息 */
nlohmann::json::object_t stringValueTooShort(std::string_view arg1,
                                             uint64_t arg2);

void stringValueTooShort(crow::Response& res, std::string_view arg1,
                         uint64_t arg2);

/**
 * @brief Formats SessionTerminated message into JSON
 * Message body: "The session was successfully terminated."
 *
 *
 * @returns Message SessionTerminated formatted to JSON */
/* @brief 将 SessionTerminated 消息格式化为 JSON
 * 消息正文："会话已成功终止。"
 * @returns 格式化为 JSON 的 SessionTerminated 消息 */
nlohmann::json::object_t sessionTerminated();

void sessionTerminated(crow::Response& res);

/**
 * @brief Formats SubscriptionTerminated message into JSON
 * Message body: "The event subscription was terminated."
 *
 *
 * @returns Message SubscriptionTerminated formatted to JSON */
/* @brief 将 SubscriptionTerminated 消息格式化为 JSON
 * 消息正文："事件订阅已终止。"
 * @returns 格式化为 JSON 的 SubscriptionTerminated 消息 */
nlohmann::json::object_t subscriptionTerminated();

void subscriptionTerminated(crow::Response& res);

/**
 * @brief Formats ResourceTypeIncompatible message into JSON
 * Message body: "The @odata.type of the request body <arg1> is incompatible
 * with the @odata.type of the resource, which is <arg2>."
 *
 * @param[in] arg1 Parameter of message that will replace %1 in its body.
 * @param[in] arg2 Parameter of message that will replace %2 in its body.
 *
 * @returns Message ResourceTypeIncompatible formatted to JSON */
/* @brief 将 ResourceTypeIncompatible 消息格式化为 JSON
 * 消息正文："请求正文的 @odata.type <arg1>
 * 与该资源的 @odata.type <arg2> 不兼容。"
 * @param[in] arg1 消息参数，将替换正文中的 %1。
 * @param[in] arg2 消息参数，将替换正文中的 %2。
 * @returns 格式化为 JSON 的 ResourceTypeIncompatible 消息 */
nlohmann::json::object_t resourceTypeIncompatible(std::string_view arg1,
                                                  std::string_view arg2);

void resourceTypeIncompatible(crow::Response& res, std::string_view arg1,
                              std::string_view arg2);

/**
 * @brief Formats PasswordChangeRequired message into JSON
 * Message body: "The password provided for this account must be changed before
 * access is granted.  PATCH the Password property for this account located at
 * the target URI <arg1> to complete this process."
 *
 * @param[in] arg1 Parameter of message that will replace %1 in its body.
 *
 * @returns Message PasswordChangeRequired formatted to JSON */
/* @brief 将 PasswordChangeRequired 消息格式化为 JSON
 * 消息正文："在获得访问权限之前，必须更改为此账户提供的密码。请对目标
 *  URI <arg1> 处该账户的 Password
 * 属性执行 PATCH 以完成此过程。"
 * @param[in] arg1 消息参数，将替换正文中的 %1。
 * @returns 格式化为 JSON 的 PasswordChangeRequired 消息 */
nlohmann::json::object_t passwordChangeRequired(
    const boost::urls::url_view_base& arg1);

void passwordChangeRequired(crow::Response& res,
                            const boost::urls::url_view_base& arg1);

/**
 * @brief Formats ResetRequired message into JSON
 * Message body: "In order to apply changes, recover from errors, or complete
 * the operation, a component reset is required with the Reset action URI <arg1>
 * and ResetType <arg2>."
 *
 * @param[in] arg1 Parameter of message that will replace %1 in its body.
 * @param[in] arg2 Parameter of message that will replace %2 in its body.
 *
 * @returns Message ResetRequired formatted to JSON */
/* @brief 将 ResetRequired 消息格式化为 JSON
 * 消息正文："为应用更改、从错误中恢复或完成操作，需要以
 *  Reset 操作 URI <arg1> 和
 *  ResetType <arg2> 重置组件。"
 * @param[in] arg1 消息参数，将替换正文中的 %1。
 * @param[in] arg2 消息参数，将替换正文中的 %2。
 * @returns 格式化为 JSON 的 ResetRequired 消息 */
nlohmann::json::object_t resetRequired(const boost::urls::url_view_base& arg1,
                                       std::string_view arg2);

void resetRequired(crow::Response& res, const boost::urls::url_view_base& arg1,
                   std::string_view arg2);

/**
 * @brief Formats ResetRecommended message into JSON
 * Message body: "In order to recover from errors, a component reset is
 * recommended with the Reset action URI <arg1> and ResetType <arg2>."
 *
 * @param[in] arg1 Parameter of message that will replace %1 in its body.
 * @param[in] arg2 Parameter of message that will replace %2 in its body.
 *
 * @returns Message ResetRecommended formatted to JSON */
/* @brief 将 ResetRecommended 消息格式化为 JSON
 * 消息正文："为从错误中恢复，建议以 Reset 操作 URI <arg1>
 * 和 ResetType <arg2> 重置组件。"
 * @param[in] arg1 消息参数，将替换正文中的 %1。
 * @param[in] arg2 消息参数，将替换正文中的 %2。
 * @returns 格式化为 JSON 的 ResetRecommended 消息 */
nlohmann::json::object_t resetRecommended(std::string_view arg1,
                                          std::string_view arg2);

void resetRecommended(crow::Response& res, std::string_view arg1,
                      std::string_view arg2);

/**
 * @brief Formats ChassisPowerStateOnRequired message into JSON
 * Message body: "The chassis with Id <arg1> is required to be powered on to
 * perform this request."
 *
 * @param[in] arg1 Parameter of message that will replace %1 in its body.
 *
 * @returns Message ChassisPowerStateOnRequired formatted to JSON */
/* @brief 将 ChassisPowerStateOnRequired 消息格式化为 JSON
 * 消息正文："需要打开 Id 为 <arg1>
 * 的机箱电源才能执行此请求。"
 * @param[in] arg1 消息参数，将替换正文中的 %1。
 * @returns 格式化为 JSON 的 ChassisPowerStateOnRequired 消息 */
nlohmann::json::object_t chassisPowerStateOnRequired(std::string_view arg1);

void chassisPowerStateOnRequired(crow::Response& res, std::string_view arg1);

/**
 * @brief Formats ChassisPowerStateOffRequired message into JSON
 * Message body: "The chassis with Id <arg1> is required to be powered off to
 * perform this request."
 *
 * @param[in] arg1 Parameter of message that will replace %1 in its body.
 *
 * @returns Message ChassisPowerStateOffRequired formatted to JSON */
/* @brief 将 ChassisPowerStateOffRequired 消息格式化为 JSON
 * 消息正文："需要关闭 Id 为 <arg1>
 * 的机箱电源才能执行此请求。"
 * @param[in] arg1 消息参数，将替换正文中的 %1。
 * @returns 格式化为 JSON 的 ChassisPowerStateOffRequired 消息 */
nlohmann::json::object_t chassisPowerStateOffRequired(std::string_view arg1);

void chassisPowerStateOffRequired(crow::Response& res, std::string_view arg1);

/**
 * @brief Formats PropertyValueConflict message into JSON
 * Message body: "The property <arg1> could not be written because its value
 * would conflict with the value of the <arg2> property."
 *
 * @param[in] arg1 Parameter of message that will replace %1 in its body.
 * @param[in] arg2 Parameter of message that will replace %2 in its body.
 *
 * @returns Message PropertyValueConflict formatted to JSON */
/* @brief 将 PropertyValueConflict 消息格式化为 JSON
 * 消息正文："无法写入属性 <arg1>，因为其值会与属性
 *  <arg2> 的值冲突。"
 * @param[in] arg1 消息参数，将替换正文中的 %1。
 * @param[in] arg2 消息参数，将替换正文中的 %2。
 * @returns 格式化为 JSON 的 PropertyValueConflict 消息 */
nlohmann::json::object_t propertyValueConflict(std::string_view arg1,
                                               std::string_view arg2);

void propertyValueConflict(crow::Response& res, std::string_view arg1,
                           std::string_view arg2);

/**
 * @brief Formats PropertyValueResourceConflict message into JSON
 * Message body: "The property <arg1> with the requested value of <arg2> could
 * not be written because the value conflicts with the state or configuration of
 * the resource at <arg3>."
 *
 * @param[in] arg1 Parameter of message that will replace %1 in its body.
 * @param[in] arg2 Parameter of message that will replace %2 in its body.
 * @param[in] arg3 Parameter of message that will replace %3 in its body.
 *
 * @returns Message PropertyValueResourceConflict formatted to JSON */
/* @brief 将 PropertyValueResourceConflict 消息格式化为 JSON
 * 消息正文："无法写入属性 <arg1>
 * 的请求值 <arg2>，因为该值与 <arg3>
 * 处资源的状态或配置冲突。"
 * @param[in] arg1 消息参数，将替换正文中的 %1。
 * @param[in] arg2 消息参数，将替换正文中的 %2。
 * @param[in] arg3 消息参数，将替换正文中的 %3。
 * @returns 格式化为 JSON 的 PropertyValueResourceConflict 消息 */
nlohmann::json::object_t propertyValueResourceConflict(
    std::string_view arg1, const nlohmann::json& arg2,
    const boost::urls::url_view_base& arg3);

void propertyValueResourceConflict(crow::Response& res, std::string_view arg1,
                                   const nlohmann::json& arg2,
                                   const boost::urls::url_view_base& arg3);

/**
 * @brief Formats PropertyValueExternalConflict message into JSON
 * Message body: "The property <arg1> with the requested value of <arg2> could
 * not be written because the value is not available due to a configuration
 * conflict."
 *
 * @param[in] arg1 Parameter of message that will replace %1 in its body.
 * @param[in] arg2 Parameter of message that will replace %2 in its body.
 *
 * @returns Message PropertyValueExternalConflict formatted to JSON */
/* @brief 将 PropertyValueExternalConflict 消息格式化为 JSON
 * 消息正文："无法写入属性 <arg1>
 * 的请求值 <arg2>
 * ，因为由于配置冲突该值不可用。"
 * @param[in] arg1 消息参数，将替换正文中的 %1。
 * @param[in] arg2 消息参数，将替换正文中的 %2。
 * @returns 格式化为 JSON 的 PropertyValueExternalConflict 消息 */
nlohmann::json::object_t propertyValueExternalConflict(
    std::string_view arg1, const nlohmann::json& arg2);

void propertyValueExternalConflict(crow::Response& res, std::string_view arg1,
                                   const nlohmann::json& arg2);

/**
 * @brief Formats PropertyValueIncorrect message into JSON
 * Message body: "The property <arg1> with the requested value of <arg2> could
 * not be written because the value does not meet the constraints of the
 * implementation."
 *
 * @param[in] arg1 Parameter of message that will replace %1 in its body.
 * @param[in] arg2 Parameter of message that will replace %2 in its body.
 *
 * @returns Message PropertyValueIncorrect formatted to JSON */
/* @brief 将 PropertyValueIncorrect 消息格式化为 JSON
 * 消息正文："无法写入属性 <arg1>
 * 的请求值 <arg2>
 * ，因为该值不满足实现的约束。"
 * @param[in] arg1 消息参数，将替换正文中的 %1。
 * @param[in] arg2 消息参数，将替换正文中的 %2。
 * @returns 格式化为 JSON 的 PropertyValueIncorrect 消息 */
nlohmann::json::object_t propertyValueIncorrect(std::string_view arg1,
                                                const nlohmann::json& arg2);

void propertyValueIncorrect(crow::Response& res, std::string_view arg1,
                            const nlohmann::json& arg2);

/**
 * @brief Formats ResourceCreationConflict message into JSON
 * Message body: "The resource could not be created.  The service has a resource
 * at URI <arg1> that conflicts with the creation request."
 *
 * @param[in] arg1 Parameter of message that will replace %1 in its body.
 *
 * @returns Message ResourceCreationConflict formatted to JSON */
/* @brief 将 ResourceCreationConflict 消息格式化为 JSON
 * 消息正文："无法创建资源。服务在 URI <arg1>
 * 处有一个与创建请求冲突的资源。"
 * @param[in] arg1 消息参数，将替换正文中的 %1。
 * @returns 格式化为 JSON 的 ResourceCreationConflict 消息 */
nlohmann::json::object_t resourceCreationConflict(
    const boost::urls::url_view_base& arg1);

void resourceCreationConflict(crow::Response& res,
                              const boost::urls::url_view_base& arg1);

/**
 * @brief Formats ActionParameterValueConflict message into JSON
 * Message body: "The parameter <arg1> with the requested value of <arg2> does
 * not meet the constraints of the implementation."
 *
 * @param[in] arg1 Parameter of message that will replace %1 in its body.
 * @param[in] arg2 Parameter of message that will replace %2 in its body.
 *
 * @returns Message ActionParameterValueConflict formatted to JSON */
/* @brief 将 ActionParameterValueConflict 消息格式化为 JSON
 * 消息正文："参数 <arg1> 的请求值
 *  <arg2> 不满足实现的约束。"
 * @param[in] arg1 消息参数，将替换正文中的 %1。
 * @param[in] arg2 消息参数，将替换正文中的 %2。
 * @returns 格式化为 JSON 的 ActionParameterValueConflict 消息 */
nlohmann::json::object_t actionParameterValueConflict(std::string_view arg1,
                                                      std::string_view arg2);

void actionParameterValueConflict(crow::Response& res, std::string_view arg1,
                                  std::string_view arg2);

/**
 * @brief Formats MaximumErrorsExceeded message into JSON
 * Message body: "Too many errors have occurred to report them all."
 *
 *
 * @returns Message MaximumErrorsExceeded formatted to JSON */
/* @brief 将 MaximumErrorsExceeded 消息格式化为 JSON
 * 消息正文："发生的错误太多，无法全部报告。"
 * @returns 格式化为 JSON 的 MaximumErrorsExceeded 消息 */
nlohmann::json::object_t maximumErrorsExceeded();

void maximumErrorsExceeded(crow::Response& res);

/**
 * @brief Formats PreconditionFailed message into JSON
 * Message body: "The ETag supplied did not match the ETag required to change
 * this resource."
 *
 *
 * @returns Message PreconditionFailed formatted to JSON */
/* @brief 将 PreconditionFailed 消息格式化为 JSON
 * 消息正文："提供的 ETag 与更改该资源所需的
 *  ETag 不匹配。"
 * @returns 格式化为 JSON 的 PreconditionFailed 消息 */
nlohmann::json::object_t preconditionFailed();

void preconditionFailed(crow::Response& res);

/**
 * @brief Formats PreconditionRequired message into JSON
 * Message body: "A precondition header or annotation is required to change this
 * resource."
 *
 *
 * @returns Message PreconditionRequired formatted to JSON */
/* @brief 将 PreconditionRequired 消息格式化为 JSON
 * 消息正文："更改该资源需要先决条件请求头或注解。
 * "
 * @returns 格式化为 JSON 的 PreconditionRequired 消息 */
nlohmann::json::object_t preconditionRequired();

void preconditionRequired(crow::Response& res);

/**
 * @brief Formats HeaderMissing message into JSON
 * Message body: "Required header <arg1> is missing in the request."
 *
 * @param[in] arg1 Parameter of message that will replace %1 in its body.
 *
 * @returns Message HeaderMissing formatted to JSON */
/* @brief 将 HeaderMissing 消息格式化为 JSON
 * 消息正文："请求中缺少必需的请求头 <arg1>。"
 * @param[in] arg1 消息参数，将替换正文中的 %1。
 * @returns 格式化为 JSON 的 HeaderMissing 消息 */
nlohmann::json::object_t headerMissing(std::string_view arg1);

void headerMissing(crow::Response& res, std::string_view arg1);

/**
 * @brief Formats HeaderInvalid message into JSON
 * Message body: "Header <arg1> is invalid."
 *
 * @param[in] arg1 Parameter of message that will replace %1 in its body.
 *
 * @returns Message HeaderInvalid formatted to JSON */
/* @brief 将 HeaderInvalid 消息格式化为 JSON
 * 消息正文："请求头 <arg1> 无效。"
 * @param[in] arg1 消息参数，将替换正文中的 %1。
 * @returns 格式化为 JSON 的 HeaderInvalid 消息 */
nlohmann::json::object_t headerInvalid(std::string_view arg1);

void headerInvalid(crow::Response& res, std::string_view arg1);

/**
 * @brief Formats OperationFailed message into JSON
 * Message body: "An error occurred internal to the service as part of the
 * overall request.  Partial results may have been returned."
 *
 *
 * @returns Message OperationFailed formatted to JSON */
/* @brief 将 OperationFailed 消息格式化为 JSON
 * 消息正文："作为整体请求的一部分，服务内部发生错误。可能已返回部分结果。
 * "
 * @returns 格式化为 JSON 的 OperationFailed 消息 */
nlohmann::json::object_t operationFailed();

void operationFailed(crow::Response& res);

/**
 * @brief Formats OperationTimeout message into JSON
 * Message body: "A timeout internal to the service occurred as part of the
 * request.  Partial results may have been returned."
 *
 *
 * @returns Message OperationTimeout formatted to JSON */
/* @brief 将 OperationTimeout 消息格式化为 JSON
 * 消息正文："作为请求的一部分，服务内部发生超时。可能已返回部分结果。
 * "
 * @returns 格式化为 JSON 的 OperationTimeout 消息 */
nlohmann::json::object_t operationTimeout();

void operationTimeout(crow::Response& res);

/**
 * @brief Formats OperationNotAllowed message into JSON
 * Message body: "The HTTP method is not allowed on this resource."
 *
 *
 * @returns Message OperationNotAllowed formatted to JSON */
/* @brief 将 OperationNotAllowed 消息格式化为 JSON
 * 消息正文："该资源不允许使用此 HTTP 方法。"
 * @returns 格式化为 JSON 的 OperationNotAllowed 消息 */
nlohmann::json::object_t operationNotAllowed();

void operationNotAllowed(crow::Response& res);

/**
 * @brief Formats UndeterminedFault message into JSON
 * Message body: "An undetermined fault condition was reported by <arg1>."
 *
 * @param[in] arg1 Parameter of message that will replace %1 in its body.
 *
 * @returns Message UndeterminedFault formatted to JSON */
/* @brief 将 UndeterminedFault 消息格式化为 JSON
 * 消息正文："<arg1> 报告了无法确定原因的故障状况。"
 * @param[in] arg1 消息参数，将替换正文中的 %1。
 * @returns 格式化为 JSON 的 UndeterminedFault 消息 */
nlohmann::json::object_t undeterminedFault(std::string_view arg1);

void undeterminedFault(crow::Response& res, std::string_view arg1);

/**
 * @brief Formats ConditionInRelatedResource message into JSON
 * Message body: "One or more conditions exist in a related resource.  See the
 * OriginOfCondition property."
 *
 *
 * @returns Message ConditionInRelatedResource formatted to JSON */
/* @brief 将 ConditionInRelatedResource 消息格式化为 JSON
 * 消息正文："相关资源中存在一个或多个状况。请参阅
 *  OriginOfCondition 属性。"
 * @returns 格式化为 JSON 的 ConditionInRelatedResource 消息 */
nlohmann::json::object_t conditionInRelatedResource();

void conditionInRelatedResource(crow::Response& res);

/**
 * @brief Formats RestrictedRole message into JSON
 * Message body: "The operation was not successful because the role <arg1> is
 * restricted."
 *
 * @param[in] arg1 Parameter of message that will replace %1 in its body.
 *
 * @returns Message RestrictedRole formatted to JSON */
/* @brief 将 RestrictedRole 消息格式化为 JSON
 * 消息正文："操作未成功，因为角色
 *  <arg1> 受到限制。"
 * @param[in] arg1 消息参数，将替换正文中的 %1。
 * @returns 格式化为 JSON 的 RestrictedRole 消息 */
nlohmann::json::object_t restrictedRole(std::string_view arg1);

void restrictedRole(crow::Response& res, std::string_view arg1);

/**
 * @brief Formats RestrictedPrivilege message into JSON
 * Message body: "The operation was not successful because the privilege <arg1>
 * is restricted."
 *
 * @param[in] arg1 Parameter of message that will replace %1 in its body.
 *
 * @returns Message RestrictedPrivilege formatted to JSON */
/* @brief 将 RestrictedPrivilege 消息格式化为 JSON
 * 消息正文："操作未成功，因为权限
 *  <arg1> 受到限制。"
 * @param[in] arg1 消息参数，将替换正文中的 %1。
 * @returns 格式化为 JSON 的 RestrictedPrivilege 消息 */
nlohmann::json::object_t restrictedPrivilege(std::string_view arg1);

void restrictedPrivilege(crow::Response& res, std::string_view arg1);

/**
 * @brief Formats StrictAccountTypes message into JSON
 * Message body: "The request could not be fulfilled with the account types
 * included in property <arg1> because the property StrictAccountTypes is set to
 * true."
 *
 * @param[in] arg1 Parameter of message that will replace %1 in its body.
 *
 * @returns Message StrictAccountTypes formatted to JSON */
/* @brief 将 StrictAccountTypes 消息格式化为 JSON
 * 消息正文："由于属性 StrictAccountTypes
 * 设为 true，无法使用属性 <arg1>
 * 中包含的账户类型来满足请求。"
 * @param[in] arg1 消息参数，将替换正文中的 %1。
 * @returns 格式化为 JSON 的 StrictAccountTypes 消息 */
nlohmann::json::object_t strictAccountTypes(std::string_view arg1);

void strictAccountTypes(crow::Response& res, std::string_view arg1);

/**
 * @brief Formats PropertyDeprecated message into JSON
 * Message body: "The deprecated property <arg1> was included in the request
 * body."
 *
 * @param[in] arg1 Parameter of message that will replace %1 in its body.
 *
 * @returns Message PropertyDeprecated formatted to JSON */
/* @brief 将 PropertyDeprecated 消息格式化为 JSON
 * 消息正文："请求正文中包含了已弃用的属性
 *  <arg1>。"
 * @param[in] arg1 消息参数，将替换正文中的 %1。
 * @returns 格式化为 JSON 的 PropertyDeprecated 消息 */
nlohmann::json::object_t propertyDeprecated(std::string_view arg1);

void propertyDeprecated(crow::Response& res, std::string_view arg1);

/**
 * @brief Formats ResourceDeprecated message into JSON
 * Message body: "The operation was performed on a deprecated resource <arg1>."
 *
 * @param[in] arg1 Parameter of message that will replace %1 in its body.
 *
 * @returns Message ResourceDeprecated formatted to JSON */
/* @brief 将 ResourceDeprecated 消息格式化为 JSON
 * 消息正文："操作针对的是已弃用的资源 <arg1>。"
 * @param[in] arg1 消息参数，将替换正文中的 %1。
 * @returns 格式化为 JSON 的 ResourceDeprecated 消息 */
nlohmann::json::object_t resourceDeprecated(std::string_view arg1);

void resourceDeprecated(crow::Response& res, std::string_view arg1);

/**
 * @brief Formats PropertyValueDeprecated message into JSON
 * Message body: "The value <arg1> for the property <arg2> is deprecated."
 *
 * @param[in] arg1 Parameter of message that will replace %1 in its body.
 * @param[in] arg2 Parameter of message that will replace %2 in its body.
 *
 * @returns Message PropertyValueDeprecated formatted to JSON */
/* @brief 将 PropertyValueDeprecated 消息格式化为 JSON
 * 消息正文："属性 <arg2> 的值 <arg1> 已弃用。"
 * @param[in] arg1 消息参数，将替换正文中的 %1。
 * @param[in] arg2 消息参数，将替换正文中的 %2。
 * @returns 格式化为 JSON 的 PropertyValueDeprecated 消息 */
nlohmann::json::object_t propertyValueDeprecated(std::string_view arg1,
                                                 std::string_view arg2);

void propertyValueDeprecated(crow::Response& res, std::string_view arg1,
                             std::string_view arg2);

/**
 * @brief Formats ActionDeprecated message into JSON
 * Message body: "The action <arg1> is deprecated."
 *
 * @param[in] arg1 Parameter of message that will replace %1 in its body.
 *
 * @returns Message ActionDeprecated formatted to JSON */
/* @brief 将 ActionDeprecated 消息格式化为 JSON
 * 消息正文："操作 <arg1> 已弃用。"
 * @param[in] arg1 消息参数，将替换正文中的 %1。
 * @returns 格式化为 JSON 的 ActionDeprecated 消息 */
nlohmann::json::object_t actionDeprecated(std::string_view arg1);

void actionDeprecated(crow::Response& res, std::string_view arg1);

/**
 * @brief Formats NetworkNameResolutionNotConfigured message into JSON
 * Message body: "Network name resolution is not configured on this service."
 *
 *
 * @returns Message NetworkNameResolutionNotConfigured formatted to JSON */
/* @brief 将 NetworkNameResolutionNotConfigured 消息格式化为 JSON
 * 消息正文："此服务未配置网络名称解析。"
 * @returns 格式化为 JSON 的 NetworkNameResolutionNotConfigured 消息 */
nlohmann::json::object_t networkNameResolutionNotConfigured();

void networkNameResolutionNotConfigured(crow::Response& res);

/**
 * @brief Formats NetworkNameResolutionNotSupported message into JSON
 * Message body: "Resolution of network-based names is not supported by this
 * service."
 *
 *
 * @returns Message NetworkNameResolutionNotSupported formatted to JSON */
/* @brief 将 NetworkNameResolutionNotSupported 消息格式化为 JSON
 * 消息正文："此服务不支持基于网络的名称解析。
 * "
 * @returns 格式化为 JSON 的 NetworkNameResolutionNotSupported 消息 */
nlohmann::json::object_t networkNameResolutionNotSupported();

void networkNameResolutionNotSupported(crow::Response& res);

/**
 * @brief Formats ServiceDisabled message into JSON
 * Message body: "The operation failed because the service at <arg1> is disabled
 * and cannot accept requests."
 *
 * @param[in] arg1 Parameter of message that will replace %1 in its body.
 *
 * @returns Message ServiceDisabled formatted to JSON */
/* @brief 将 ServiceDisabled 消息格式化为 JSON
 * 消息正文："操作失败，因为 <arg1>
 * 处的服务已禁用，无法接受请求。"
 * @param[in] arg1 消息参数，将替换正文中的 %1。
 * @returns 格式化为 JSON 的 ServiceDisabled 消息 */
nlohmann::json::object_t serviceDisabled(std::string_view arg1);

void serviceDisabled(crow::Response& res, std::string_view arg1);

/**
 * @brief Formats EventBufferExceeded message into JSON
 * Message body: "Undelivered events may have been lost due to exceeding the
 * event buffer."
 *
 *
 * @returns Message EventBufferExceeded formatted to JSON */
/* @brief 将 EventBufferExceeded 消息格式化为 JSON
 * 消息正文："由于超出事件缓冲区，可能已丢失未送达的事件。
 * "
 * @returns 格式化为 JSON 的 EventBufferExceeded 消息 */
nlohmann::json::object_t eventBufferExceeded();

void eventBufferExceeded(crow::Response& res);

/**
 * @brief Formats AuthenticationTokenRequired message into JSON
 * Message body: "The request could not be performed because an authentication
 * token was not provided."
 *
 *
 * @returns Message AuthenticationTokenRequired formatted to JSON */
/* @brief 将 AuthenticationTokenRequired 消息格式化为 JSON
 * 消息正文："由于未提供认证令牌，无法执行该请求。
 * "
 * @returns 格式化为 JSON 的 AuthenticationTokenRequired 消息 */
nlohmann::json::object_t authenticationTokenRequired();

void authenticationTokenRequired(crow::Response& res);

/**
 * @brief Formats OneTimePasscodeSent message into JSON
 * Message body: "A one-time passcode was sent to: <arg1>.  Supply the passcode
 * as the `Token` property in the request to create a session."
 *
 * @param[in] arg1 Parameter of message that will replace %1 in its body.
 *
 * @returns Message OneTimePasscodeSent formatted to JSON */
/* @brief 将 OneTimePasscodeSent 消息格式化为 JSON
 * 消息正文："已向 <arg1> 发送一次性密码。请在创建会话的请求中将该密码作为
 *  `Token` 属性提供。"
 * @param[in] arg1 消息参数，将替换正文中的 %1。
 * @returns 格式化为 JSON 的 OneTimePasscodeSent 消息 */
nlohmann::json::object_t oneTimePasscodeSent(std::string_view arg1);

void oneTimePasscodeSent(crow::Response& res, std::string_view arg1);

/**
 * @brief Formats LicenseRequired message into JSON
 * Message body: "A license is required for this operation: <arg1>."
 *
 * @param[in] arg1 Parameter of message that will replace %1 in its body.
 *
 * @returns Message LicenseRequired formatted to JSON */
/* @brief 将 LicenseRequired 消息格式化为 JSON
 * 消息正文："此操作需要许可证：<arg1>。"
 * @param[in] arg1 消息参数，将替换正文中的 %1。
 * @returns 格式化为 JSON 的 LicenseRequired 消息 */
nlohmann::json::object_t licenseRequired(std::string_view arg1);

void licenseRequired(crow::Response& res, std::string_view arg1);

/**
 * @brief Formats PropertyModified message into JSON
 * Message body: "One or more properties were successfully modified."
 *
 *
 * @returns Message PropertyModified formatted to JSON */
/* @brief 将 PropertyModified 消息格式化为 JSON
 * 消息正文："一个或多个属性已成功修改。"
 * @returns 格式化为 JSON 的 PropertyModified 消息 */
nlohmann::json::object_t propertyModified();

void propertyModified(crow::Response& res);

/**
 * @brief Formats GenerateSecretKeyRequired message into JSON
 * Message body: "The Time-based One-Time Password (TOTP) secret key for this
 * account must be generated before access is granted.  Perform the
 * GenerateSecretKey action at URI <arg1> and retain the secret key from the
 * response."
 *
 * @param[in] arg1 Parameter of message that will replace %1 in its body.
 *
 * @returns Message GenerateSecretKeyRequired formatted to JSON */
/* @brief 将 GenerateSecretKeyRequired 消息格式化为 JSON
 * 消息正文："在获得访问权限之前，必须为此账户生成基于时间的一次性密码（
 * TOTP）密钥。请在 URI <arg1>
 * 处执行 GenerateSecretKey
 * 操作，并保留响应中的密钥。"
 * @param[in] arg1 消息参数，将替换正文中的 %1。
 * @returns 格式化为 JSON 的 GenerateSecretKeyRequired 消息 */
nlohmann::json::object_t generateSecretKeyRequired(
    const boost::urls::url_view_base& arg1);

void generateSecretKeyRequired(crow::Response& res,
                               const boost::urls::url_view_base& arg1);

} // namespace messages
} // namespace redfish
