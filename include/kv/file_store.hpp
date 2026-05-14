#pragma once

#include <string>
#include <string_view>

#include "kv/entry_map.hpp"
#include "kv/store.hpp"

namespace kv
{

    class FileStore : public Store
    {
    public:
        explicit FileStore(std::string file_path);
        ~FileStore() override = default;

        [[nodiscard]] Status set(std::string_view key, std::string_view value) override;

        [[nodiscard]] GetResult get(std::string_view key) const override;

        [[nodiscard]] Status remove(std::string_view key) override;

    private:
        [[nodiscard]] bool ready() const noexcept;

        std::string file_path_;
        EntryMap entries_;
        Status init_status_{};
    };

} // namespace kv
