#pragma once

#include <string>
#include <string_view>
#include <unordered_map>

#include "kv/store.hpp"

namespace kv {

[[nodiscard]] Status saveCommand(const std::string& file_path,
                                 char command,
                                 std::string_view key,
                                 std::string_view value = {});

[[nodiscard]] Status rebuildHashTree(
    const std::string& file_path,
    std::unordered_map<std::string, std::string>& entries);

} // namespace kv
