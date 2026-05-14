#include <optional>
#include <utility>

#include "kv/file_store.hpp"
#include "kv/wal.hpp"

kv::FileStore::FileStore(std::string file_path)
    : file_path_(std::move(file_path)),
      entries_(65'536)
{
    init_status_ = rebuildHashTree(file_path_, entries_);
}

bool kv::FileStore::ready() const noexcept
{
    return init_status_.ok();
}

kv::Status kv::FileStore::set(std::string_view key, std::string_view value)
{
    if (!ready())
    {
        return init_status_;
    }

    if (entries_.get(key) == nullptr && entries_.size() == entries_.capacity())
    {
        return kv::Status{
            kv::StoreError::invalid_argument,
            "Hash map capacity exceeded.\n",
        };
    }

    const kv::Status status = saveCommand(file_path_, '+', key, value);
    if (!status.ok())
    {
        return status;
    }

    if (!entries_.set(std::string(key), std::string(value)))
    {
        return kv::Status{
            kv::StoreError::invalid_argument,
            "Hash map capacity exceeded.\n",
        };
    }

    return kv::Status{};
}

kv::GetResult kv::FileStore::get(std::string_view key) const
{
    if (!ready())
    {
        return kv::GetResult{
            init_status_,
            std::nullopt,
        };
    }

    const std::string *value = entries_.get(key);
    if (value == nullptr)
    {
        return kv::GetResult{
            kv::Status{},
            std::nullopt,
        };
    }

    return kv::GetResult{
        kv::Status{},
        *value,
    };
}

kv::Status kv::FileStore::remove(std::string_view key)
{
    if (!ready())
    {
        return init_status_;
    }

    const kv::Status status = saveCommand(file_path_, '-', key);
    if (!status.ok())
    {
        return status;
    }

    entries_.remove(key);

    return kv::Status{};
}
