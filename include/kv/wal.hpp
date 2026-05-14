#pragma once

#include <string>
#include <string_view>

#include "kv/entry_map.hpp"
#include "kv/store.hpp"

namespace kv
{

    [[nodiscard]] Status saveCommand(const std::string &file_path,
                                     char command,
                                     std::string_view key,
                                     std::string_view value = {});

    [[nodiscard]] Status rebuildHashTree(
        const std::string &file_path,
        EntryMap &entries);

} // namespace kv
