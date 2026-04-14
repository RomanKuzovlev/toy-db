#include <chrono>
#include <fstream>
#include <iostream>
#include <random>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "kv/file_store.hpp"

namespace {

struct Command {
    enum class Type {
        set,
        get,
        remove,
    };

    Type type;
    std::string key;
    std::string value;
};

struct Timings {
    std::chrono::nanoseconds total {0};
    std::chrono::nanoseconds set_time {0};
    std::chrono::nanoseconds get_time {0};
    std::chrono::nanoseconds remove_time {0};

    int set_count = 0;
    int get_count = 0;
    int remove_count = 0;
};

std::vector<Command> generateOperations(int operation_count) {
    std::vector<Command> commands;
    commands.reserve(operation_count);

    std::mt19937 rng(42);
    std::uniform_int_distribution<int> op_dist(0, 99);
    std::uniform_int_distribution<int> key_dist(0, operation_count / 4);
    std::uniform_int_distribution<int> value_dist(0, 1'000'000);

    for (int i = 0; i < operation_count; ++i) {
        int op_roll = op_dist(rng);
        int key_id = key_dist(rng);

        Command cmd;
        cmd.key = "key_" + std::to_string(key_id);

        if (op_roll < 50) {
            cmd.type = Command::Type::set;
            cmd.value = "value_" + std::to_string(value_dist(rng)) + "_op_" + std::to_string(i);
        } else if (op_roll < 80) {
            cmd.type = Command::Type::get;
        } else {
            cmd.type = Command::Type::remove;
        }

        commands.push_back(std::move(cmd));
    }

    return commands;
}

void printSummary(const Timings& timings, int passed, int failed) {
    using namespace std::chrono;

    auto to_seconds = [](nanoseconds ns) {
        return duration<double>(ns).count();
    };

    std::cout << "\n=== Test summary ===\n";
    std::cout << "Passed checks: " << passed << '\n';
    std::cout << "Failed checks: " << failed << '\n';

    std::cout << "\n=== Timing summary ===\n";
    std::cout << "Total time: " << to_seconds(timings.total) << " s\n";

    std::cout << "set count: " << timings.set_count
              << ", total: " << to_seconds(timings.set_time) << " s";
    if (timings.set_count > 0) {
        std::cout << ", avg: " << (to_seconds(timings.set_time) / timings.set_count) << " s";
    }
    std::cout << '\n';

    std::cout << "get count: " << timings.get_count
              << ", total: " << to_seconds(timings.get_time) << " s";
    if (timings.get_count > 0) {
        std::cout << ", avg: " << (to_seconds(timings.get_time) / timings.get_count) << " s";
    }
    std::cout << '\n';

    std::cout << "remove count: " << timings.remove_count
              << ", total: " << to_seconds(timings.remove_time) << " s";
    if (timings.remove_count > 0) {
        std::cout << ", avg: " << (to_seconds(timings.remove_time) / timings.remove_count) << " s";
    }
    std::cout << '\n';
}

} // namespace

int main() {
    const std::string db_path = "kv_db_test";
    const int operations = 10000;

    {
        std::ofstream reset(db_path, std::ios::trunc);
        if (!reset) {
            std::cerr << "Failed to create or truncate DB file.\n";
            return 1;
        }
    }

    kv::FileStore store(db_path);
    std::vector<Command> commands = generateOperations(operations);

    std::unordered_map<std::string, std::string> model;
    Timings timings {};

    int passed = 0;
    int failed = 0;

    const auto total_start = std::chrono::steady_clock::now();

    for (std::size_t i = 0; i < commands.size(); ++i) {
        const Command& cmd = commands[i];

        if (cmd.type == Command::Type::set) {
            const auto op_start = std::chrono::steady_clock::now();
            kv::Status status = store.set(cmd.key, cmd.value);
            const auto op_end = std::chrono::steady_clock::now();

            timings.set_time += (op_end - op_start);
            ++timings.set_count;

            if (!status.ok()) {
                ++failed;
                std::cout << "[FAIL] set(" << cmd.key << ", " << cmd.value
                          << ") returned error: " << status.message << '\n';
                continue;
            }

            model[cmd.key] = cmd.value;

            kv::GetResult verify = store.get(cmd.key);
            if (!verify.ok()) {
                ++failed;
                std::cout << "[FAIL] verification get after set failed for key "
                          << cmd.key << ": " << verify.status.message << '\n';
                continue;
            }

            if (!verify.found() || !verify.value.has_value() || *verify.value != cmd.value) {
                ++failed;
                std::cout << "[FAIL] verification get after set mismatch for key "
                          << cmd.key << '\n';
                continue;
            }

            ++passed;
            continue;
        }

        if (cmd.type == Command::Type::get) {
            const auto op_start = std::chrono::steady_clock::now();
            kv::GetResult result = store.get(cmd.key);
            const auto op_end = std::chrono::steady_clock::now();

            timings.get_time += (op_end - op_start);
            ++timings.get_count;

            if (!result.ok()) {
                ++failed;
                std::cout << "[FAIL] get(" << cmd.key
                          << ") returned error: " << result.status.message << '\n';
                continue;
            }

            auto it = model.find(cmd.key);
            const bool expected_found = (it != model.end());
            const bool actual_found = result.found();

            if (expected_found != actual_found) {
                ++failed;
                std::cout << "[FAIL] get(" << cmd.key
                          << ") found-state mismatch. expected=" << expected_found
                          << ", actual=" << actual_found << '\n';
                continue;
            }

            if (expected_found && actual_found && *result.value != it->second) {
                ++failed;
                std::cout << "[FAIL] get(" << cmd.key
                          << ") value mismatch. expected=" << it->second
                          << ", actual=" << *result.value << '\n';
                continue;
            }

            ++passed;
            continue;
        }

        if (cmd.type == Command::Type::remove) {
            const auto op_start = std::chrono::steady_clock::now();
            kv::Status status = store.remove(cmd.key);
            const auto op_end = std::chrono::steady_clock::now();

            timings.remove_time += (op_end - op_start);
            ++timings.remove_count;

            if (!status.ok()) {
                ++failed;
                std::cout << "[FAIL] remove(" << cmd.key
                          << ") returned error: " << status.message << '\n';
                continue;
            }

            model.erase(cmd.key);

            kv::GetResult verify = store.get(cmd.key);
            if (!verify.ok()) {
                ++failed;
                std::cout << "[FAIL] verification get after remove failed for key "
                          << cmd.key << ": " << verify.status.message << '\n';
                continue;
            }

            if (verify.found()) {
                ++failed;
                std::cout << "[FAIL] key still present after remove: " << cmd.key << '\n';
                continue;
            }

            ++passed;
        }
    }

    const auto total_end = std::chrono::steady_clock::now();
    timings.total = total_end - total_start;

    printSummary(timings, passed, failed);

    return failed == 0 ? 0 : 1;
}