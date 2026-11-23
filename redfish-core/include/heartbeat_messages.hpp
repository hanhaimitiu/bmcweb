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
/*                 请先阅读此警告
 * 这是一个自动生成的头文件，其中包含针对
 * Redfish DMTF 定义消息的定义。
 * 请勿在运行 parse_registries.py 脚本之外
 * 修改此注册表。本文件中包含的定义
 * 归 DMTF 所有。对这些文件的任何修改
 * 都应首先提交到 DMTF
 * github 组织中的相应注册表。 */
// These generated headers are a superset of what is needed.
// clang sees them as an error, so ignore
// NOLINTBEGIN(misc-include-cleaner)
// 这些生成的头文件是所需内容的超集。
// clang 会将其视为错误，因此忽略
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
nlohmann::json::object_t redfishServiceFunctional();

}
} // namespace redfish
