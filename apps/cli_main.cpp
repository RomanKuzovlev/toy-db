#include <fstream>
#include <iostream>
#include <string>

#include "kv/file_store.hpp"

namespace {

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
        first.clear();
        rest.clear();
        return false;
    }

    std::size_t space_pos = cleaned.find(' ');
    if (space_pos == std::string::npos) {
        first = cleaned;
        rest.clear();
        return true;
    }

    first = cleaned.substr(0, space_pos);
    rest = trimLeft(cleaned.substr(space_pos + 1));
    return true;
}

void showFile(const std::string& db_path) {
    std::ifstream in(db_path);
    if (!in) {
        std::cout << "Failed to open DB file for reading.\n";
        return;
    }

    std::string line;
    std::cout << "DB contents:\n";
    while (std::getline(in, line)) {
        std::cout << line << '\n';
    }
}

} // namespace

int main() {
    const std::string db_path = "kv_db";

    // Ensure the DB file exists before FileStore starts using it.
    std::ofstream ensure_file(db_path, std::ios::app);
    if (!ensure_file) {
        std::cerr << "Failed to create or open DB file.\n";
        return 1;
    }

    kv::FileStore store(db_path);

    while (true) {
        std::string input;
        std::cout << "\nEnter command: ";
        std::getline(std::cin >> std::ws, input);

        if (!std::cin) {
            std::cerr << "Input failed.\n";
            return 1;
        }

        std::string command;
        std::string args;
        if (!splitFirstWord(input, command, args)) {
            continue;
        }

        if (command == "q" || command == "exit") {
            break;
        }

        if (command == "set" || command == "put") {
            std::string key;
            std::string value;
            if (!splitFirstWord(args, key, value) || value.empty()) {
                std::cout << "Usage: set <key> <value>\n";
                continue;
            }

            kv::Status status = store.set(key, value);
            if (!status.ok()) {
                std::cout << "Error: " << status.message << '\n';
                continue;
            }

            std::cout << "OK\n";
            continue;
        }

        if (command == "get") {
            std::string key = trimLeft(args);
            if (key.empty()) {
                std::cout << "Usage: get <key>\n";
                continue;
            }

            kv::GetResult result = store.get(key);
            if (!result.ok()) {
                std::cout << "Error: " << result.status.message << '\n';
                continue;
            }

            if (!result.found()) {
                std::cout << "(not found)\n";
                continue;
            }

            std::cout << *result.value << '\n';
            continue;
        }

        if (command == "remove" || command == "delete" || command == "rm") {
            std::string key = trimLeft(args);
            if (key.empty()) {
                std::cout << "Usage: remove <key>\n";
                continue;
            }

            kv::Status status = store.remove(key);
            if (!status.ok()) {
                std::cout << "Error: " << status.message << '\n';
                continue;
            }

            std::cout << "OK\n";
            continue;
        }

        if (command == "show") {
            showFile(db_path);
            continue;
        }

        std::cout << "Unknown command.\n";
        std::cout << "Available commands: set, get, remove, show, q, exit\n";
    }

    return 0;
}