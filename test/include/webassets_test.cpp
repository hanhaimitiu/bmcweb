// SPDX-License-Identifier: Apache-2.0
// SPDX-FileCopyrightText: Copyright OpenBMC Authors
#include "webassets.hpp"

#include <gtest/gtest.h>

namespace crow::webassets
{
namespace
{

TEST(GetStaticEtagTest, CorrectValues)
{
    // Webpack-style hash in filename.
    // 文件名中 Webpack 风格的哈希。
    EXPECT_EQ(getStaticEtag("app.63e2c453.css"), "\"63e2c453\"");

    // Vite-style hash in filename.
    // 文件名中 Vite 风格的哈希。
    EXPECT_EQ(getStaticEtag("app.DhhjLIym.js"), "\"DhhjLIym\"");

    // Hash extraction still works when file has multiple dots.
    // 文件名含多个点号时仍能正确提取哈希。
    EXPECT_EQ(getStaticEtag("vendor.app.DhhjLIym.js"), "\"DhhjLIym\"");

    // Path prefixes do not affect hash extraction.
    // 路径前缀不影响哈希提取。
    EXPECT_EQ(getStaticEtag("/usr/share/www/app.63e2c453.css"), "\"63e2c453\"");
    EXPECT_EQ(getStaticEtag("./public/assets/app.DhhjLIym.js"), "\"DhhjLIym\"");

    // Too few segments to contain name/hash/extension.
    // 片段太少，无法包含名称/哈希/扩展名。
    EXPECT_EQ(getStaticEtag("nested/dir/vendor.app.DhhjLIym.js"),
              "\"DhhjLIym\"");
    EXPECT_EQ(getStaticEtag("app.js"), "");
    EXPECT_EQ(getStaticEtag("/usr/share/www/app.js"), "");

    // Hash must be exactly 8 characters.
    // 哈希必须恰好为 8 个字符。
    EXPECT_EQ(getStaticEtag("app.63e2c45.css"), "");
    EXPECT_EQ(getStaticEtag("app.63e2c4537.css"), "");

    // Empty string and no-dot filenames yield no hash.
    // 空字符串和无点号的文件名不会产生哈希。
    EXPECT_EQ(getStaticEtag(""), "");
    EXPECT_EQ(getStaticEtag("appjs"), "");

    // Mixed-case hex hash still matches under alphanumeric rule.
    // 大小写混合的十六进制哈希在字母数字规则下仍能匹配。
    EXPECT_EQ(getStaticEtag("app.63E2C453.css"), "\"63E2C453\"");

    // Hash must be alphanumeric only.
    // 哈希只能包含字母和数字。
    EXPECT_EQ(getStaticEtag("app.63e2c45-.css"), "");
    EXPECT_EQ(getStaticEtag("app.63e2c45_.css"), "");
}

} // namespace
} // namespace crow::webassets
