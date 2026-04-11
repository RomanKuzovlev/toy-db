#include <fstream>
#include <iostream>
#include <string>
#include <vector>

int parseAndProcess(const std::string& input, const std::string& dbFile);
std::string trimLeft(const std::string& text);
bool splitFirstWord(const std::string& text, std::string& first, std::string& rest);

int main() {
    const std::string dbFile = "db.txt";

    // create the file if it doesn't exist
    std::ifstream test(dbFile);
    if (!test) {
        std::cout << "No DB file was found. Creating a new one.\n";
        std::ofstream create(dbFile);
    } else {
        std::cout << "DB file found, using it.\n";
    }

    while (true) {
        std::string input;
        std::cout << "\nEnter your command: ";
        std::getline(std::cin >> std::ws, input);

        if (input == "exit" || input == "q") {
            break;
        }

        if (input.empty()) {
            continue;
        }

        int result = parseAndProcess(input, dbFile);
        if (result == 0) {
            std::cout << "Unknown or invalid command.\n";
        }
    }

    return 0;
}

int parseAndProcess(const std::string& input, const std::string& dbFile) {
    std::string command;
    std::string args;

    if (!splitFirstWord(input, command, args)) {
        return 0;
    }

    if (command == "put") {
        std::string key;
        std::string value;

        if (!splitFirstWord(args, key, value) || value.empty()) {
            std::cout << "Usage: put <key> <value>\n";
            return 1;
        }

        std::ifstream in(dbFile);
        if (!in) {
            std::cout << "Failed to open DB file for reading.\n";
            return 1;
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
                entries.push_back({key, value});
                found = true;
            } else {
                entries.push_back({lineKey, lineValue});
            }
        }

        in.close();

        if (!found) {
            entries.push_back({key, value});
        }

        std::ofstream out(dbFile, std::ios::trunc);
        if (!out) {
            std::cout << "Failed to open DB file for writing.\n";
            return 1;
        }

        for (const auto& entry : entries) {
            out << entry.first << ' ' << entry.second << '\n';
        }

        if (found) {
            std::cout << "Updated key: " << key << '\n';
        } else {
            std::cout << "Added key: " << key << '\n';
        }

        return 1;
    }

    if (command == "get") {
        std::string key = trimLeft(args);

        if (key.empty()) {
            std::cout << "Usage: get <key>\n";
            return 1;
        }

        std::ifstream in(dbFile);
        if (!in) {
            std::cout << "Failed to open DB file for reading.\n";
            return 1;
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
                std::cout << lineValue << '\n';
                return 1;
            }
        }

        std::cout << "Key not found: " << key << '\n';
        return 1;
    }

    if (command == "list") {
        std::ifstream in(dbFile);
        if (!in) {
            std::cout << "Failed to open DB file for reading.\n";
            return 1;
        }

        std::string line;
        std::cout << "DB contents:\n";
        while (std::getline(in, line)) {
            std::cout << line << '\n';
        }

        return 1;
    }

    return 0;
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