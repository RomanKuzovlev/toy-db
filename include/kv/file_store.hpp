#pragma once

#include <string>
#include <string_view>
#include "kv/store.hpp"

namespace kv {

class FileStore : public Store {
public:
    explicit FileStore(std::string file_path);
    ~FileStore() override = default;

    [[nodiscard]] Status set(std::string_view key, std::string_view value) override;

    [[nodiscard]] GetResult get(std::string_view key) const override;

    [[nodiscard]] Status remove(std::string_view key) override;

private:
    std::string file_path_;
};


} // namespace kv