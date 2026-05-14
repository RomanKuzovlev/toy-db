#pragma once

#include <cstddef>
#include <string>
#include <string_view>
#include <unordered_map>

namespace kv
{

class StdHashMap
{
public:
    explicit StdHashMap(std::size_t capacity);

    bool set(std::string key, std::string value);
    const std::string *get(std::string_view key) const;
    bool remove(std::string_view key);
    void clear();

    std::size_t size() const noexcept;
    std::size_t capacity() const noexcept;

private:
    std::unordered_map<std::string, std::string> entries_;
};

} // namespace kv
