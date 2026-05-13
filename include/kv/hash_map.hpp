#pragma once

#include <string>
#include <string_view>
#include <vector>
#include <functional>

namespace kv
{

    class HashMap
    {
    public:
        explicit HashMap(std::size_t capacity)
            : slots_(capacity) {};

        bool set(std::string key, std::string value);
        const std::string *get(std::string_view key) const;

        std::size_t size() const noexcept;
        std::size_t capacity() const noexcept;

    private:
        enum class SlotState
        {
            empty,
            occupied,
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

    std::size_t HashMap::size() const noexcept
    {
        return size_;
    }

    std::size_t HashMap::capacity() const noexcept
    {
        return slots_.size();
    };

    bool HashMap::set(std::string key, std::string value)
    {
        if (slots_.empty())
        {
            return false;
        }

        std::size_t hash = std::hash<std::string_view>{}(key);
        std::size_t index = hash % slots_.size();

        for (std::size_t probes = 0; probes < slots_.size(); ++probes)
        {
            Slot &slot = slots_[index];

            if (slot.state == SlotState::empty)
            {
                slot.state = SlotState::occupied;
                slot.key = key;
                slot.value = value;
                slot.hash = hash;
                ++size_;
                return true;
            }

            if (slot.hash == hash && slot.key == key)
            {
                slot.value = value;
                return true;
            }

            index = (index + 1) % slots_.size();
        }

        return false;
    };

    const std::string *HashMap::get(std::string_view key) const
    {
        if (slots_.empty())
        {
            return nullptr;
        }

        std::size_t hash = std::hash<std::string_view>{}(key);
        std::size_t index = hash % slots_.size();

        for (std::size_t probes = 0; probes < slots_.size(); ++probes)
        {
            const Slot &slot = slots_[index];

            if (slot.state == SlotState::empty)
            {
                return nullptr;
            }

            if (slot.hash == hash && slot.key == key)
            {
                return &slot.value;
            }

            index = (index + 1) % slots_.size();
        }

        return nullptr;
    }

} // namespace kv
