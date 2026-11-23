// SPDX-License-Identifier: Apache-2.0
// SPDX-FileCopyrightText: Copyright OpenBMC Authors
// SPDX-FileCopyrightText: Copyright 2018 Intel Corporation
#pragma once

#include "logging.hpp"
#include "sessions.hpp"

#include <boost/beast/http/verb.hpp>
#include <boost/container/flat_map.hpp>
#include <boost/container/vector.hpp>

#include <array>
#include <bitset>
#include <cstddef>
#include <initializer_list>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace redfish
{

enum class PrivilegeType
{
    BASE,
    OEM
};

/** @brief A fixed array of compile time privileges  */
/* @brief 编译期权限的固定数组 */
constexpr std::array<std::string_view, 5> basePrivileges{
    "Login", "ConfigureManager", "ConfigureComponents", "ConfigureSelf",
    "ConfigureUsers"};

constexpr const size_t basePrivilegeCount = basePrivileges.size();

/** @brief Max number of privileges per type  */
/* @brief 每种类型的最大权限数 */
constexpr const size_t maxPrivilegeCount = 32;

/**
 * @brief A vector of all privilege names and their indexes
 * The privilege "OpenBMCHostConsole" is added to users who are members of the
 * "hostconsole" user group. This privilege is required to access the host
 * console.
 */
/* @brief 所有权限名称及其索引组成的 vector
 * 权限 "OpenBMCHostConsole" 会添加到属于
 * "hostconsole" 用户组的用户。访问主机控制台
 * 需要此权限。 */
constexpr std::array<std::string_view, maxPrivilegeCount> privilegeNames{
    "Login",         "ConfigureManager", "ConfigureComponents",
    "ConfigureSelf", "ConfigureUsers",   "OpenBMCHostConsole"};

/**
 * @brief Redfish privileges
 *
 *        This implements a set of Redfish privileges.  These directly represent
 *        user privileges and help represent entity privileges.
 *
 *        Each incoming Connection requires a comparison between privileges held
 *        by the user issuing a request and the target entity's privileges.
 *
 *        To ensure best runtime performance of this comparison, privileges
 *        are represented as bitsets. Each bit in the bitset corresponds to a
 *        unique privilege name.
 *
 *        A bit is set if the privilege is required (entity domain) or granted
 *        (user domain) and false otherwise.
 *
 */
/* @brief Redfish 权限
 *        这里实现了一组 Redfish 权限。它们直接表示
 *        用户权限，并有助于表示实体权限。
 *        每个传入的 Connection 都需要比较发起请求的用户
 *        所持有的权限与目标实体的权限。
 *        为确保此比较具有最佳运行时性能，权限用
 *        bitset 表示。bitset 中的每一位对应一个
 *        唯一的权限名称。
 *        当某权限是被要求的（实体域）或已被授予的（用户域）时，
 *        相应的位被置位，否则为 false。 */
class Privileges
{
  public:
    /**
     * @brief Constructs object without any privileges active
     *
     */
    /* @brief 构造一个不激活任何权限的对象 */
    Privileges() = default;

    /**
     * @brief Constructs object with given privileges active
     *
     * @param[in] privilegeList  List of privileges to be activated
     *
     */
    /* @brief 构造一个激活给定权限的对象
     * @param[in] privilegeList  要激活的权限列表 */
    Privileges(std::initializer_list<const char*> privilegeList)
    {
        for (const char* privilege : privilegeList)
        {
            if (!setSinglePrivilege(privilege))
            {
                BMCWEB_LOG_CRITICAL("Unable to set privilege {} in constructor",
                                    privilege);
            }
        }
    }

    /**
     * @brief Sets given privilege in the bitset
     *
     * @param[in] privilege  Privilege to be set
     *
     * @return               None
     *
     */
    /* @brief 在 bitset 中设置给定的权限
     * @param[in] privilege  要设置的权限
     * @return               无 */
    bool setSinglePrivilege(std::string_view privilege)
    {
        for (size_t searchIndex = 0; searchIndex < privilegeNames.size();
             searchIndex++)
        {
            if (privilege == privilegeNames[searchIndex])
            {
                privilegeBitset.set(searchIndex);
                return true;
            }
        }

        return false;
    }

    /**
     * @brief Resets the given privilege in the bitset
     *
     * @param[in] privilege  Privilege to be reset
     *
     * @return               None
     *
     */
    /* @brief 在 bitset 中重置给定的权限
     * @param[in] privilege  要重置的权限
     * @return               无 */
    bool resetSinglePrivilege(const char* privilege)
    {
        for (size_t searchIndex = 0; searchIndex < privilegeNames.size();
             searchIndex++)
        {
            if (privilege == privilegeNames[searchIndex])
            {
                privilegeBitset.reset(searchIndex);
                return true;
            }
        }
        return false;
    }

    /**
     * @brief Retrieves names of all active privileges for a given type
     *
     * @param[in] type    Base or OEM
     *
     * @return            Vector of active privileges.  Pointers are valid until
     * the setSinglePrivilege is called, or the Privilege structure is destroyed
     *
     */
    /* @brief 获取给定类型的所有激活权限的名称
     * @param[in] type    Base 或 OEM
     * @return            激活权限组成的 vector。指针在调用
     * setSinglePrivilege 或 Privilege 结构被销毁之前有效 */
    std::vector<std::string> getActivePrivilegeNames(
        const PrivilegeType type) const
    {
        std::vector<std::string> activePrivileges;

        size_t searchIndex = 0;
        size_t endIndex = basePrivilegeCount;
        if (type == PrivilegeType::OEM)
        {
            searchIndex = basePrivilegeCount;
            endIndex = privilegeNames.size();
        }

        for (; searchIndex < endIndex; searchIndex++)
        {
            if (privilegeBitset.test(searchIndex))
            {
                activePrivileges.emplace_back(privilegeNames[searchIndex]);
            }
        }

        return activePrivileges;
    }

    /**
     * @brief Determines if this Privilege set is a superset of the given
     * privilege set
     *
     * @param[in] privilege  Privilege to be checked
     *
     * @return               None
     *
     */
    /* @brief 判断此 Privilege 集合是否为给定权限集合的
     * 超集
     * @param[in] privilege  要检查的权限
     * @return               无 */
    bool isSupersetOf(const Privileges& p) const
    {
        return (privilegeBitset & p.privilegeBitset) == p.privilegeBitset;
    }

    /**
     * @brief Returns the intersection of two Privilege sets.
     *
     * @param[in] privilege  Privilege set to intersect with.
     *
     * @return               The new Privilege set.
     *
     */
    /* @brief 返回两个 Privilege 集合的交集。
     * @param[in] privilege  要求交集的权限集合。
     * @return               新的 Privilege 集合。 */
    Privileges intersection(const Privileges& p) const
    {
        return Privileges{privilegeBitset & p.privilegeBitset};
    }

  private:
    explicit Privileges(const std::bitset<maxPrivilegeCount>& p) :
        privilegeBitset{p}
    {}
    std::bitset<maxPrivilegeCount> privilegeBitset = 0;
};

inline Privileges getUserPrivileges(const persistent_data::UserSession& session)
{
    // default to no access
    // 默认无访问权限
    Privileges privs;

    // Check if user is member of hostconsole group
    // 检查用户是否属于 hostconsole 组
    for (const auto& userGroup : session.userGroups)
    {
        if (userGroup == "hostconsole")
        {
            // Redfish privilege : host console access
            // Redfish 权限：主机控制台访问
            privs.setSinglePrivilege("OpenBMCHostConsole");
            break;
        }
    }

    if (session.userRole == "priv-admin")
    {
        // Redfish privilege : Administrator
        // Redfish 权限：管理员
        privs.setSinglePrivilege("Login");
        privs.setSinglePrivilege("ConfigureManager");
        privs.setSinglePrivilege("ConfigureSelf");
        privs.setSinglePrivilege("ConfigureUsers");
        privs.setSinglePrivilege("ConfigureComponents");
    }
    else if (session.userRole == "priv-operator")
    {
        // Redfish privilege : Operator
        // Redfish 权限：操作员
        privs.setSinglePrivilege("Login");
        privs.setSinglePrivilege("ConfigureSelf");
        privs.setSinglePrivilege("ConfigureComponents");
    }
    else if (session.userRole == "priv-user")
    {
        // Redfish privilege : Readonly
        // Redfish 权限：只读
        privs.setSinglePrivilege("Login");
        privs.setSinglePrivilege("ConfigureSelf");
    }

    return privs;
}

/**
 * @brief The OperationMap represents the privileges required for a
 * single entity (URI).  It maps from the allowable verbs to the
 * privileges required to use that operation.
 *
 * This represents the Redfish "Privilege AND and OR syntax" as given
 * in the spec and shown in the Privilege Registry.  This does not
 * implement any Redfish property overrides, subordinate overrides, or
 * resource URI overrides.  This does not implement the limitation of
 * the ConfigureSelf privilege to operate only on your own account or
 * session.
 **/
/* @brief OperationMap 表示访问单个实体（URI）所需的
 * 权限。它将允许的 verb 映射到使用该操作
 * 所需的权限。
 * 这表示规范中给出并在 Privilege Registry 中展示的
 * Redfish "Privilege AND and OR syntax"。它不实现任何
 * Redfish 属性覆盖、下级覆盖或资源 URI 覆盖。
 * 它也不实现 ConfigureSelf 权限只能操作
 * 自己账户或会话的限制。 */
using OperationMap = boost::container::flat_map<boost::beast::http::verb,
                                                std::vector<Privileges>>;

/* @brief Checks if user is allowed to call an operation
 *
 * @param[in] operationPrivilegesRequired   Privileges required
 * @param[in] userPrivileges                Privileges the user has
 *
 * @return                 True if operation is allowed, false otherwise
 */
/* @brief 检查是否允许用户调用某个操作
 * @param[in] operationPrivilegesRequired   所需的权限
 * @param[in] userPrivileges                用户拥有的权限
 * @return                 允许操作则为 true，否则为 false */
inline bool isOperationAllowedWithPrivileges(
    const std::vector<Privileges>& operationPrivilegesRequired,
    const Privileges& userPrivileges)
{
    // If there are no privileges assigned, there are no privileges required
    // 如果未分配任何权限，则不需要任何权限
    if (operationPrivilegesRequired.empty())
    {
        return true;
    }
    for (const auto& requiredPrivileges : operationPrivilegesRequired)
    {
        BMCWEB_LOG_DEBUG("Checking operation privileges...");
        if (userPrivileges.isSupersetOf(requiredPrivileges))
        {
            BMCWEB_LOG_DEBUG("...success");
            return true;
        }
    }
    return false;
}

/**
 * @brief Checks if given privileges allow to call an HTTP method
 *
 * @param[in] method       HTTP method
 * @param[in] user         Privileges
 *
 * @return                 True if method allowed, false otherwise
 *
 */
/* @brief 检查给定权限是否允许调用某个 HTTP 方法
 * @param[in] method       HTTP 方法
 * @param[in] user         权限
 * @return                 允许该方法则为 true，否则为 false */
inline bool isMethodAllowedWithPrivileges(const boost::beast::http::verb method,
                                          const OperationMap& operationMap,
                                          const Privileges& userPrivileges)
{
    const auto& it = operationMap.find(method);
    if (it == operationMap.end())
    {
        return false;
    }

    return isOperationAllowedWithPrivileges(it->second, userPrivileges);
}

} // namespace redfish
