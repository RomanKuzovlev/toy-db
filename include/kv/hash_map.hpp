#pragma once

#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

namespace kv
{

    class HashMap
    {
    public:
        explicit HashMap(std::size_t capacity)
            : slots_(capacity) {};

        bool set(std::string key, std::string value);
        const std::string *get(std::string_view key) const;
        bool remove(std::string_view key);
        void clear();

        std::size_t size() const noexcept;
        std::size_t capacity() const noexcept;

    private:
        enum class SlotState
        {
            empty,
            occupied,
            deleted,
        };

        struct Slot
        {
            SlotState state = SlotState::empty;
            std::string key;
            std::string value;
            std::size_t hash = 0;
        };

        std::vector<Slot> slots_;
        std::size_t size_ = 0;
    };

} // namespace kv
