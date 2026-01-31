#include "CommandParser.hpp"
#include <algorithm>
#include <cctype>
#include <sstream>

CommandParser::CommandParser() {}

CommandParser::~CommandParser() {}

static std::string trim(const std::string& input) {
    size_t start = 0;
    while (start < input.size() && std::isspace(static_cast<unsigned char>(input[start]))) {
        ++start;
    }

    size_t end = input.size();
    while (end > start && std::isspace(static_cast<unsigned char>(input[end - 1]))) {
        --end;
    }

    return input.substr(start, end - start);
}

int CommandParser::parse(const std::string& message) {
    command.clear();
    args.clear();

    std::string trimmed = trim(message);
    if (trimmed.empty()) {
        return -1;
    }

    std::istringstream iss(trimmed);
    iss >> command;
    std::getline(iss, args);
    args = trim(args);

    std::transform(command.begin(), command.end(), command.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

    return 0;
}

std::string CommandParser::getCommand() const {
    return command;
}

std::string CommandParser::getArgs() const {
    return args;
}
