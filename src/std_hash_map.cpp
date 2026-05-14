#include "kv/std_hash_map.hpp"

#include <utility>

namespace kv
{

StdHashMap::StdHashMap(std::size_t capacity)
{
    entries_.reserve(capacity);
}

bool StdHashMap::set(std::string key, std::string value)
{
    entries_[std::move(key)] = std::move(value);
    return true;
}

const std::string *StdHashMap::get(std::string_view key) const
{
    const auto it = entries_.find(std::string(key));
    if (it == entries_.end())
    {
        return nullptr;
    }

    return &it->second;
}

bool StdHashMap::remove(std::string_view key)
{
    return entries_.erase(std::string(key)) > 0;
}

void StdHashMap::clear()
{
    entries_.clear();
}

std::size_t StdHashMap::size() const noexcept
{
    return entries_.size();
}

std::size_t StdHashMap::capacity() const noexcept
{
    return entries_.max_size();
}

} // namespace kv
