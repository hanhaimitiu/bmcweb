// SPDX-License-Identifier: Apache-2.0
// SPDX-FileCopyrightText: Copyright OpenBMC Authors
// SPDX-FileCopyrightText: Copyright 2018 Intel Corporation
#pragma once

#include "error_messages.hpp"
#include "http_request.hpp"
#include "http_response.hpp"
#include "human_sort.hpp"
#include "logging.hpp"

#include <boost/system/result.hpp>
#include <boost/url/parse.hpp>
#include <boost/url/url_view.hpp>
#include <nlohmann/json.hpp>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <map>
#include <optional>
#include <ranges>
#include <span>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

namespace redfish
{

namespace json_util
{

/**
 * @brief Processes request to extract JSON from its body. If it fails, adds
 *       MalformedJSON message to response and ends it.
 *
 * @param[io]  res       Response object
 * @param[in]  req       Request object
 * @param[out] reqJson   JSON object extracted from request's body
 *
 * @return true if JSON is valid, false when JSON is invalid and response has
 *         been filled with message and ended.
 */
/* @brief 处理请求，从其 body 中提取 JSON。若失败，则向响应添加
 *       MalformedJSON 消息并结束响应。
 * @param[io]  res       响应对象
 * @param[in]  req       请求对象
 * @param[out] reqJson   从请求 body 中提取的 JSON 对象
 * @return JSON 有效时返回 true；JSON 无效且响应已填充消息并结束时
 *         返回 false。 */
bool processJsonFromRequest(crow::Response& res, const crow::Request& req,
                            nlohmann::json& reqJson);
namespace details
{

template <typename Type>
struct IsOptional : std::false_type
{};

template <typename Type>
struct IsOptional<std::optional<Type>> : std::true_type
{};

template <typename Type>
struct IsVector : std::false_type
{};

template <typename Type>
struct IsVector<std::vector<Type>> : std::true_type
{};

template <typename Type>
struct IsStdArray : std::false_type
{};

template <typename Type, std::size_t size>
struct IsStdArray<std::array<Type, size>> : std::true_type
{};

template <typename Type>
struct IsVariant : std::false_type
{};

template <typename... Types>
struct IsVariant<std::variant<Types...>> : std::true_type
{};

enum class UnpackErrorCode
{
    success,
    invalidType,
    outOfRange
};

template <typename ToType, typename FromType>
bool checkRange(const FromType& from, std::string_view key)
{
    if constexpr (std::is_floating_point_v<ToType>)
    {
        if (std::isnan(from))
        {
            BMCWEB_LOG_DEBUG("Value for key {} was NAN", key);
            return false;
        }
        // Assume for the moment that all floats can represent the full range
        // of any int/uint in a cast.  This is close enough to true for the
        // precision of this json parser.
        // 暂时假设所有 float 都能在转换中完整表示任意 int/uint 的
        // 取值范围。就本 json 解析器的精度而言，这一假设足够接近
        // 事实。
    }
    else
    {
        if (std::cmp_greater(from, std::numeric_limits<ToType>::max()))
        {
            BMCWEB_LOG_DEBUG("Value for key {} was greater than max {}", key,
                             std::numeric_limits<FromType>::max());
            return false;
        }
        if (std::cmp_less(from, std::numeric_limits<ToType>::lowest()))
        {
            BMCWEB_LOG_DEBUG("Value for key {} was less than min {}", key,
                             std::numeric_limits<FromType>::lowest());
            return false;
        }
    }

    return true;
}

template <typename Type>
UnpackErrorCode unpackValueWithErrorCode(nlohmann::json& jsonValue,
                                         std::string_view key, Type& value);

template <std::size_t Index = 0, typename... Args>
UnpackErrorCode unpackValueVariant(nlohmann::json& j, std::string_view key,
                                   std::variant<Args...>& v)
{
    if constexpr (Index < std::variant_size_v<std::variant<Args...>>)
    {
        std::variant_alternative_t<Index, std::variant<Args...>> type{};
        UnpackErrorCode unpack = unpackValueWithErrorCode(j, key, type);
        if (unpack == UnpackErrorCode::success)
        {
            v = std::move(type);
            return unpack;
        }

        return unpackValueVariant<Index + 1, Args...>(j, key, v);
    }
    return UnpackErrorCode::invalidType;
}

template <typename Type>
UnpackErrorCode unpackValueWithErrorCode(nlohmann::json& jsonValue,
                                         std::string_view key, Type& value)
{
    UnpackErrorCode ret = UnpackErrorCode::success;

    if constexpr (std::is_floating_point_v<Type>)
    {
        double helper = 0;
        double* jsonPtr = jsonValue.get_ptr<double*>();

        if (jsonPtr == nullptr)
        {
            int64_t* intPtr = jsonValue.get_ptr<int64_t*>();
            if (intPtr != nullptr)
            {
                helper = static_cast<double>(*intPtr);
                jsonPtr = &helper;
            }
        }
        if (jsonPtr == nullptr)
        {
            uint64_t* uintPtr = jsonValue.get_ptr<uint64_t*>();
            if (uintPtr != nullptr)
            {
                helper = static_cast<double>(*uintPtr);
                jsonPtr = &helper;
            }
        }
        if (jsonPtr == nullptr)
        {
            return UnpackErrorCode::invalidType;
        }
        if (!checkRange<Type>(*jsonPtr, key))
        {
            return UnpackErrorCode::outOfRange;
        }
        value = static_cast<Type>(*jsonPtr);
    }

    else if constexpr (std::is_signed_v<Type>)
    {
        int64_t* jsonPtr = jsonValue.get_ptr<int64_t*>();
        if (jsonPtr == nullptr)
        {
            // Value wasn't int, check uint
            // 该值不是 int，检查是否为 uint
            uint64_t* uJsonPtr = jsonValue.get_ptr<uint64_t*>();
            if (uJsonPtr == nullptr)
            {
                return UnpackErrorCode::invalidType;
            }
            if (!checkRange<Type>(*uJsonPtr, key))
            {
                return UnpackErrorCode::outOfRange;
            }
            value = static_cast<Type>(*uJsonPtr);
        }
        else
        {
            if (!checkRange<Type>(*jsonPtr, key))
            {
                return UnpackErrorCode::outOfRange;
            }
            value = static_cast<Type>(*jsonPtr);
        }
    }

    else if constexpr ((std::is_unsigned_v<Type>) &&
                       (!std::is_same_v<bool, Type>))
    {
        uint64_t* jsonPtr = jsonValue.get_ptr<uint64_t*>();
        if (jsonPtr == nullptr)
        {
            int64_t* ijsonPtr = jsonValue.get_ptr<int64_t*>();
            if (ijsonPtr == nullptr)
            {
                return UnpackErrorCode::invalidType;
            }
            if (!checkRange<Type>(*ijsonPtr, key))
            {
                return UnpackErrorCode::outOfRange;
            }
            value = static_cast<Type>(*ijsonPtr);
        }
        else
        {
            if (!checkRange<Type>(*jsonPtr, key))
            {
                return UnpackErrorCode::outOfRange;
            }
            value = static_cast<Type>(*jsonPtr);
        }
    }

    else if constexpr (std::is_same_v<nlohmann::json, Type>)
    {
        value = std::move(jsonValue);
    }
    else if constexpr (std::is_same_v<std::nullptr_t, Type>)
    {
        if (!jsonValue.is_null())
        {
            return UnpackErrorCode::invalidType;
        }
    }
    else if constexpr (IsVector<Type>::value)
    {
        nlohmann::json::object_t* obj =
            jsonValue.get_ptr<nlohmann::json::object_t*>();
        if (obj == nullptr)
        {
            return UnpackErrorCode::invalidType;
        }

        for (const auto& val : *obj)
        {
            value.emplace_back();
            ret = unpackValueWithErrorCode<typename Type::value_type>(
                      val, key, value.back()) &&
                  ret;
        }
    }
    else
    {
        using JsonType = std::add_const_t<std::add_pointer_t<Type>>;
        JsonType jsonPtr = jsonValue.get_ptr<JsonType>();
        if (jsonPtr == nullptr)
        {
            BMCWEB_LOG_DEBUG("Value for key {} was incorrect type: {}", key,
                             jsonValue.type_name());
            return UnpackErrorCode::invalidType;
        }
        value = std::move(*jsonPtr);
    }
    return ret;
}

template <typename Type>
bool unpackValue(nlohmann::json& jsonValue, std::string_view key,
                 crow::Response& res, Type& value)
{
    bool ret = true;

    if constexpr (IsOptional<Type>::value)
    {
        value.emplace();
        ret = unpackValue<typename Type::value_type>(jsonValue, key, res,
                                                     *value) &&
              ret;
    }
    else if constexpr (IsStdArray<Type>::value)
    {
        nlohmann::json::array_t* arr =
            jsonValue.get_ptr<nlohmann::json::array_t*>();
        if (arr == nullptr)
        {
            messages::propertyValueTypeError(res, jsonValue, key);
            return false;
        }
        if (jsonValue.size() != value.size())
        {
            messages::propertyValueTypeError(res, jsonValue, key);
            return false;
        }
        size_t index = 0;
        for (auto& val : *arr)
        {
            ret = unpackValue<typename Type::value_type>(val, key, res,
                                                         value[index++]) &&
                  ret;
        }
    }
    else if constexpr (IsVector<Type>::value)
    {
        nlohmann::json::array_t* arr =
            jsonValue.get_ptr<nlohmann::json::array_t*>();
        if (arr == nullptr)
        {
            messages::propertyValueTypeError(res, jsonValue, key);
            return false;
        }

        for (auto& val : *arr)
        {
            value.emplace_back();
            ret = unpackValue<typename Type::value_type>(val, key, res,
                                                         value.back()) &&
                  ret;
        }
    }
    else if constexpr (IsVariant<Type>::value)
    {
        UnpackErrorCode ec = unpackValueVariant(jsonValue, key, value);
        if (ec != UnpackErrorCode::success)
        {
            if (ec == UnpackErrorCode::invalidType)
            {
                messages::propertyValueTypeError(res, jsonValue, key);
            }
            else if (ec == UnpackErrorCode::outOfRange)
            {
                messages::propertyValueOutOfRange(res, jsonValue, key);
            }
            return false;
        }
    }
    else
    {
        UnpackErrorCode ec = unpackValueWithErrorCode(jsonValue, key, value);
        if (ec != UnpackErrorCode::success)
        {
            if (ec == UnpackErrorCode::invalidType)
            {
                messages::propertyValueTypeError(res, jsonValue, key);
            }
            else if (ec == UnpackErrorCode::outOfRange)
            {
                messages::propertyValueOutOfRange(res, jsonValue, key);
            }
            return false;
        }
    }

    return ret;
}

template <typename Type>
bool unpackValue(nlohmann::json& jsonValue, std::string_view key, Type& value)
{
    bool ret = true;
    if constexpr (IsOptional<Type>::value)
    {
        value.emplace();
        ret = unpackValue<typename Type::value_type>(jsonValue, key, *value) &&
              ret;
    }
    else if constexpr (IsStdArray<Type>::value)
    {
        nlohmann::json::array_t* arr =
            jsonValue.get_ptr<nlohmann::json::array_t*>();
        if (arr == nullptr)
        {
            return false;
        }
        if (jsonValue.size() != value.size())
        {
            return false;
        }
        size_t index = 0;
        for (const auto& val : *arr)
        {
            ret = unpackValue<typename Type::value_type>(val, key,
                                                         value[index++]) &&
                  ret;
        }
    }
    else if constexpr (IsVector<Type>::value)
    {
        nlohmann::json::array_t* arr =
            jsonValue.get_ptr<nlohmann::json::array_t*>();
        if (arr == nullptr)
        {
            return false;
        }

        for (const auto& val : *arr)
        {
            value.emplace_back();
            ret = unpackValue<typename Type::value_type>(val, key,
                                                         value.back()) &&
                  ret;
        }
    }
    else
    {
        UnpackErrorCode ec = unpackValueWithErrorCode(jsonValue, key, value);
        if (ec != UnpackErrorCode::success)
        {
            return false;
        }
    }

    return ret;
}

//  boost::hash_combine
//  对应 boost::hash_combine（Boost 的哈希组合实现）
inline std::size_t combine(std::size_t seed, std::size_t h) noexcept
{
    seed ^= h + 0x9e3779b9 + (seed << 6U) + (seed >> 2U);
    return seed;
}
} // namespace details

// clang-format off
using UnpackVariant = std::variant<
    uint8_t*,
    uint16_t*,
    int16_t*,
    uint32_t*,
    int32_t*,
    uint64_t*,
    int64_t*,
    bool*,
    double*,
    std::string*,
    nlohmann::json::object_t*,
    std::variant<std::string, std::nullptr_t>*,
    std::variant<uint8_t, std::nullptr_t>*,
    std::variant<int16_t, std::nullptr_t>*,
    std::variant<uint16_t, std::nullptr_t>*,
    std::variant<int32_t, std::nullptr_t>*,
    std::variant<uint32_t, std::nullptr_t>*,
    std::variant<int64_t, std::nullptr_t>*,
    std::variant<uint64_t, std::nullptr_t>*,
    std::variant<double, std::nullptr_t>*,
    std::variant<bool, std::nullptr_t>*,
    std::vector<uint8_t>*,
    std::vector<uint16_t>*,
    std::vector<int16_t>*,
    std::vector<uint32_t>*,
    std::vector<int32_t>*,
    std::vector<uint64_t>*,
    std::vector<int64_t>*,
    //std::vector<bool>*,
    // std::vector<bool>*（bool 向量指针类型），
    std::vector<double>*,
    std::vector<std::string>*,
    std::vector<nlohmann::json::object_t>*,
    std::optional<uint8_t>*,
    std::optional<uint16_t>*,
    std::optional<int16_t>*,
    std::optional<uint32_t>*,
    std::optional<int32_t>*,
    std::optional<uint64_t>*,
    std::optional<int64_t>*,
    std::optional<bool>*,
    std::optional<double>*,
    std::optional<std::string>*,
    std::optional<nlohmann::json::object_t>*,
    std::optional<std::vector<uint8_t>>*,
    std::optional<std::vector<uint16_t>>*,
    std::optional<std::vector<int16_t>>*,
    std::optional<std::vector<uint32_t>>*,
    std::optional<std::vector<int32_t>>*,
    std::optional<std::vector<uint64_t>>*,
    std::optional<std::vector<int64_t>>*,
    //std::optional<std::vector<bool>>*,
    // std::optional<std::vector<bool>>*（可选 bool 向量指针类型），
    std::optional<std::vector<double>>*,
    std::optional<std::vector<std::string>>*,
    std::optional<std::vector<nlohmann::json::object_t>>*,
    std::optional<std::variant<std::string, std::nullptr_t>>*,
    std::optional<std::variant<uint8_t, std::nullptr_t>>*,
    std::optional<std::variant<int16_t, std::nullptr_t>>*,
    std::optional<std::variant<uint16_t, std::nullptr_t>>*,
    std::optional<std::variant<int32_t, std::nullptr_t>>*,
    std::optional<std::variant<uint32_t, std::nullptr_t>>*,
    std::optional<std::variant<int64_t, std::nullptr_t>>*,
    std::optional<std::variant<uint64_t, std::nullptr_t>>*,
    std::optional<std::variant<double, std::nullptr_t>>*,
    std::optional<std::variant<bool, std::nullptr_t>>*,
    std::optional<std::vector<std::variant<nlohmann::json::object_t, std::nullptr_t>>>*,
    std::optional<std::vector<std::variant<std::string, nlohmann::json::object_t, std::nullptr_t>>>*
>;
// clang-format on

struct PerUnpack
{
    std::string_view key;
    UnpackVariant value;
    bool complete = false;
};

inline bool readJsonHelperObject(nlohmann::json::object_t& obj,
                                 crow::Response& res,
                                 std::span<PerUnpack> toUnpack)
{
    bool result = true;
    for (auto& item : obj)
    {
        size_t unpackIndex = 0;
        for (; unpackIndex < toUnpack.size(); unpackIndex++)
        {
            PerUnpack& unpackSpec = toUnpack[unpackIndex];
            std::string_view key = unpackSpec.key;
            size_t keysplitIndex = key.find('/');
            std::string_view leftover;
            if (keysplitIndex != std::string_view::npos)
            {
                leftover = key.substr(keysplitIndex + 1);
                key = key.substr(0, keysplitIndex);
            }

            if (key != item.first || unpackSpec.complete)
            {
                continue;
            }

            // Sublevel key
            // 下级 key（子层级键）
            if (!leftover.empty())
            {
                // Include the slash in the key so we can compare later
                // 在 key 中包含斜杠，以便后续进行比较
                key = unpackSpec.key.substr(0, keysplitIndex + 1);
                nlohmann::json::object_t j;
                result = details::unpackValue<nlohmann::json::object_t>(
                             item.second, key, res, j) &&
                         result;
                if (!result)
                {
                    return result;
                }

                std::vector<PerUnpack> nextLevel;
                for (PerUnpack& p : toUnpack)
                {
                    if (!p.key.starts_with(key))
                    {
                        continue;
                    }
                    std::string_view thisLeftover = p.key.substr(key.size());
                    nextLevel.push_back({thisLeftover, p.value, false});
                    p.complete = true;
                }

                result = readJsonHelperObject(j, res, nextLevel) && result;
                break;
            }

            result =
                std::visit(
                    [&item, &unpackSpec, &res](auto& val) {
                        using ContainedT =
                            std::remove_pointer_t<std::decay_t<decltype(val)>>;
                        return details::unpackValue<ContainedT>(
                            item.second, unpackSpec.key, res, *val);
                    },
                    unpackSpec.value) &&
                result;

            unpackSpec.complete = true;
            break;
        }

        if (unpackIndex == toUnpack.size())
        {
            messages::propertyUnknown(res, item.first);
            result = false;
        }
    }

    for (PerUnpack& perUnpack : toUnpack)
    {
        if (!perUnpack.complete)
        {
            bool isOptional = std::visit(
                [](auto& val) {
                    using ContainedType =
                        std::remove_pointer_t<std::decay_t<decltype(val)>>;
                    return details::IsOptional<ContainedType>::value;
                },
                perUnpack.value);
            if (isOptional)
            {
                continue;
            }
            messages::propertyMissing(res, perUnpack.key);
            result = false;
        }
    }
    return result;
}

inline void packVariant(std::span<PerUnpack> /*toPack*/) {}

template <typename FirstType, typename... UnpackTypes>
void packVariant(std::span<PerUnpack> toPack, std::string_view key,
                 FirstType&& first, UnpackTypes&&... in)
{
    if (toPack.empty())
    {
        return;
    }
    toPack[0].key = key;
    toPack[0].value = &first;
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-array-to-pointer-decay)
    packVariant(toPack.subspan(1), std::forward<UnpackTypes&&>(in)...);
}

template <typename FirstType, typename... UnpackTypes>
bool readJsonObject(nlohmann::json::object_t& jsonRequest, crow::Response& res,
                    std::string_view key, FirstType&& first,
                    UnpackTypes&&... in)
{
    const std::size_t n = sizeof...(UnpackTypes) + 2;
    std::array<PerUnpack, n / 2> toUnpack2;
    packVariant(toUnpack2, key, std::forward<FirstType>(first),
                std::forward<UnpackTypes&&>(in)...);
    return readJsonHelperObject(jsonRequest, res, toUnpack2);
}

template <typename FirstType, typename... UnpackTypes>
bool readJson(nlohmann::json& jsonRequest, crow::Response& res,
              std::string_view key, FirstType&& first, UnpackTypes&&... in)
{
    nlohmann::json::object_t* obj =
        jsonRequest.get_ptr<nlohmann::json::object_t*>();
    if (obj == nullptr)
    {
        BMCWEB_LOG_DEBUG("Json value is not an object");
        messages::unrecognizedRequestBody(res);
        return false;
    }
    return readJsonObject(*obj, res, key, std::forward<FirstType>(first),
                          std::forward<UnpackTypes&&>(in)...);
}

inline std::optional<nlohmann::json::object_t> readJsonPatchHelper(
    const crow::Request& req, crow::Response& res)
{
    nlohmann::json jsonRequest;
    if (!json_util::processJsonFromRequest(res, req, jsonRequest))
    {
        BMCWEB_LOG_DEBUG("Json value not readable");
        return std::nullopt;
    }
    nlohmann::json::object_t* object =
        jsonRequest.get_ptr<nlohmann::json::object_t*>();
    if (object == nullptr || object->empty())
    {
        BMCWEB_LOG_DEBUG("Json value is empty");
        messages::emptyJSON(res);
        return std::nullopt;
    }
    std::erase_if(*object,
                  [](const std::pair<std::string, nlohmann::json>& item) {
                      return item.first.starts_with("@odata.");
                  });
    if (object->empty())
    {
        //  If the update request only contains OData annotations, the service
        //  should return the HTTP 400 Bad Request status code with the
        //  NoOperation message from the Base Message Registry, ...
        //  如果更新请求只包含 OData 注解，服务应返回 HTTP 400 Bad Request
        //  状态码，并附带来自 Base Message Registry 的 NoOperation 消息，
        //  ...
        messages::noOperation(res);
        return std::nullopt;
    }

    return {std::move(*object)};
}

inline const nlohmann::json* findNestedKey(std::string_view key,
                                           const nlohmann::json& value)
{
    size_t keysplitIndex = key.find('/');
    std::string_view leftover;
    nlohmann::json::const_iterator it;
    if (keysplitIndex != std::string_view::npos)
    {
        const nlohmann::json::object_t* obj =
            value.get_ptr<const nlohmann::json::object_t*>();
        if (obj == nullptr || obj->empty())
        {
            BMCWEB_LOG_ERROR("Requested key wasn't an object");
            return nullptr;
        }

        leftover = key.substr(keysplitIndex + 1);
        std::string_view keypart = key.substr(0, keysplitIndex);
        it = value.find(keypart);
        if (it == value.end())
        {
            // Entry didn't have key
            // 条目中没有该 key
            return nullptr;
        }
        return findNestedKey(leftover, it.value());
    }

    it = value.find(key);
    if (it == value.end())
    {
        return nullptr;
    }
    return &*it;
}

template <typename... UnpackTypes>
bool readJsonPatch(const crow::Request& req, crow::Response& res,
                   std::string_view key, UnpackTypes&&... in)
{
    std::optional<nlohmann::json::object_t> jsonRequest =
        readJsonPatchHelper(req, res);
    if (!jsonRequest)
    {
        return false;
    }
    if (jsonRequest->empty())
    {
        messages::emptyJSON(res);
        return false;
    }

    return readJsonObject(*jsonRequest, res, key,
                          std::forward<UnpackTypes&&>(in)...);
}

inline std::optional<nlohmann::json::json_pointer>
    createJsonPointerFromFragment(std::string_view input)
{
    auto hashPos = input.find('#');
    if (hashPos == std::string_view::npos || hashPos + 1 >= input.size())
    {
        BMCWEB_LOG_ERROR(
            "createJsonPointerFromFragment() No fragment found after #");
        return std::nullopt;
    }

    std::string_view fragment = input.substr(hashPos + 1);
    return nlohmann::json::json_pointer(std::string(fragment));
}

template <typename... UnpackTypes>
bool readJsonAction(const crow::Request& req, crow::Response& res,
                    const char* key, UnpackTypes&&... in)
{
    nlohmann::json jsonRequest;
    if (!json_util::processJsonFromRequest(res, req, jsonRequest))
    {
        BMCWEB_LOG_DEBUG("Json value not readable");
        return false;
    }
    nlohmann::json::object_t* object =
        jsonRequest.get_ptr<nlohmann::json::object_t*>();
    if (object == nullptr)
    {
        BMCWEB_LOG_DEBUG("Json value is empty");
        messages::emptyJSON(res);
        return false;
    }
    return readJsonObject(*object, res, key,
                          std::forward<UnpackTypes&&>(in)...);
}

// Determines if two json objects are less, based on the presence of the
// @odata.id key
// 根据 @odata.id key 是否存在，判断两个 json 对象的大小
// 顺序
inline int objectKeyCmp(std::string_view key, const nlohmann::json& a,
                        const nlohmann::json& b)
{
    using object_t = nlohmann::json::object_t;
    const object_t* aObj = a.get_ptr<const object_t*>();
    const object_t* bObj = b.get_ptr<const object_t*>();

    if (aObj == nullptr)
    {
        if (bObj == nullptr)
        {
            return 0;
        }
        return -1;
    }
    if (bObj == nullptr)
    {
        return 1;
    }
    object_t::const_iterator aIt = aObj->find(key);
    object_t::const_iterator bIt = bObj->find(key);
    // If either object doesn't have the key, they get "sorted" to the
    // beginning.
    // 若任一对象没有该 key，它会被"排序"到
    // 开头。
    if (aIt == aObj->end())
    {
        if (bIt == bObj->end())
        {
            return 0;
        }
        return -1;
    }
    if (bIt == bObj->end())
    {
        return 1;
    }
    const nlohmann::json::string_t* nameA =
        aIt->second.get_ptr<const std::string*>();
    const nlohmann::json::string_t* nameB =
        bIt->second.get_ptr<const std::string*>();
    // If either object doesn't have a string as the key, they get "sorted" to
    // the beginning.
    // 若任一对象的 key 不是字符串，它会被"排序"到
    // 开头。
    if (nameA == nullptr)
    {
        if (nameB == nullptr)
        {
            return 0;
        }
        return -1;
    }
    if (nameB == nullptr)
    {
        return 1;
    }
    if (key != "@odata.id")
    {
        return alphanumComp(*nameA, *nameB);
    }

    boost::system::result<boost::urls::url_view> aUrl =
        boost::urls::parse_relative_ref(*nameA);
    boost::system::result<boost::urls::url_view> bUrl =
        boost::urls::parse_relative_ref(*nameB);
    if (!aUrl)
    {
        if (!bUrl)
        {
            return 0;
        }
        return -1;
    }
    if (!bUrl)
    {
        return 1;
    }

    auto segmentsAIt = aUrl->segments().begin();
    auto segmentsBIt = bUrl->segments().begin();

    while (true)
    {
        if (segmentsAIt == aUrl->segments().end())
        {
            if (segmentsBIt == bUrl->segments().end())
            {
                return 0;
            }
            return -1;
        }
        if (segmentsBIt == bUrl->segments().end())
        {
            return 1;
        }
        int res = alphanumComp(*segmentsAIt, *segmentsBIt);
        if (res != 0)
        {
            return res;
        }

        segmentsAIt++;
        segmentsBIt++;
    }
    return 0;
};

// kept for backward compatibility
// 保留以保持向后兼容
inline int odataObjectCmp(const nlohmann::json& left,
                          const nlohmann::json& right)
{
    return objectKeyCmp("@odata.id", left, right);
}

struct ODataObjectLess
{
    std::string_view key;

