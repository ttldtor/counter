#include <algorithm>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string_regex.hpp>
#include <fmt/format.h>
#include <fstream>
#include <iostream>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

#include "CommandParser.hpp"

template<typename... Ts>
struct overloaded : Ts... {
  using Ts::operator()...;
};

template<class... Ts>
overloaded(Ts...) -> overloaded<Ts...>;

namespace cnt {
struct Counter {
  enum class ProcessingStatus { None, Quit, Ok, Error };

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
    fmt::print("Commands:\n"
               "  ? : Print this help\n"
               "  ! : Print the current records dump\n"
               "  q : Exit\n"
               "  + <record> : Add the record\n"
               "  - <record> : Remove the record\n"
               "  d - <file name> : Dump all records to the file\n"
               "  l - <file name> : Load records from the file\n\n");
  }

  void printDump() {
    std::cout << makeDump();
  }

  ProcessingStatus processCommand(const std::string &command) {
    return std::visit(overloaded{[](auto arg) {
                                   return printHelp(), ProcessingStatus::Error;
                                 },
                                 [](NoneCommand) {
                                   return ProcessingStatus::None;
                                 },
                                 [](PrintHelpCommand) {
                                   return printHelp(), ProcessingStatus::Ok;
                                 },
                                 [this](PrintRecordsCommand) {
                                   return printDump(), ProcessingStatus::Ok;
                                 },
                                 [](QuitCommand) {
                                   return ProcessingStatus::Quit;
                                 },
                                 [this](const AddRecordCommand &c) {
                                   add(c.recordName);

                                   return printDump(), ProcessingStatus::Ok;
                                 },
                                 [this](const RemoveRecordCommand &c) {
                                   remove(c.recordName);

                                   return printDump(), ProcessingStatus::Ok;
                                 },
                                 [this](const DumpRecordsCommand &c) {
                                   dumpToFile(c.fileName);

                                   return printDump(), ProcessingStatus::Ok;
                                 },
                                 [this](const LoadRecordsCommand &c) {
                                   auto result = loadFromFile(c.fileName);

                                   return printDump(), result;
                                 }},
                      CommandParser{}(command));
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

  void dumpToFile(const std::string &fileName) {
    std::ofstream os{fileName};

    os << makeDump();

    fmt::print("Dumped: {}\n", data.size());
  }

  ProcessingStatus loadFromFile(const std::string &fileName) {
    std::ifstream is(fileName);

    if (!is.is_open()) {
      fmt::print("Can't open file: {}\n", fileName);

      return ProcessingStatus::Error;
    }

    std::unordered_map<std::string, std::size_t> tempData{};

    for (std::string record{}; std::getline(is, record);) {
      Record r;
      auto recordString = boost::trim_copy(record);

      if (recordString.empty()) {
        continue;
      }

      if (r.fromString(recordString)) {
        tempData[r.name] = r.count;
      } else {
        is.close();
        fmt::print("Error while reading the file: {}, error record: '{}'\n", fileName, recordString);

        return ProcessingStatus::Error;
      }
    }

    is.close();

    data = tempData;
    fmt::print("The dump has been loaded.\n");

    return ProcessingStatus::Ok;
  }
};
}// namespace cnt

int main() {
  cnt::Counter counter{};

  cnt::Counter::printHelp();

  for (;;) {
    std::cout << "Counter> " << std::flush;

    std::string command{};
    std::getline(std::cin, command);

    if (counter.processCommand(boost::trim_copy(command)) == cnt::Counter::ProcessingStatus::Quit)
      break;
  }

  return 0;
}
