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
 * 这是一个自动生成的头文件，其中包含
 * Redfish DMTF 定义消息的相关定义。
 * 除运行 parse_registries.py 脚本外，
 * 请勿修改此注册表。本文件中所含的
 * 定义归 DMTF 所有。对这些文件的任何修改，
 * 都应先推送到 DMTF github 组织中的
 * 相应注册表。 */
// These generated headers are a superset of what is needed.
// clang sees them as an error, so ignore
// NOLINTBEGIN(misc-include-cleaner)
// 这些生成的头文件包含了超出所需的超集内容。
// clang 会将它们视为错误，因此忽略。
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
nlohmann::json::object_t targetDetermined(std::string_view arg1,
                                          std::string_view arg2);

nlohmann::json::object_t allTargetsDetermined();

nlohmann::json::object_t noTargetsDetermined(std::string_view arg1);

nlohmann::json::object_t updateInProgress();

nlohmann::json::object_t transferringToComponent(std::string_view arg1,
                                                 std::string_view arg2);

nlohmann::json::object_t verifyingAtComponent(std::string_view arg1,
                                              std::string_view arg2);

nlohmann::json::object_t installingOnComponent(std::string_view arg1,
                                               std::string_view arg2);

nlohmann::json::object_t applyingOnComponent(std::string_view arg1,
                                             std::string_view arg2);

nlohmann::json::object_t transferFailed(std::string_view arg1,
                                        std::string_view arg2);

nlohmann::json::object_t verificationFailed(std::string_view arg1,
                                            std::string_view arg2);

nlohmann::json::object_t applyFailed(std::string_view arg1,
                                     std::string_view arg2);

nlohmann::json::object_t activateFailed(std::string_view arg1,
                                        std::string_view arg2);

nlohmann::json::object_t awaitToUpdate(std::string_view arg1,
                                       std::string_view arg2);

nlohmann::json::object_t awaitToActivate(std::string_view arg1,
                                         std::string_view arg2);

nlohmann::json::object_t updateSuccessful(std::string_view arg1,
                                          std::string_view arg2);

nlohmann::json::object_t operationTransitionedToJob(std::string_view arg1);

nlohmann::json::object_t updateSkipped(std::string_view arg1,
                                       std::string_view arg2);

nlohmann::json::object_t updateSkippedSameVersion(std::string_view arg1,
                                                  std::string_view arg2);

nlohmann::json::object_t updateNotApplicable(std::string_view arg1,
                                             std::string_view arg2);

} // namespace messages
} // namespace redfish
