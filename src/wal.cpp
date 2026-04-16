#include "kv/wal.hpp"

#include <fstream>
#include <string>

namespace {

kv::Status invalidWalLine(std::size_t line_number) {
    return kv::Status{
        kv::StoreError::corrupt_data,
        "Invalid WAL record at line " + std::to_string(line_number) + ".\n",
    };
}

bool hasLineBreak(std::string_view text) {
    for (char ch : text) {
        if (ch == '\n' || ch == '\r') {
            return true;
        }
    }

    return false;
}

} // namespace

kv::Status kv::saveCommand(const std::string& file_path,
                           char command,
                           std::string_view key,
                           std::string_view value) {
    if (key.empty() || key.find(' ') != std::string_view::npos || hasLineBreak(key) ||
        hasLineBreak(value)) {
        return kv::Status{
            kv::StoreError::invalid_argument,
            "Keys cannot contain spaces or line breaks, and values cannot contain line breaks.\n",
        };
    }

    if (command != '+' && command != '-') {
        return kv::Status{
            kv::StoreError::invalid_argument,
            "Unknown WAL command.\n",
        };
    }

    std::ofstream out(file_path, std::ios::app);
    if (!out) {
        return kv::Status{
            kv::StoreError::io_error,
            "Failed to open WAL for appending.\n",
        };
    }

    if (command == '+') {
        out << "+ " << key << ' ' << value << '\n';
    } else {
        out << "- " << key << '\n';
    }

    if (!out) {
        return kv::Status{
            kv::StoreError::io_error,
            "Failed to write WAL command.\n",
        };
    }

    return kv::Status{};
}

kv::Status kv::rebuildHashTree(
    const std::string& file_path,
    std::unordered_map<std::string, std::string>& entries) {
    std::ifstream in(file_path);
    if (!in) {
        std::ofstream create(file_path, std::ios::app);
        if (!create) {
            return kv::Status{
                kv::StoreError::io_error,
                "Failed to create WAL file.\n",
            };
        }

        entries.clear();
        return kv::Status{};
    }

    entries.clear();

    std::string line;
    std::size_t line_number = 0;
    while (std::getline(in, line)) {
        ++line_number;

        if (line.empty()) {
            continue;
        }

        if (line.size() < 3 || line[1] != ' ') {
            return invalidWalLine(line_number);
        }

        if (line[0] == '+') {
            const std::size_t key_end = line.find(' ', 2);
            if (key_end == std::string::npos || key_end == 2) {
                return invalidWalLine(line_number);
            }

            const std::string key = line.substr(2, key_end - 2);
            const std::string value = line.substr(key_end + 1);
            entries[key] = value;
            continue;
        }

        if (line[0] == '-') {
            if (line.find(' ', 2) != std::string::npos) {
                return invalidWalLine(line_number);
            }

            const std::string key = line.substr(2);
            if (key.empty()) {
                return invalidWalLine(line_number);
            }

            entries.erase(key);
            continue;
        }

        return invalidWalLine(line_number);
    }

    if (!in.eof()) {
        return kv::Status{
            kv::StoreError::io_error,
            "Failed while reading WAL file.\n",
        };
    }

    return kv::Status{};
}
