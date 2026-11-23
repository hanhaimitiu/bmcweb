// SPDX-License-Identifier: Apache-2.0
// SPDX-FileCopyrightText: Copyright OpenBMC Authors
#include "async_resp.hpp"
#include "http/http2_connection.hpp"
#include "http/http_request.hpp"
#include "http/http_response.hpp"
#include "http_connect_types.hpp"
#include "nghttp2_adapters.hpp"
#include "test_stream.hpp"

#include <nghttp2/nghttp2.h>
#include <sys/types.h>

#include <boost/asio/buffer.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ssl/context.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/write.hpp>
#include <boost/beast/http/field.hpp>

#include <array>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
namespace crow
{

namespace
{

using ::testing::Pair;
using ::testing::UnorderedElementsAre;

struct FakeHandler
{
    bool called = false;
    void handle(const std::shared_ptr<Request>& req,
                const std::shared_ptr<bmcweb::AsyncResp>& asyncResp)
    {
        called = true;
        EXPECT_EQ(req->url().buffer(), "/redfish/v1/");
        EXPECT_EQ(req->methodString(), "GET");
        EXPECT_EQ(req->getHeaderValue(boost::beast::http::field::user_agent),
                  "curl/8.5.0");
        EXPECT_EQ(req->getHeaderValue(boost::beast::http::field::accept),
                  "*/*");
        asyncResp->res.write("StringOutput");
    }
};

std::string getDateStr()
{
    return "TestTime";
}

void unpackHeaders(std::string_view dataField,
                   std::vector<std::pair<std::string, std::string>>& headers)
{
    nghttp2_hd_inflater_ex inflater;

    while (!dataField.empty())
    {
        nghttp2_nv nv;
        int inflateFlags = 0;
        const uint8_t* data = std::bit_cast<const uint8_t*>(dataField.data());
        ssize_t parsed =
            inflater.hd2(&nv, &inflateFlags, data, dataField.size(), 1);

        ASSERT_GT(parsed, 0);
        dataField.remove_prefix(static_cast<size_t>(parsed));
        if ((inflateFlags & NGHTTP2_HD_INFLATE_EMIT) > 0)
        {
            const char* namePtr = std::bit_cast<const char*>(nv.name);
            std::string key(namePtr, nv.namelen);
            const char* valPtr = std::bit_cast<const char*>(nv.value);
            std::string value(valPtr, nv.valuelen);
            headers.emplace_back(key, value);
        }
        if ((inflateFlags & NGHTTP2_HD_INFLATE_FINAL) > 0)
        {
            EXPECT_EQ(inflater.endHeaders(), 0);
            break;
        }
    }
    EXPECT_TRUE(dataField.empty());
}

TEST(http_connection, RequestPropogates)
{
    using namespace std::literals;
    boost::asio::io_context io;
    TestStream stream(io);
    TestStream out(io);
    stream.connect(out);
    // This is a binary pre-encrypted stream captured from curl for a request to
    // curl https://localhost:18080/redfish/v1/
    // 这是从 curl 捕获的二进制预加密流，对应的请求为
    // curl https://localhost:18080/redfish/v1/
    std::string_view toSend =
        // Hello
        // Hello
        "PRI * HTTP/2.0\r\n\r\nSM\r\n\r\n"
        // 18 byte settings frame
        // 18 字节的 settings 帧
        "\x00\x00\x12\x04\x00\x00\x00\x00\x00"
        // Settings
        // Settings
        "\x00\x03\x00\x00\x00\x64\x00\x04\x00\xa0\x00\x00\x00\x02\x00\x00\x00\x00"
        // Window update frame
        // Window update 帧
        "\x00\x00\x04\x08\x00\x00\x00\x00\x00"
        // Window update
        // Window update
        "\x3e\x7f\x00\x01"
        // Header frame END_STREAM, END_HEADERS set
        // Header 帧，设置了 END_STREAM、END_HEADERS 标志
        "\x00\x00\x29\x01\x05\x00\x00\x00"
        // Header payload
        // Header 负载
        "\x01\x82\x87\x41\x8b\xa0\xe4\x1d\x13\x9d\x09\xb8\x17\x80\xf0\x3f"
        "\x04\x89\x62\xc2\xc9\x29\x91\x3b\x1d\xc2\xc7\x7a\x88\x25\xb6\x50"
        "\xc3\xcb\xb6\xb8\x3f\x53\x03\x2a\x2f\x2a"sv;

    boost::asio::write(out, boost::asio::buffer(toSend));

    FakeHandler handler;
    boost::asio::steady_timer timer(io);
    std::function<std::string()> date(getDateStr);
    boost::asio::ssl::context sslCtx(boost::asio::ssl::context::tls_server);
    auto conn = std::make_shared<HTTP2Connection<TestStream, FakeHandler>>(
        boost::asio::ssl::stream<TestStream>(std::move(stream), sslCtx),
        &handler, date, HttpType::HTTP, nullptr, boost::asio::ip::address());
    conn->start();

    std::array<std::string_view, 9> expectedPrefix = {
        // Settings frame size 24
        // Settings 帧大小为 24
        "\x00\x00\x18\x04\x00\x00\x00\x00\x00"sv,
        // 4 max concurrent streams
        // 最大并发流数为 4
        "\x00\x03\x00\x00\x00\x04"sv,
        // Enable push = false
        // Enable push = false
        "\x00\x02\x00\x00\x00\x00"sv,
        // Max window size 1 << 20
        // 最大窗口大小 1 << 20
        "\x00\x04\x00\x10\x00\x00"sv,
        // Max frame size 1 << 14
        // 最大帧大小 1 << 14
        "\x00\x05\x00\x00\x40\x00"sv,

        // Frame window update stream 0
        // 流 0 的窗口更新帧
        "\x00\x00\x04\x08\x00\x00\x00\x00\x00\x00\x0f\x00\x01"sv,

        // Settings ACK from server to client
        // 从服务器到客户端的 Settings ACK
        "\x00\x00\x00\x04\x01\x00\x00\x00\x00"sv,

        // Window update stream 1
        // 流 1 的窗口更新
        "\x00\x00\x04\x08\x00\x00\x00\x00\x01\x00\x07\x00\x01"sv,

        // Start Headers frame stream 1, size 0x005f
        // 开始发送流 1 的 Headers 帧，大小为 0x005f
        "\x00\x00\x5f\x01\x04\x00\x00\x00\x01"sv,
    };

    std::string_view expectedPostfix =
        // Data Frame, Length 12, Stream 1, End Stream flag set
        // Data 帧，长度 12，流 1，设置了 End Stream 标志
        "\x00\x00\x0c\x00\x01\x00\x00\x00\x01"
        // The body expected
        // 预期的响应体
        "StringOutput"sv;

    std::string_view outStr;
    constexpr size_t headerSize = 0x05f;

    size_t expectedPrefixSize = 0;
    for (const auto prefix : expectedPrefix)
    {
        expectedPrefixSize += prefix.size();
    }

    // Run until we receive the expected amount of data
    // 持续运行直到接收到预期数量的数据
    while (outStr.size() <
           expectedPrefixSize + headerSize + expectedPostfix.size())
    {
        io.run_one();
        outStr = out.str();
    }
    EXPECT_TRUE(handler.called);

    // check the stream output against expected
    // 将流输出与预期值进行核对
    for (const auto& prefix : expectedPrefix)
    {
        EXPECT_EQ(outStr.substr(0, prefix.size()), prefix);
        outStr.remove_prefix(prefix.size());
    }

    std::vector<std::pair<std::string, std::string>> headers;
    unpackHeaders(outStr.substr(0, headerSize), headers);
    outStr.remove_prefix(headerSize);

    EXPECT_THAT(headers,
                UnorderedElementsAre(
                    Pair(":status", "200"), Pair("content-length", "12"),
                    Pair("strict-transport-security",
                         "max-age=31536000; includeSubdomains"),
                    Pair("cache-control", "no-store, max-age=0"),
                    Pair("x-content-type-options", "nosniff"),
                    Pair("pragma", "no-cache"), Pair("date", "TestTime")));

    EXPECT_EQ(outStr, expectedPostfix);
}

} // namespace
} // namespace crow
