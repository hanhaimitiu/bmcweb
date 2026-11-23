// SPDX-License-Identifier: Apache-2.0
// SPDX-FileCopyrightText: Copyright OpenBMC Authors
#pragma once

#include "filter_expr_parser_ast.hpp"

#include <boost/spirit/home/x3/char/char.hpp>
#include <boost/spirit/home/x3/char/negated_char_parser.hpp>
#include <boost/spirit/home/x3/directive/lexeme.hpp>
#include <boost/spirit/home/x3/directive/with.hpp>
#include <boost/spirit/home/x3/nonterminal/rule.hpp>
#include <boost/spirit/home/x3/numeric/int.hpp>
#include <boost/spirit/home/x3/numeric/real.hpp>
#include <boost/spirit/home/x3/numeric/real_policies.hpp>
#include <boost/spirit/home/x3/operator/alternative.hpp>
#include <boost/spirit/home/x3/operator/kleene.hpp>
#include <boost/spirit/home/x3/operator/optional.hpp>
#include <boost/spirit/home/x3/operator/plus.hpp>
#include <boost/spirit/home/x3/operator/sequence.hpp>
#include <boost/spirit/home/x3/string/literal_string.hpp>
#include <boost/spirit/home/x3/string/symbols.hpp>

#include <cstddef>

namespace redfish::filter_grammar
{

// Tag for storing recursion depth in the parser context
// 用于在解析器上下文中存储递归深度的标签
struct RecursionDepth
{
    // Maximum recursion depth to prevent stack overflow
    // 防止堆栈溢出的最大递归深度
    constexpr static size_t max = 10;
};

// The below rules very intentionally use the same naming as section 7.3.4 of
// the redfish specification and are declared in the order of the precedence
// that the standard requires.
// 下面的规则有意采用与 redfish 规范 7.3.4 节相同的命名，
// 并按照标准要求的优先级顺序
// 声明。

namespace details
{
using boost::spirit::x3::char_;
using boost::spirit::x3::int64;
using boost::spirit::x3::lexeme;
using boost::spirit::x3::lit;
using boost::spirit::x3::real_parser;
using boost::spirit::x3::rule;
using boost::spirit::x3::strict_real_policies;
using boost::spirit::x3::symbols;

using filter_ast::BooleanOp;
using filter_ast::Comparison;
using filter_ast::ComparisonOpEnum;
using filter_ast::LogicalAnd;
using filter_ast::LogicalNot;
using filter_ast::LogicalOr;
using filter_ast::QuotedString;
using filter_ast::UnquotedString;

// Clang format makes a mess of these rules and makes them hard to read
// clang-format on
// Clang format 会把这些规则弄得一团糟、难以阅读
// clang-format on

// Basic argument types
// 基本参数类型
const rule<class QuotedStringId, QuotedString> quotedString("QuotedString");
const rule<class UnquotedStrId, UnquotedString> unquotedString("UnquotedStr");

// Value comparisons -> boolean (value eq value) (value lt number)
// 值比较 -> 布尔值（value eq value）（value lt number）
const rule<class BooleanOpId, BooleanOp> booleanOp("BooleanOp");
const rule<class ComparisonId, Comparison> comparison("Comparison");

// Logical Comparisons (bool eq bool)
// 逻辑比较 (bool eq bool)
const rule<class LogicalAndId, LogicalAnd> logicalAnd("LogicalAnd");
const rule<class LogicalOrId, LogicalOr> logicalOr("LogicalOr");
const rule<class LogicalNotId, LogicalNot> logicalNot("LogicalNot");

///// BEGIN GRAMMAR
// /// BEGIN GRAMMAR（语法开始）

// Two types of strings.
// 两种字符串类型。
const auto quotedString_def =
    '\'' >> lexeme[*('\\' >> char_ | ~char_('\''))] >> '\'';
const auto unquotedString_def = char_("a-zA-Z") >> *(char_("a-zA-Z0-9[]/"));

// Make sure we only parse true floating points as doubles
// This requires we have a "." which causes 1 to parse as int64, and 1.0 to
// parse as double
// 确保只把真正的浮点数解析为 double
// 这要求必须有 '.'，这样 1 会解析为 int64，而 1.0 会
// 解析为 double
constexpr const real_parser<double, strict_real_policies<double>> strictDouble;

// Argument
// 参数
const auto arg = strictDouble | int64 | unquotedString | quotedString;

// Greater Than/Less Than/Equals
// 大于/小于/等于
const symbols<ComparisonOpEnum> compare{
    {"gt", ComparisonOpEnum::GreaterThan},
    {"ge", ComparisonOpEnum::GreaterThanOrEqual},
    {"lt", ComparisonOpEnum::LessThan},
    {"le", ComparisonOpEnum::LessThanOrEqual},
    {"ne", ComparisonOpEnum::NotEquals},
    {"eq", ComparisonOpEnum::Equals}};

// paren parser that limits the depth of the max recursion.
// 限制最大递归深度的括号解析器。
struct DepthCheckingParser : boost::spirit::x3::parser<DepthCheckingParser>
{
    using attribute_type = LogicalAnd;

    template <typename Iterator, typename Context, typename RContext,
              typename Attribute>
    bool parse(Iterator& first, const Iterator& last, const Context& context,
               RContext& rcontext, Attribute& attr) const
    {
        using boost::spirit::x3::get;
        using boost::spirit::x3::with;

        // Get current recursion depth from context
        // 从上下文获取当前递归深度
        size_t& currentDepth = get<RecursionDepth>(context).get();

        // Check if we've exceeded the maximum depth
        // 检查是否已超过最大深度
        if (currentDepth >= RecursionDepth::max)
        {
            return false;
        }

        // Increment depth for this level
        // 为当前层级增加深度
        ++currentDepth;

        // Parse with the current context (depth is already incremented via
        // reference)
        // 使用当前上下文进行解析（深度已通过引用
        // 递增）
        const auto parens = lit('(') >> logicalAnd >> lit(')');
        bool result = parens.parse(first, last, context, rcontext, attr);

        // Decrement depth after parsing
        // 解析完成后递减深度
        --currentDepth;

        return result;
    }
};

// Parenthesis with depth checking
// 带深度检查的括号
const DepthCheckingParser parens;

// Note, unlike most other comparisons, spaces are required here (one or more)
// to differentiate keys from values (ex Fooeq eq foo)
// 注意，与大多数其他比较不同，这里需要空格（一个或多个）
// 以区分键和值（如 Fooeq eq foo）
const auto comparison_def =
    lexeme[arg >> +lit(' ') >> compare >> +lit(' ') >> arg];

// Logical values
// 逻辑值
const auto booleanOp_def = comparison | parens;

// Not
// Not（非）
const auto logicalNot_def = -(char_('n') >> lit("ot")) >> booleanOp;

// Or
// Or（或）
const auto logicalOr_def = logicalNot >> *(lit("or") >> logicalNot);

// And
// And（与）
const auto logicalAnd_def = logicalOr >> *(lit("and") >> logicalOr);

BOOST_SPIRIT_DEFINE(booleanOp, logicalAnd, logicalNot, logicalOr, quotedString,
                    comparison, unquotedString);
///// END GRAMMAR
// /// END GRAMMAR（语法结束）

// Make the grammar and AST available outside of the system
// 使语法和 AST 在系统外部可用
static constexpr auto& grammar = logicalAnd;
using program = filter_ast::LogicalAnd;

} // namespace details

using details::grammar;
using details::program;

} // namespace redfish::filter_grammar
