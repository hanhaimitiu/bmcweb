// SPDX-License-Identifier: Apache-2.0
// SPDX-FileCopyrightText: Copyright OpenBMC Authors
#include "mutual_tls.hpp"
#include "mutual_tls_private.hpp"
#include "ossl_test_memory.hpp"
#include "ossl_wrappers.hpp"
#include "sessions.hpp"

#include <chrono>
#include <initializer_list>
#include <optional>
#include <string>
#include <string_view>

extern "C"
{
#include <openssl/obj_mac.h>
#include <openssl/types.h>
}

#include <boost/asio/ip/address.hpp>

#include <memory>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

using ::testing::IsNull;
using ::testing::NotNull;

static const OpenSSLTestMemory osslInit;

namespace
{
// Pump data between the two SSL objects via their memory BIOs
// until both sides report completion (or error).
// 通过两个 SSL 对象的内存 BIO 在它们之间泵送数据，
// 直到双方都报告完成（或出错）。
void pumpHandshake(OpenSSLSSL& client, OpenSSLSSL& server)
{
    for (int i = 0; i < 100; ++i)
    {
        int clientRet = client.doHandshake();
        client.transferTo(server);

        int serverRet = server.doHandshake();
        server.transferTo(client);

        if (clientRet == 1 && serverRet == 1)
        {
            return;
        }
    }
}

// Performs a TLS handshake over memory BIOs so the server-side SSL object
// sees the client certificate as its peer certificate.
// clientCert: the X509 certificate the client presents to the server.
//             If nullptr, the client will not present a certificate.
// outServerSsl: receives the server-side SSL handle on success.
// Must be called from a TEST body so ASSERT_* can abort the test.
// 通过内存 BIO 执行 TLS 握手，使服务器端的 SSL 对象
// 将客户端证书视为其对等证书。
// clientCert: 客户端提供给服务器的 X509 证书。
//             如果为 nullptr，则客户端不提供证书。
// outServerSsl: 成功时接收服务器端的 SSL 句柄。
// 必须从 TEST 主体中调用，以便 ASSERT_* 能够中止测试。
void mtlsHandshake(OpenSSLX509* clientCert,
                   std::optional<OpenSSLSSL>& outServerSsl)
{
    OpenSSLSSLCtx serverCtx = OpenSSLSSLCtx::createServerCtx();
    ASSERT_TRUE(serverCtx.valid());
    OpenSSLSSLCtx clientCtx = OpenSSLSSLCtx::createClientCtx();
    ASSERT_TRUE(clientCtx.valid());

    // ---- Generate a self-signed server certificate + key ----
    // ---- 生成自签名的服务器证书 + 密钥 ----
    std::optional<OpenSSLEVPKey> serverKey = OpenSSLEVPKeyCTX::createPKey();
    ASSERT_TRUE(serverKey);
    if (!serverKey)
    {
        return;
    }

    OpenSSLX509 serverCert;
    serverCert.setVersion(2);
    serverCert.setSerialNumber(1);
    serverCert.setValidityPeriodFromNow(std::chrono::seconds(60 * 60));
    ASSERT_TRUE(serverCert.setPubkey(*serverKey));
    serverCert.setSubjectName("server");
    ASSERT_TRUE(serverCert.setIssuerNameToSubject());
    ASSERT_TRUE(serverCert.sign(*serverKey));

    ASSERT_TRUE(serverCtx.useCertificate(serverCert));
    ASSERT_TRUE(serverCtx.usePrivateKey(*serverKey));
    // Ask the client for a certificate but don't verify it ourselves
    // (we let verifyMtlsUser handle the verification logic)
    // 向客户端请求证书，但我们自己不验证它
    // （由 verifyMtlsUser 处理验证逻辑）
    serverCtx.setVerifyPeer();

    // Trust the server certificate
    // 信任服务器证书
    ASSERT_TRUE(clientCtx.addCertToStore(serverCert));
    clientCtx.setVerifyPeer();

    // ---- Prepare client certificate with a matching private key ----
    // ---- 准备带有匹配私钥的客户端证书 ----
    std::optional<OpenSSLEVPKey> clientKey;
    if (clientCert != nullptr)
    {
        // Set required fields for TLS verification to succeed
        // 设置 TLS 验证成功所需的字段
        ASSERT_TRUE(clientCert->setIssuerNameToSubject());
        clientCert->setSerialNumber(2);
        clientCert->setValidityPeriodFromNow(std::chrono::seconds(60 * 60));

        // Generate a new key and re-sign the cert so we have both
        // the certificate and its private key for the handshake.
        // 生成新密钥并重新签署证书，以便我们同时拥有证书
        // 及其私钥用于握手。
        clientKey = OpenSSLEVPKeyCTX::createPKey();
        ASSERT_TRUE(clientKey);
        if (!clientKey)
        {
            return;
        }
        ASSERT_TRUE(clientCert->setPubkey(*clientKey));
        ASSERT_TRUE(clientCert->sign(*clientKey));

        // Trust the (re-signed) client cert on the server side
        // 在服务器端信任（重新签署后的）客户端证书
        ASSERT_TRUE(serverCtx.addCertToStore(*clientCert));
    }

    // ---- Create SSL objects with memory BIOs ----
    // ---- 使用内存 BIO 创建 SSL 对象 ----
    outServerSsl.emplace(serverCtx);
    ASSERT_TRUE(outServerSsl->valid());
    ASSERT_TRUE(outServerSsl->setMemBio());
    outServerSsl->setAcceptState();

    OpenSSLSSL clientSsl(clientCtx);
    ASSERT_TRUE(clientSsl.valid());
    ASSERT_TRUE(clientSsl.setMemBio());
    clientSsl.setConnectState();

    if (clientCert != nullptr)
    {
        ASSERT_TRUE(clientSsl.useCertificate(*clientCert));
        ASSERT_TRUE(clientKey);
        if (!clientKey)
        {
            return;
        }
        ASSERT_TRUE(clientSsl.usePrivateKey(*clientKey));
    }

    // ---- Perform the handshake ----
    // ---- 执行握手 ----
    pumpHandshake(clientSsl, *outServerSsl);
}

void verifyMtlsCert(std::string_view keyUsage)
{
    OpenSSLX509 x509;
    x509.setSubjectName("user");

    x509.addExt(NID_key_usage, keyUsage);
    x509.addExt(NID_ext_key_usage, "clientAuth");

    std::optional<OpenSSLSSL> serverSsl;
    mtlsHandshake(&x509, serverSsl);

    ASSERT_TRUE(serverSsl);
    if (!serverSsl)
    {
        return;
    }

    boost::asio::ip::address ip;
    std::shared_ptr<persistent_data::UserSession> session =
        verifyMtlsUser(ip, *serverSsl);
    ASSERT_THAT(session, NotNull());
    EXPECT_THAT(session->username, "user");
}

TEST(MutualTLS, GoodCert)
{
    verifyMtlsCert("digitalSignature, keyAgreement");
}

TEST(MutualTLS, MissingKeyUsage)
{
    for (const char* usageString :
         {"digitalSignature", "keyAgreement", "digitalSignature, keyAgreement"})
    {
        verifyMtlsCert(usageString);
    }
}

TEST(MutualTLS, MissingCert)
{
    std::optional<OpenSSLSSL> serverSsl;
    mtlsHandshake(nullptr, serverSsl);
    ASSERT_TRUE(serverSsl);
    if (!serverSsl)
    {
        return;
    }
    boost::asio::ip::address ip;
    std::shared_ptr<persistent_data::UserSession> session =
        verifyMtlsUser(ip, *serverSsl);
    ASSERT_THAT(session, IsNull());
}

TEST(GetCommonNameFromCert, EmptyCommonName)
{
    OpenSSLX509 x509;
    std::string commonName = x509.getCommonName();
    EXPECT_THAT(commonName, "");
}

TEST(GetCommonNameFromCert, ValidCommonName)
{
    OpenSSLX509 x509;
    x509.setSubjectName("user");
    std::string commonName = x509.getCommonName();
    EXPECT_THAT(commonName, "user");
}

TEST(GetUPNFromCert, EmptySubjectAlternativeName)
{
    OpenSSLX509 x509;
    std::string upn = getUPNFromCert(x509, "");
    EXPECT_THAT(upn, "");
}

TEST(GetUPNFromCert, NonOthernameSubjectAlternativeName)
{
    OpenSSLX509 x509;
    ASSERT_TRUE(x509.addAltNameEmails({"user@domain.com"}));
    std::string upn = getUPNFromCert(x509, "hostname.domain.com");
    EXPECT_THAT(upn, "");
}

TEST(GetUPNFromCert, NonUPNSubjectAlternativeName)
{
    OpenSSLX509 x509;
    ASSERT_TRUE(x509.addAltNames(NID_SRVName, {"user@domain.com"}));
    std::string upn = getUPNFromCert(x509, "hostname.domain.com");
    EXPECT_THAT(upn, "");
}

TEST(GetUPNFromCert, ValidUPN)
{
    OpenSSLX509 x509;
    ASSERT_TRUE(x509.addAltNameUpns({"user@domain.com"}));

    std::string upn = getUPNFromCert(x509, "hostname.domain.com");
    EXPECT_THAT(upn, "user");
}

TEST(IsUPNMatch, MultipleCases)
{
    EXPECT_FALSE(isUPNMatch("user", "hostname.domain.com"));
    EXPECT_TRUE(isUPNMatch("user@domain.com", "hostname.domain.com"));
    EXPECT_FALSE(isUPNMatch("user@domain.com", "hostname.domain.org"));
    EXPECT_FALSE(isUPNMatch("user@region.com", "hostname.domain.com"));
    EXPECT_FALSE(isUPNMatch("user@com", "hostname.region.domain.com"));
    EXPECT_TRUE(isUPNMatch("user@hostname", "hostname"));
    EXPECT_FALSE(isUPNMatch("user@domain.com", ""));
    EXPECT_FALSE(isUPNMatch("user@", ""));
}

TEST(IsUPNMatch, CaseSensitivity)
{
    // Domain names are case-insensitive per RFC standards
    // 根据 RFC 标准，域名不区分大小写
    EXPECT_TRUE(isUPNMatch("user@DOMAIN.COM", "host.domain.com"));
    EXPECT_TRUE(isUPNMatch("user@Domain.Com", "host.DOMAIN.COM"));
    EXPECT_TRUE(isUPNMatch("user@domain.com", "host.DOMAIN.COM"));
}

TEST(GetUPNFromCert, DomainMismatchRejects)
{
    OpenSSLX509 x509;
    x509.addAltNameUpns({"user@evil.com"});

    std::string upn = getUPNFromCert(x509, "hostname.domain.com");
    EXPECT_THAT(upn, "");
}

TEST(GetUPNFromCert, MultipleUPNEntriesFirstWins)
{
    OpenSSLX509 x509;
    ASSERT_TRUE(x509.addAltNameUpns({"first@domain.com", "second@domain.com"}));

    std::string upn = getUPNFromCert(x509, "hostname.domain.com");
    EXPECT_THAT(upn, "first");
}

TEST(GetUPNFromCert, UPNWithoutAtSymbol)
{
    OpenSSLX509 x509;
    x509.addAltNameUpns({"userwithoutatsymbol"});

    std::string upn = getUPNFromCert(x509, "hostname.domain.com");
    EXPECT_THAT(upn, "");
}

TEST(GetUPNFromCert, UPNWithEmptyUsername)
{
    OpenSSLX509 x509;
    x509.addAltNameUpns({"@domain.com"});

    std::string upn = getUPNFromCert(x509, "hostname.domain.com");
    EXPECT_THAT(upn, "");
}

TEST(MutualTLS, CertificateWithoutClientAuthPurpose)
{
    OpenSSLX509 x509;
    x509.setSubjectName("user");

    x509.addExt(NID_key_usage, "digitalSignature");
    x509.addExt(NID_ext_key_usage, "serverAuth");

    std::optional<OpenSSLSSL> serverSsl;
    mtlsHandshake(&x509, serverSsl);
    ASSERT_TRUE(serverSsl);
    if (!serverSsl)
    {
        return;
    }

    boost::asio::ip::address ip;
    std::shared_ptr<persistent_data::UserSession> session =
        verifyMtlsUser(ip, *serverSsl);
    ASSERT_THAT(session, IsNull());
}

TEST(MutualTLS, UPNValidCertSuccess)
{
    persistent_data::SessionStore::getInstance().getAuthMethodsConfig().tls =
        true;
    persistent_data::SessionStore::getInstance()
        .getAuthMethodsConfig()
        .mTLSCommonNameParsingMode =
        persistent_data::MTLSCommonNameParseMode::CommonName;

    OpenSSLX509 x509;
    x509.setSubjectName("user");

    x509.addExt(NID_key_usage, "digitalSignature, keyAgreement");
    x509.addExt(NID_ext_key_usage, "clientAuth");

    std::optional<OpenSSLSSL> serverSsl;
    mtlsHandshake(&x509, serverSsl);
    ASSERT_TRUE(serverSsl);
    if (!serverSsl)
    {
        return;
    }

    boost::asio::ip::address ip;
    std::shared_ptr<persistent_data::UserSession> session =
        verifyMtlsUser(ip, *serverSsl);
    ASSERT_THAT(session, NotNull());
    EXPECT_THAT(session->username, "user");
    EXPECT_THAT(session->sessionType, persistent_data::SessionType::MutualTLS);
}

TEST(MutualTLS, UPNDomainMismatchRejectsAuth)
{
    persistent_data::SessionStore::getInstance().getAuthMethodsConfig().tls =
        true;
    persistent_data::SessionStore::getInstance()
        .getAuthMethodsConfig()
        .mTLSCommonNameParsingMode =
        persistent_data::MTLSCommonNameParseMode::UserPrincipalName;

    OpenSSLX509 x509;
    x509.addAltNameUpns({"attacker@evil.com"});

    x509.addExt(NID_key_usage, "digitalSignature, keyAgreement");
    x509.addExt(NID_ext_key_usage, "clientAuth");

    std::optional<OpenSSLSSL> serverSsl;
    mtlsHandshake(&x509, serverSsl);
    ASSERT_TRUE(serverSsl);
    if (!serverSsl)
    {
        return;
    }

    boost::asio::ip::address ip;
    std::shared_ptr<persistent_data::UserSession> session =
        verifyMtlsUser(ip, *serverSsl);
    ASSERT_THAT(session, IsNull());
}

TEST(MutualTLS, CommonNameModeReturnsCommonName)
{
    persistent_data::SessionStore::getInstance().getAuthMethodsConfig().tls =
        true;
    persistent_data::SessionStore::getInstance()
        .getAuthMethodsConfig()
        .mTLSCommonNameParsingMode =
        persistent_data::MTLSCommonNameParseMode::CommonName;

    OpenSSLX509 x509;
    x509.setSubjectName("user");
    x509.addAltNameUpns({"upnuser@domain.com"});
    x509.addExt(NID_key_usage, "digitalSignature, keyAgreement");
    x509.addExt(NID_ext_key_usage, "clientAuth");

    std::optional<OpenSSLSSL> serverSsl;
    mtlsHandshake(&x509, serverSsl);
    ASSERT_TRUE(serverSsl);
    if (!serverSsl)
    {
        return;
    }

    boost::asio::ip::address ip;
    std::shared_ptr<persistent_data::UserSession> session =
        verifyMtlsUser(ip, *serverSsl);
    ASSERT_THAT(session, NotNull());
    EXPECT_THAT(session->username, "user");
}

TEST(MutualTLS, InvalidModeRejectsAuth)
{
    persistent_data::SessionStore::getInstance().getAuthMethodsConfig().tls =
        true;
    persistent_data::SessionStore::getInstance()
        .getAuthMethodsConfig()
        .mTLSCommonNameParsingMode =
        persistent_data::MTLSCommonNameParseMode::Invalid;

    OpenSSLX509 x509;
    x509.setSubjectName("user");
    x509.addExt(NID_key_usage, "digitalSignature, keyAgreement");
    x509.addExt(NID_ext_key_usage, "clientAuth");

    std::optional<OpenSSLSSL> serverSsl;
    mtlsHandshake(&x509, serverSsl);
    ASSERT_TRUE(serverSsl);
    if (!serverSsl)
    {
        return;
    }

    boost::asio::ip::address ip;
    std::shared_ptr<persistent_data::UserSession> session =
        verifyMtlsUser(ip, *serverSsl);
    ASSERT_THAT(session, IsNull());
}

TEST(GetUPNFromCert, UPNWithMultipleAtSymbols)
{
    OpenSSLX509 x509;
    x509.addAltNameUpns({"user@sub@domain.com"});

    std::string upn = getUPNFromCert(x509, "hostname.domain.com");
    EXPECT_THAT(upn, "");
}

TEST(GetUPNFromCert, UPNWithEmptyDomain)
{
    OpenSSLX509 x509;
    x509.addAltNameUpns({"user@"});

    std::string upn = getUPNFromCert(x509, "hostname.domain.com");
    EXPECT_THAT(upn, "");
}

TEST(IsUPNMatch, InternationalizedDomain)
{
    EXPECT_TRUE(isUPNMatch("user@münchen.de", "host.münchen.de"));
    EXPECT_FALSE(isUPNMatch("user@münchen.de", "host.berlin.de"));
}

TEST(GetUPNFromCert, UPNWithSpecialCharacters)
{
    OpenSSLX509 x509;
    x509.addAltNameUpns({"user.name+tag@domain.com"});

    std::string upn = getUPNFromCert(x509, "hostname.domain.com");
    EXPECT_THAT(upn, "user.name+tag");
}

TEST(GetUPNFromCert, FirstUPNInvalidSecondValid)
{
    OpenSSLX509 x509;
    x509.addAltNameUpns({"first@wrong.com", "second@domain.com"});

    std::string upn = getUPNFromCert(x509, "hostname.domain.com");
    EXPECT_THAT(upn, "second");
}
} // namespace
