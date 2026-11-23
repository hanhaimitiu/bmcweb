// SPDX-License-Identifier: Apache-2.0
// SPDX-FileCopyrightText: Copyright OpenBMC Authors
#pragma once

#include <charconv>
#include <iterator>
#include <string_view>

namespace details
{

// This implementation avoids the complexity of using std::isdigit, which pulls
// in all of <locale>, and likely has other consequences.
// 此实现避免了使用 std::isdigit 的复杂性，后者会引入
// 整个 <locale>，并且可能带来其他影响。
inline bool simpleIsDigit(const char c)
{
    return c >= '0' && c <= '9';
}

enum class ModeType
{
    STRING,
    NUMBER
};

} // namespace details

inline int alphanumComp(std::string_view left, std::string_view right)
{
    std::string_view::const_iterator l = left.cbegin();
    std::string_view::const_iterator r = right.cbegin();

    details::ModeType mode = details::ModeType::STRING;

    while (l != left.end() && r != right.end())
    {
        if (mode == details::ModeType::STRING)
        {
            // check if this are digit characters
            // 检查这些是否为数字字符
            const bool lDigit = details::simpleIsDigit(*l);
            const bool rDigit = details::simpleIsDigit(*r);
            // if both characters are digits, we continue in NUMBER mode
            // 如果两个字符都是数字，则继续以 NUMBER 模式处理
            if (lDigit && rDigit)
            {
                mode = details::ModeType::NUMBER;
                continue;
            }
            // if only the left character is a digit, we have a result
            // 如果只有左侧字符是数字，则已经得到比较结果
            if (lDigit)
            {
                return -1;
            } // if only the right character is a digit, we have a result
            if (rDigit)
            {
                return +1;
            }
            // compute the difference of both characters
            // 计算两个字符的差值
            const int diff = *l - *r;
            // if they differ we have a result
            // 如果它们不同则已得到比较结果
            if (diff != 0)
            {
                return diff;
            }
            // otherwise process the next characters
            // 否则继续处理下一个字符
            std::advance(l, 1);
            std::advance(r, 1);
        }
        else // mode==NUMBER
        {
            // get the left number
            // 获取左侧的数字
            int lInt = 0;
            auto fc = std::from_chars(&(*l), &(*left.end()), lInt);
            std::advance(l, std::distance(l, fc.ptr));

            // get the right number
            // 获取右侧的数字
            int rInt = 0;
            fc = std::from_chars(&(*r), &(*right.end()), rInt);
            std::advance(r, std::distance(r, fc.ptr));

            // if the difference is not equal to zero, we have a comparison
            // result
            // 如果差值不为零，我们就得到了比较
            // 结果
            const int diff = lInt - rInt;
            if (diff != 0)
            {
                return diff;
            }

            // otherwise we process the next substring in STRING mode
            // 否则以 STRING 模式处理下一个子串
            mode = details::ModeType::STRING;
        }
    }
    if (r == right.end() && l == left.end())
    {
        return 0;
    }
    if (r == right.end())
    {
        return 1;
    }
    return -1;
}

// A generic template type compatible with std::less that can be used on generic
// containers (set, map, etc)
// 一个与 std::less 兼容的通用模板类型，可用于通用
// 容器（set、map 等）
template <class Type>
struct AlphanumLess
{
    bool operator()(const Type& left, const Type& right) const
    {
        return alphanumComp(left, right) < 0;
    }
};
