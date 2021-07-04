#include <algorithm>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string_regex.hpp>
#include <concepts>
#include <fmt/format.h>
#include <fstream>
#include <iostream>
#include <string_view>
#include <unordered_map>
#include <vector>

struct Counter {
    enum class ProcessingStatus { None,
                                  Quit,
                                  Ok,
                                  Error };

    struct Record {
        std::string name{};
        std::size_t count{};

        [[nodiscard]] std::string toString() const {
            return fmt::format("{} - {}", name, count);
        }

        bool fromString(const std::string &str) {
            std::vector<std::string> split = {};

            boost::algorithm::split_regex(split, str, boost::regex(R"(\s+-\s+)"));

            if (split.size() == 2) {
                name = split[0];
                count = static_cast<std::size_t>(std::stoull(split[1]));

                return true;
            }

            return false;
        }

        friend std::ostream &operator<<(std::ostream &stream, const Counter::Record &record) {
            stream << record.toString();

            return stream;
        }
    };

    std::unordered_map<std::string, std::size_t> data{};

    std::string makeDump() {
        std::vector<Record> records;
        std::string result;

        for (auto [k, v] : data) {
            records.emplace_back(Record{k, v});
        }

        std::sort(records.begin(), records.end(), [](const Record &r1, const Record &r2) {
            return (r1.count > r2.count) || (r1.count == r2.count && r1.name < r2.name);
        });

        for (const auto &r : records) {
            result += fmt::format("{}\n", r.toString());
        }

        return result;
    }

    static void printHelp() {
        fmt::print("? -> print the status; q -> exit; + <record> -> add the record;\n"
                   "- <record> -> remove the record; d - <file name> -> dump all records to file;\n"
                   "l - <file name> -> load records from file\n\n");
    }

    void printStatus() {
        printHelp();

        std::cout << makeDump();
    }

    ProcessingStatus processCommand(const std::string &command) {
        if (command.empty()) return ProcessingStatus::None;

        if (command == "?") {
            printStatus();

            return ProcessingStatus::Ok;
        }

        if (command == "q") return ProcessingStatus::Quit;

        if (command.size() < 2) return ProcessingStatus::Error;

        if (command.starts_with("+ ")) {
            add(command.substr(2));
            printStatus();

            return ProcessingStatus::Ok;
        }

        if (command.starts_with("- ")) {
            remove(command.substr(2));
            printStatus();

            return ProcessingStatus::Ok;
        }

        if (command.starts_with("d ")) {
            dump(command.substr(2));

            printStatus();

            return ProcessingStatus::Ok;
        }

        if (command.starts_with("l ")) {
            if (load(command.substr(2)) == ProcessingStatus::Ok) {
            }

            printStatus();

            return ProcessingStatus::Ok;
        }

        printHelp();

        return ProcessingStatus::Ok;
    }

    void add(const std::string &name) {
        auto it = data.find(name);

        if (it == data.end()) {
            data[name] = 1;

            fmt::print("Added the '{}' record\n", name);
        } else {
            it->second++;

            fmt::print("Incremented the '{}' record\n", name);
        }
    }

    void remove(const std::string &name) {
        auto it = data.find(name);

        if (it != data.end()) {
            it->second--;

            if (it->second == 0) {
                data.erase(it);

                fmt::print("Removed the '{}' record\n", name);
            } else {
                fmt::print("Decremented the '{}' record\n", name);
            }
        }
    }

    void dump(const std::string &fileName) {
        std::ofstream os{fileName};

        os << makeDump();

        fmt::print("Dumped: {}\n", data.size());
    }

    ProcessingStatus load(const std::string &fileName) {
        std::ifstream is(fileName);

        if (!is.is_open()) {
            fmt::print("Can't open file: {}\n", fileName);

            return ProcessingStatus::Error;
        }

        for (std::string record{}; std::getline(is, record);) {
            Record r;
            auto recordString = boost::trim_copy(record);

            if (recordString.empty()) {
                continue;
            }

            if (r.fromString(recordString)) {
                data[r.name] = r.count;
            } else {
                is.close();
                fmt::print("Error while reading the file: {}, error record: '{}'\n", fileName, recordString);

                return ProcessingStatus::Error;
            }
        }

        is.close();
        fmt::print("The dump has been loaded.\n");

        return ProcessingStatus::Ok;
    }
};

int main() {
    Counter counter{};

    std::string command{};

    for (;;) {
        std::cout << "Counter> " << std::flush;
        std::getline(std::cin, command);
        if (counter.processCommand(boost::trim_copy(command)) == Counter::ProcessingStatus::Quit) break;
    }

    return 0;
}
