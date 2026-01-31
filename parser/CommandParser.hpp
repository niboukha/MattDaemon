#ifndef COMMANDPARSER_HPP
#define COMMANDPARSER_HPP

#include <string>

class CommandParser {
    private:
        std::string command;
        std::string args;

    public:
        CommandParser();
        ~CommandParser();
        
        int parse(const std::string& message);
        std::string getCommand() const;
        std::string getArgs() const;
};

#endif
