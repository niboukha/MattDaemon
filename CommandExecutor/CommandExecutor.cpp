
#include "CommandExecutor.h"

enum CommandType {
    CMD_HELP,
    CMD_PWD,
    CMD_WHOAMI,
    CMD_LS,
    CMD_ECHO,
    CMD_RM,
    CMD_UNKNOWN
};

CommandType getCommandType(const std::string& command) {
    if (command == "help") return CMD_HELP;
    if (command == "pwd") return CMD_PWD;
    if (command == "whoami") return CMD_WHOAMI;
    if (command == "ls") return CMD_LS;
    if (command.rfind("echo", 0) == 0 || command.rfind("echo ", 0) == 0) return CMD_ECHO;
    if (command.rfind("rm ", 0) == 0) return CMD_RM;
    return CMD_UNKNOWN;
}



std::string CommandExecutor::executeCommand(std::string& command) {
    
    printf("Executing command: %s\n", command.c_str());

    switch (getCommandType(command)) {
        case CMD_HELP:
            return "Available commands:\n  ls - list files\n  pwd - show directory\n  whoami - show user\n  echo - echo text\n  exit - quit\n";
        case CMD_PWD: {
            char cwd[PATH_MAX];
            if (getcwd(cwd, sizeof(cwd)) != nullptr) {
                return std::string(cwd);
            } else {
                return "Error retrieving current directory\n";
            }
        }
        case CMD_WHOAMI: {
            char username[LOGIN_NAME_MAX];
            if (getlogin_r(username, sizeof(username)) == 0) {
                return std::string(username);
            } else {
                return "Error retrieving username";
            }
        }
        case CMD_LS: {
            FILE *pipe = popen("ls", "r");
            if (!pipe) {
                return "Error executing ls command";
            }
            char buffer[128];
            std::string result;
            while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
                result += buffer;
            }
            pclose(pipe);
            return result;
        }
        case CMD_ECHO:
            if(command.size() > 5) {
                command = command.substr(5);
                command.erase(0, command.find_first_not_of(" \t\n\r\f\v"));
                command.erase(command.find_last_not_of(" \t\n\r\f\v") + 1);
                return command;
            } else {
                return "";
            }
        case CMD_RM: {
           MailService().sendEmail("ALERT: File Deletion Detected",
                                 "A file deletion command was executed on the server: " + command);
           return "File deletion is not permitted on this server.\n";
        }
        default:
            return "Command not recognized. Type 'help' for a list of available commands.\n";
    }
}