    explicit ODataObjectLess(std::string_view keyIn) : key(keyIn) {}

    bool operator()(const nlohmann::json& left,
                    const nlohmann::json& right) const
    {
        return objectKeyCmp(key, left, right) < 0;
    }
};

// Sort the JSON array by |element[key]|.
// Elements without |key| or type of |element[key]| is not string are smaller
// those whose |element[key]| is string.
// 按 |element[key]| 对 JSON 数组排序。
// 没有 |key| 或 |element[key]| 类型不是字符串的元素，排在
// |element[key]| 为字符串的元素之前（更小）。
inline void sortJsonArrayByKey(nlohmann::json::array_t& array,
                               std::string_view key)
{
    std::ranges::sort(array, ODataObjectLess(key));
}

// Sort the JSON array by |element[key]|.
// Elements without |key| or type of |element[key]| is not string are smaller
// those whose |element[key]| is string.
// 按 |element[key]| 对 JSON 数组排序。
// 没有 |key| 或 |element[key]| 类型不是字符串的元素，排在
// |element[key]| 为字符串的元素之前（更小）。
inline void sortJsonArrayByOData(nlohmann::json::array_t& array)
{
    std::ranges::sort(array, ODataObjectLess("@odata.id"));
}

// Returns the estimated size of the JSON value
// The implementation walks through every key and every value, accumulates the
//  total size of keys and values.
// Ideally, we should use a custom allocator that nlohmann JSON supports.
// 返回 JSON 值的估算大小
// 实现会遍历每个 key 和每个 value，累加所有 key 和 value 的
// 总大小。
// 理想情况下，应使用 nlohmann JSON 支持的自定义分配器。

// Assumption made:
//  1. number: 8 characters
//  2. boolean: 5 characters (False)
//  3. string: len(str) + 2 characters (quote)
//  4. bytes: len(bytes) characters
//  5. null: 4 characters (null)
// 所做假设：
//  1. number（数字）：8 个字符
//  2. boolean（布尔）：5 个字符（False）
//  3. string（字符串）：len(str) + 2 个字符（引号）
//  4. bytes（字节）：len(bytes) 个字符
//  5. null：4 个字符（null）
uint64_t getEstimatedJsonSize(const nlohmann::json& root);

// Hashes a json value, recursively omitting every member with key `keyToIgnore`
// 对 json 值计算哈希，递归忽略 key 为 `keyToIgnore` 的所有成员
inline size_t hashJsonWithoutKey(const nlohmann::json& jsonValue,
                                 std::string_view keyToIgnore)
{
    const nlohmann::json::object_t* obj =
        jsonValue.get_ptr<const nlohmann::json::object_t*>();
    if (obj == nullptr)
    {
        // Object has no keys to remove so just return hash
        // 对象中没有需要移除的 key，因此直接返回哈希值
        return std::hash<nlohmann::json>{}(jsonValue);
    }

    const size_t type = static_cast<std::size_t>(jsonValue.type());
    size_t seed = details::combine(type, jsonValue.size());
    for (const auto& element : *obj)
    {
        const size_t h = std::hash<std::string>{}(element.first);
        seed = details::combine(seed, h);
        if (element.first != keyToIgnore)
        {
            seed = details::combine(
                seed, std::hash<nlohmann::json>{}(element.second));
        }
    }
    return seed;
}

} // namespace json_util
} // namespace redfish
