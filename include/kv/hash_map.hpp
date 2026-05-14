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
        bool remove(std::string_view key);
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

    inline std::size_t HashMap::size() const noexcept
    {
        return size_;
    }

    inline std::size_t HashMap::capacity() const noexcept
    {
        return slots_.size();
    };

    inline bool HashMap::set(std::string key, std::string value)
    {
        if (slots_.empty())
        {
            return false;
        }

        std::size_t hash = std::hash<std::string_view>{}(key);
        std::size_t index = hash % slots_.size();
        std::size_t first_deleted = slots_.size();

        for (std::size_t probes = 0; probes < slots_.size(); ++probes)
        {
            Slot &slot = slots_[index];
            if (slot.state == SlotState::deleted && first_deleted == slots_.size())
            {
                first_deleted = index;
            }

            if (slot.state == SlotState::empty)
            {
                Slot &target = first_deleted == slots_.size()
                                   ? slot
                                   : slots_[first_deleted];

                target.state = SlotState::occupied;
                target.key = key;
                target.value = value;
                target.hash = hash;
                ++size_;
                return true;
            }

            if (slot.state == SlotState::occupied && slot.hash == hash && slot.key == key)
            {
                slot.value = value;
                return true;
            }

            index = (index + 1) % slots_.size();
        }

        if (first_deleted != slots_.size())
        {
            Slot &target = slots_[first_deleted];
            target.state = SlotState::occupied;
            target.key = key;
            target.value = value;
            target.hash = hash;
            ++size_;
            return true;
        }

        return false;
    };

    inline const std::string *HashMap::get(std::string_view key) const
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

            if (slot.state == SlotState::occupied && slot.hash == hash && slot.key == key)
            {
                return &slot.value;
            }

            index = (index + 1) % slots_.size();
        }

        return nullptr;
    }

    inline bool HashMap::remove(std::string_view key)
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
                return false;
            }

            if (slot.state == SlotState::occupied && slot.hash == hash && slot.key == key)
            {
                slot.state = SlotState::deleted;
                slot.key.clear();
                slot.value.clear();
                slot.hash = 0;
                --size_;

                return true;
            }

            index = (index + 1) % slots_.size();
        }

        return false;
    }

} // namespace kv
