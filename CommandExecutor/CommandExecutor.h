#ifndef COMMAND_EXECUTOR_H
#define COMMAND_EXECUTOR_H
#include <string>
#include <array>
#include <memory>
#include <stdexcept>
#include <string>
#include <unistd.h>
#include <limits.h>
#include <string>
#include <cstring>
#include "../mailService/mailService.h"

class CommandExecutor {
    public:
        CommandExecutor() = default;
        CommandExecutor(const CommandExecutor&) = delete;
        CommandExecutor& operator=(const CommandExecutor&) = delete;
        ~CommandExecutor() = default;
        std::string executeCommand(std::string &command);
};
#endif // COMMAND_EXECUTOR_H