#include <string>
#include <vector>
#include <fstream>
#include <utility>
#include <kv/file_store.hpp>

kv::Status kv::FileStore::set(std::string_view key, std::string_view value) {
    std::ifstream in(this->file_path_);
    if (!in) {
        return kv::Status{
            {kv::StoreError::io_error},
            "Failed to open DB file for reading.\n",
        };
    }

    std::vector<std::pair<std::string, std::string>> entries;
    std::string line;
    bool found = false;

    while (std::getline(in, line)) {
        if (line.empty()) {
            continue;
        }

        std::string lineKey;
        std::string lineValue;

        if (!splitFirstWord(line, lineKey, lineValue)) {
            continue;
        }

        if (lineKey == key) {
            entries.push_back({lineKey, std::string(value)});
            found = true;
        } else {
            entries.push_back({lineKey, lineValue});
        }
    }

    if (!found) {
        entries.push_back({std::string(key), std::string(value)});
    }

    std::ofstream out(this->file_path_, std::ios::trunc);
    if (!out) {
        return kv::Status{
            {kv::StoreError::io_error},
            "Failed to open DB file for writing.\n",
        };
    }

    for (const auto& entry : entries) {
        out << entry.first << ' ' << entry.second << '\n';
    }

    return kv::Status{};
}

kv::GetResult kv::FileStore::get(std::string_view key) const {
    std::ifstream in(this->file_path_);
    if (!in) {
        return kv::GetResult{
            kv::Status{
                {kv::StoreError::io_error},
                "Failed to open DB file for reading.\n",
            },
        };
    }

    std::string line;
    while (std::getline(in, line)) {
        if (line.empty()) {
            continue;
        }

        std::string lineKey;
        std::string lineValue;

        if (!splitFirstWord(line, lineKey, lineValue)) {
            continue;
        }

        if (lineKey == key) {
            return kv::GetResult{
                kv::Status{},
                lineValue,
            };
        }
    }

    return kv::GetResult{kv::Status{}};
}

kv::Status kv::FileStore::remove(std::string_view key) {
    std::ifstream in(this->file_path_);
    if (!in) {
        return kv::Status{
            {kv::StoreError::io_error},
            "Failed to open DB file for reading.\n",
        };
    }

    std::vector<std::pair<std::string, std::string>> entries;
    std::string line;

    while (std::getline(in, line)) {
        if (line.empty()) {
            continue;
        }

        std::string lineKey;
        std::string lineValue;

        if (!splitFirstWord(line, lineKey, lineValue)) {
            continue;
        }

        if (lineKey != key) {
            entries.push_back({lineKey, lineValue});
        }
    }

    std::ofstream out(this->file_path_, std::ios::trunc);
    if (!out) {
        return kv::Status{
            {kv::StoreError::io_error},
            "Failed to open DB file for writing.\n",
        };
    }

    for (const auto& entry : entries) {
        out << entry.first << ' ' << entry.second << '\n';
    }

    return kv::Status{};
}

std::string trimLeft(const std::string& text) {
    std::size_t start = text.find_first_not_of(' ');
    if (start == std::string::npos) {
        return "";
    }
    return text.substr(start);
}

bool splitFirstWord(const std::string& text, std::string& first, std::string& rest) {
    std::string cleaned = trimLeft(text);

    if (cleaned.empty()) {
        first = "";
        rest = "";
        return false;
    }

    std::size_t spacePos = cleaned.find(' ');

    if (spacePos == std::string::npos) {
        first = cleaned;
        rest = "";
        return true;
    }

    first = cleaned.substr(0, spacePos);
    rest = trimLeft(cleaned.substr(spacePos + 1));
    return true;
}   