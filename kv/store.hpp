#pragma once

#include <optional>
#include <string>
#include <string_view>

namespace kv {

enum class StoreError {
    none = 0,
    io_error,
    corrupt_data,
    invalid_argument,
};

struct Status {
    StoreError error {StoreError::none};
    std::string message;

    [[nodiscard]] bool ok() const noexcept {
        return error == StoreError::none;
    }

    [[nodiscard]] explicit operator bool() const noexcept {
        return ok();
    }

    [[nodiscard]] static Status success() noexcept {
        return {};
    }
};

struct GetResult {
    Status status {};
    std::optional<std::string> value;

    [[nodiscard]] bool ok() const noexcept {
        return status.ok();
    }

    [[nodiscard]] bool found() const noexcept {
        return value.has_value();
    }

    [[nodiscard]] explicit operator bool() const noexcept {
        return ok();
    }
};

class Store {
public:
    virtual ~Store() = default;

    [[nodiscard]] virtual Status set(std::string_view key,
                                     std::string_view value) = 0;

    [[nodiscard]] virtual GetResult get(std::string_view key) const = 0;

    [[nodiscard]] virtual Status remove(std::string_view key) = 0;
};

} // namespace kv