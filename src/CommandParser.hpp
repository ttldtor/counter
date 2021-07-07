#pragma once

#include <boost/algorithm/string.hpp>
#include <string>

namespace cnt {
struct NoneCommand {};
struct UnknownCommand {};
struct PrintHelpCommand {};
struct PrintRecordsCommand {};
struct QuitCommand {};
struct AddRecordCommand {
  std::string recordName{};

  explicit AddRecordCommand(std::string s) : recordName{std::move(s)} {
  }
};

struct RemoveRecordCommand {
  std::string recordName{};

  explicit RemoveRecordCommand(std::string s) : recordName{std::move(s)} {
  }
};

struct RemoveAllRecordsCommand {};

struct DumpRecordsCommand {
  std::string fileName{};

  explicit DumpRecordsCommand(std::string s) : fileName{std::move(s)} {
  }
};

struct LoadRecordsCommand {
  std::string fileName{};

  explicit LoadRecordsCommand(std::string s) : fileName{std::move(s)} {
  }
};

using Command = std::variant<NoneCommand, UnknownCommand, PrintHelpCommand, PrintRecordsCommand, QuitCommand,
                             AddRecordCommand, RemoveRecordCommand, RemoveAllRecordsCommand, DumpRecordsCommand, LoadRecordsCommand>;

struct CommandParser {
  Command operator()(const std::string &commandString) const {
    if (commandString.empty())
      return NoneCommand{};

    if (commandString == "?") {
      return PrintHelpCommand{};
    }

    if (commandString == "!") {
      return PrintRecordsCommand{};
    }

    if (commandString == "q") {
      return QuitCommand{};
    }

    if (commandString == "*") {
      return RemoveAllRecordsCommand{};
    }

    if (commandString.size() < 3) {
      return UnknownCommand{};
    }

    if (commandString.starts_with("+ ")) {
      return AddRecordCommand{boost::trim_copy(commandString.substr(2))};
    }

    if (commandString.starts_with("- ")) {
      return RemoveRecordCommand{boost::trim_copy(commandString.substr(2))};
    }

    if (commandString.starts_with("d ")) {
      return DumpRecordsCommand{boost::trim_copy(commandString.substr(2))};
    }

    if (commandString.starts_with("l ")) {
      return LoadRecordsCommand{boost::trim_copy(commandString.substr(2))};
    }

    return UnknownCommand{};
  }
};
}// namespace cnt