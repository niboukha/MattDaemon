#ifndef TINTIN_REPORTER_HPP
#define TINTIN_REPORTER_HPP

#include <string>
#include <fstream>
#include <ctime>

enum LogLevel {
    DEBUG,
    INFO,
    WARNING,
    ERROR,
    LOG
};

class Tintin_reporter {
    private:
        std::string logFilePath;
        std::ofstream logFile;
        LogLevel currentLevel;
        
        Tintin_reporter();
        
        std::string formatTimestamp() const;
        std::string levelToString(LogLevel level) const;
        void writeToFile(const std::string& entry);

    public:
        static Tintin_reporter& getInstance();
        
        Tintin_reporter(const Tintin_reporter&) = delete;
        Tintin_reporter& operator=(const Tintin_reporter&) = delete;        
        ~Tintin_reporter();
        
        void initialize(const std::string& filePath);
        
        void log(LogLevel level, const std::string& message);
        void info(const std::string& message);
        void warning(const std::string& message);
        void error(const std::string& message);
        void debug(const std::string& message);
        void logUserInput(const std::string& input);
        void userLog(const std::string& message);
        
        void logMessageReceived(const std::string& client, const std::string& content);
        void logMessageStored(int messageId, const std::string& sender);
        void logClientConnection(const std::string& clientInfo);
        void logClientDisconnection(const std::string& clientInfo);
        void logDaemonStart();
        void logDaemonStop();
        
        void setLogLevel(LogLevel level);
        void setLogFile(const std::string& filePath);

};

#endif
