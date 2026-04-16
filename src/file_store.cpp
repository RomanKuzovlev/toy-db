#include <utility>

#include "kv/file_store.hpp"
#include "kv/wal.hpp"

kv::FileStore::FileStore(std::string file_path)
    : file_path_(std::move(file_path)) {
    init_status_ = rebuildHashTree(file_path_, entries_);
}

bool kv::FileStore::ready() const noexcept {
    return init_status_.ok();
}

kv::Status kv::FileStore::set(std::string_view key, std::string_view value) {
    if (!ready()) {
        return init_status_;
    }

    const kv::Status status = saveCommand(file_path_, '+', key, value);
    if (!status.ok()) {
        return status;
    }

    entries_[std::string(key)] = std::string(value);

    return kv::Status{};
}

kv::GetResult kv::FileStore::get(std::string_view key) const {
    if (!ready()) {
        return kv::GetResult{
            init_status_,
        };
    }

    const auto it = entries_.find(std::string(key));
    if (it == entries_.end()) {
        return kv::GetResult{kv::Status{}};
    }

    return kv::GetResult{
        kv::Status{},
        it->second,
    };
}

kv::Status kv::FileStore::remove(std::string_view key) {
    if (!ready()) {
        return init_status_;
    }

    const kv::Status status = saveCommand(file_path_, '-', key);
    if (!status.ok()) {
        return status;
    }

    entries_.erase(std::string(key));

    return kv::Status{};
}
