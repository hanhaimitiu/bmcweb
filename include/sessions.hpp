// SPDX-License-Identifier: Apache-2.0
// SPDX-FileCopyrightText: Copyright OpenBMC Authors
#pragma once

#include "bmcweb_config.h"

#include "logging.hpp"
#include "ossl_random.hpp"
#include "utils/ip_utils.hpp"

#include <boost/asio/ip/address.hpp>
#include <nlohmann/json.hpp>

#include <chrono>
#include <csignal>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace persistent_data
{

// entropy: 20 characters, 62 possibilities.  log2(62^20) = 119 bits of
// entropy.  OWASP recommends at least 64
// https://cheatsheetseries.owasp.org/cheatsheets/Session_Management_Cheat_Sheet.html#session-id-entropy
// 熵：20 个字符，62 种可能。log2(62^20) = 119 比特熵。
// OWASP 建议至少 64 比特。
// https://cheatsheetseries.owasp.org/cheatsheets/Session_Management_Cheat_Sheet.html#session-id-entropy
constexpr std::size_t sessionTokenSize = 20;

enum class SessionType
{
    None,
    Basic,
    Session,
    Cookie,
    MutualTLS
};

struct UserSession
{
    std::string uniqueId;
    std::string sessionToken;
    std::string username;
    std::string csrfToken;
    std::optional<std::string> clientId;
    std::string clientIp;
    std::chrono::time_point<std::chrono::steady_clock> lastUpdated;
    SessionType sessionType{SessionType::None};
    bool cookieAuth = false;
    bool isConfigureSelfOnly = false;
    std::string userRole;
    std::vector<std::string> userGroups;

    // There are two sources of truth for isConfigureSelfOnly:
    //  1. When pamAuthenticateUser() returns PAM_NEW_AUTHTOK_REQD.
    //  2. D-Bus User.Manager.GetUserInfo property UserPasswordExpired.
    // These should be in sync, but the underlying condition can change at any
    // time.  For example, a password can expire or be changed outside of
    // bmcweb.  The value stored here is updated at the start of each
    // operation and used as the truth within bmcweb.
    // isConfigureSelfOnly 有两个信息来源：
    //  1. 当 pamAuthenticateUser() 返回 PAM_NEW_AUTHTOK_REQD 时。
    //  2. D-Bus User.Manager.GetUserInfo 的 UserPasswordExpired 属性。
    // 两者本应保持一致，但底层条件随时可能改变。
    // 例如密码可能在 bmcweb 之外过期或被修改。
    // 此处存储的值在每次操作开始时更新，
    // 并作为 bmcweb 内部使用的判断依据。

    /**
     * @brief Fills object with data from UserSession's JSON representation
     *
     * This replaces nlohmann's from_json to ensure no-throw approach
     *
     * @param[in] j   JSON object from which data should be loaded
     *
     * @return a shared pointer if data has been loaded properly, nullptr
     * otherwise
     */
    /* @brief 用 UserSession 的 JSON 表示中的数据填充对象
     * 这里取代 nlohmann 的 from_json，以确保采用不抛异常的方式
     * @param[in] j   从中加载数据的 JSON 对象
     * @return 数据加载成功时返回 shared pointer，
     * 否则返回 nullptr */
    static std::shared_ptr<UserSession> fromJson(
        const nlohmann::json::object_t& j)
    {
        std::shared_ptr<UserSession> userSession =
            std::make_shared<UserSession>();
        for (const auto& element : j)
        {
            const std::string* thisValue =
                element.second.get_ptr<const std::string*>();
            if (thisValue == nullptr)
            {
                BMCWEB_LOG_ERROR(
                    "Error reading persistent store.  Property {} was not of type string",
                    element.first);
                continue;
            }
            if (element.first == "unique_id")
            {
                userSession->uniqueId = *thisValue;
            }
            else if (element.first == "session_token")
            {
                userSession->sessionToken = *thisValue;
            }
            else if (element.first == "csrf_token")
            {
                userSession->csrfToken = *thisValue;
            }
            else if (element.first == "username")
            {
                userSession->username = *thisValue;
            }
            else if (element.first == "client_id")
            {
                userSession->clientId = *thisValue;
            }
            else if (element.first == "client_ip")
            {
                userSession->clientIp = *thisValue;
            }

            else
            {
                BMCWEB_LOG_ERROR(
                    "Got unexpected property reading persistent file: {}",
                    element.first);
                continue;
            }
        }
        // If any of these fields are missing, we can't restore the session, as
        // we don't have enough information.  These 4 fields have been present
        // in every version of this file in bmcwebs history, so any file, even
        // on upgrade, should have these present
        // 如果这些字段中任何一个缺失，我们就无法恢复会话，
        // 因为信息不足。这 4 个字段在 bmcweb 历史的
        // 每个版本文件中都存在，因此任何文件（即使是在
        // 升级时）都应包含它们。
        if (userSession->uniqueId.empty() || userSession->username.empty() ||
            userSession->sessionToken.empty() || userSession->csrfToken.empty())
        {
            BMCWEB_LOG_DEBUG("Session missing required security "
                             "information, refusing to restore");
            return nullptr;
        }

        // For now, sessions that were persisted through a reboot get their idle
        // timer reset.  This could probably be overcome with a better
        // understanding of wall clock time and steady timer time, possibly
        // persisting values with wall clock time instead of steady timer, but
        // the tradeoffs of all the corner cases involved are non-trivial, so
        // this is done temporarily
        // 目前，通过重启持久化的会话会重置其空闲计时器。
        // 要克服这一点，可能需要更好地理解
        // 挂钟时间与稳定计时器时间，或许可以持久化
        // 挂钟时间而非稳定计时器时间，但其中涉及的
        // 所有边界情况的权衡并非易事，因此
        // 暂时采用这种做法。
        userSession->lastUpdated = std::chrono::steady_clock::now();
        userSession->sessionType = SessionType::Session;

        return userSession;
    }
};

enum class MTLSCommonNameParseMode
{
    Invalid = 0,
    // This section approximately matches Redfish AccountService
    // CertificateMappingAttribute,  plus bmcweb defined OEM ones.
    // Note, IDs in this enum must be maintained between versions, as they are
    // persisted to disk
    // 本节大致对应 Redfish AccountService 的
    // CertificateMappingAttribute，外加 bmcweb 定义的 OEM 项。
    // 注意，此枚举中的 ID 必须在各版本间保持一致，
    // 因为它们会被持久化到磁盘。
    Whole = 1,
    CommonName = 2,
    UserPrincipalName = 3,

    // Intentional gap for future DMTF-defined enums
    // 为未来 DMTF 定义的枚举有意留出的空档

    // OEM parsing modes for various OEMs
    // 各家 OEM 的解析模式
};

inline MTLSCommonNameParseMode getMTLSCommonNameParseMode(std::string_view name)
{
    if (name == "CommonName")
    {
        return MTLSCommonNameParseMode::CommonName;
    }
    if (name == "Whole")
    {
        // Not yet supported
        // return MTLSCommonNameParseMode::Whole;
        // 尚不支持
        // return MTLSCommonNameParseMode::Whole;
    }
    if (name == "UserPrincipalName")
    {
        return MTLSCommonNameParseMode::UserPrincipalName;
    }
    return MTLSCommonNameParseMode::Invalid;
}

struct AuthConfigMethods
{
    // Authentication paths
    // 认证路径
    bool basic = BMCWEB_BASIC_AUTH;
    bool sessionToken = BMCWEB_SESSION_AUTH;
    bool xtoken = BMCWEB_XTOKEN_AUTH;
    bool cookie = BMCWEB_COOKIE_AUTH;
    bool tls = BMCWEB_MUTUAL_TLS_AUTH;

    // Whether or not unauthenticated TLS should be accepted
    // true = reject connections if mutual tls is not provided
    // false = allow connection, and allow user to use other auth method
    // Always default to false, because root certificates will not
    // be provisioned at startup
    // 是否接受未经认证的 TLS：
    // true = 如果未提供双向 TLS 则拒绝连接
    // false = 允许连接，并允许用户使用其他认证方式
    // 始终默认为 false，因为启动时不会预置根
    // 证书。
    bool tlsStrict = false;

    MTLSCommonNameParseMode mTLSCommonNameParsingMode =
        getMTLSCommonNameParseMode(
            BMCWEB_MUTUAL_TLS_COMMON_NAME_PARSING_DEFAULT);

    void fromJson(const nlohmann::json::object_t& j)
    {
        for (const auto& element : j)
        {
            const bool* value = element.second.get_ptr<const bool*>();
            if (value != nullptr)
            {
                if (element.first == "XToken")
                {
                    xtoken = *value;
                }
                else if (element.first == "Cookie")
                {
                    cookie = *value;
                }
                else if (element.first == "SessionToken")
                {
                    sessionToken = *value;
                }
                else if (element.first == "BasicAuth")
                {
                    basic = *value;
                }
                else if (element.first == "TLS")
                {
                    tls = *value;
                }
                else if (element.first == "TLSStrict")
                {
                    tlsStrict = *value;
                }
            }
            const uint64_t* intValue =
                element.second.get_ptr<const uint64_t*>();
            if (intValue != nullptr)
            {
                if (element.first == "MTLSCommonNameParseMode")
                {
                    MTLSCommonNameParseMode tmpMTLSCommonNameParseMode =
                        static_cast<MTLSCommonNameParseMode>(*intValue);
                    if (tmpMTLSCommonNameParseMode <=
                        MTLSCommonNameParseMode::UserPrincipalName)
                    {
                        mTLSCommonNameParsingMode = tmpMTLSCommonNameParseMode;
                    }
                    else
                    {
                        BMCWEB_LOG_WARNING(
                            "Json value of {} was out of range of the enum.  Ignoring",
                            *intValue);
                    }
                }
            }
        }
    }
};

class SessionStore
{
  public:
    std::shared_ptr<UserSession> generateUserSession(
        std::string_view username, const boost::asio::ip::address& clientIp,
        const std::optional<std::string>& clientId, SessionType sessionType,
        bool isConfigureSelfOnly = false)
    {
        // Only need csrf tokens for cookie based auth, token doesn't matter
        // 只有基于 cookie 的认证才需要 csrf 令牌，令牌本身无关紧要
        std::string sessionToken =
            bmcweb::getRandomIdOfLength(sessionTokenSize);
        std::string csrfToken = bmcweb::getRandomIdOfLength(sessionTokenSize);
        std::string uniqueId = bmcweb::getRandomIdOfLength(10);

        //
        if (sessionToken.empty() || csrfToken.empty() || uniqueId.empty())
        {
            BMCWEB_LOG_ERROR("Failed to generate session tokens");
            return nullptr;
        }

        auto session = std::make_shared<UserSession>(UserSession{
            uniqueId,
            sessionToken,
            std::string(username),
            csrfToken,
            clientId,
            redfish::ip_util::toString(clientIp),
            std::chrono::steady_clock::now(),
            sessionType,
            false,
            isConfigureSelfOnly,
            "",
            {}});
        auto it = authTokens.emplace(sessionToken, session);
        // Only need to write to disk if session isn't about to be destroyed.
        // 只有当会话不会立即销毁时才需要写入磁盘。
        needWrite = sessionType != SessionType::Basic &&
                    sessionType != SessionType::MutualTLS;
        return it.first->second;
    }

    std::shared_ptr<UserSession> loginSessionByToken(std::string_view token)
    {
        applySessionTimeouts();
        if (token.size() != sessionTokenSize)
        {
            return nullptr;
        }
        auto sessionIt = authTokens.find(std::string(token));
        if (sessionIt == authTokens.end())
        {
            return nullptr;
        }
        std::shared_ptr<UserSession> userSession = sessionIt->second;
        userSession->lastUpdated = std::chrono::steady_clock::now();
        return userSession;
    }

    std::shared_ptr<UserSession> getSessionByUid(std::string_view uid)
    {
        applySessionTimeouts();
        // TODO(Ed) this is inefficient
        // TODO(Ed) 这样效率不高
        auto sessionIt = authTokens.begin();
        while (sessionIt != authTokens.end())
        {
            if (sessionIt->second->uniqueId == uid)
            {
                return sessionIt->second;
            }
            sessionIt++;
        }
        return nullptr;
    }

    void removeSession(const std::shared_ptr<UserSession>& session)
    {
        authTokens.erase(session->sessionToken);
        needWrite = true;
    }

    std::vector<std::string> getAllUniqueIds()
    {
        applySessionTimeouts();
        std::vector<std::string> ret;
        ret.reserve(authTokens.size());
        for (auto& session : authTokens)
        {
            ret.push_back(session.second->uniqueId);
        }
        return ret;
    }

    std::vector<std::string> getUniqueIdsBySessionType(SessionType type)
    {
        applySessionTimeouts();

        std::vector<std::string> ret;
        ret.reserve(authTokens.size());
        for (auto& session : authTokens)
        {
            if (type == session.second->sessionType)
            {
                ret.push_back(session.second->uniqueId);
            }
        }
        return ret;
    }

    std::vector<std::shared_ptr<UserSession>> getSessions()
    {
        std::vector<std::shared_ptr<UserSession>> sessions;
        sessions.reserve(authTokens.size());
        for (auto& session : authTokens)
        {
            sessions.push_back(session.second);
        }
        return sessions;
    }

    void removeSessionsByUsername(std::string_view username)
    {
        std::erase_if(authTokens, [username](const auto& value) {
            if (value.second == nullptr)
            {
                return false;
            }
            return value.second->username == username;
        });
    }

    void removeSessionsByUsernameExceptSession(
        std::string_view username, const std::shared_ptr<UserSession>& session)
    {
        std::erase_if(authTokens, [username, session](const auto& value) {
            if (value.second == nullptr)
            {
                return false;
            }

            return value.second->username == username &&
                   value.second->uniqueId != session->uniqueId;
        });
    }

    void updateAuthMethodsConfig(const AuthConfigMethods& config)
    {
        bool isTLSchanged = (authMethodsConfig.tls != config.tls);
        authMethodsConfig = config;
        needWrite = true;
        if (isTLSchanged)
        {
            // recreate socket connections with new settings
            // NOLINTNEXTLINE(misc-include-cleaner)
            // 用新设置重建 socket 连接
            // NOLINTNEXTLINE(misc-include-cleaner)
            std::raise(SIGHUP);
        }
    }

    AuthConfigMethods& getAuthMethodsConfig()
    {
        return authMethodsConfig;
    }

    bool needsWrite() const
    {
        return needWrite;
    }
    int64_t getTimeoutInSeconds() const
    {
        return std::chrono::seconds(timeoutInSeconds).count();
    }

    void updateSessionTimeout(std::chrono::seconds newTimeoutInSeconds)
    {
        timeoutInSeconds = newTimeoutInSeconds;
        needWrite = true;
    }

    static SessionStore& getInstance()
    {
        static SessionStore sessionStore;
        return sessionStore;
    }

    void applySessionTimeouts()
    {
        auto timeNow = std::chrono::steady_clock::now();
        if (timeNow - lastTimeoutUpdate > std::chrono::seconds(1))
        {
            lastTimeoutUpdate = timeNow;
            auto authTokensIt = authTokens.begin();
            while (authTokensIt != authTokens.end())
            {
                if (timeNow - authTokensIt->second->lastUpdated >=
                    timeoutInSeconds)
                {
                    authTokensIt = authTokens.erase(authTokensIt);

                    needWrite = true;
                }
                else
                {
                    authTokensIt++;
                }
            }
        }
    }

    SessionStore(const SessionStore&) = delete;
    SessionStore& operator=(const SessionStore&) = delete;
    SessionStore(SessionStore&&) = delete;
    SessionStore& operator=(const SessionStore&&) = delete;
    ~SessionStore() = default;

    std::unordered_map<std::string, std::shared_ptr<UserSession>,
                       std::hash<std::string>, bmcweb::ConstantTimeCompare>
        authTokens;

    std::chrono::time_point<std::chrono::steady_clock> lastTimeoutUpdate;
    bool needWrite{false};
    std::chrono::seconds timeoutInSeconds;
    AuthConfigMethods authMethodsConfig;

  private:
    SessionStore() : timeoutInSeconds(1800) {}
};

} // namespace persistent_data
