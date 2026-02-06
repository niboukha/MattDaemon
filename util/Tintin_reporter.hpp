#ifndef TINTIN_REPORTER_HPP
#define TINTIN_REPORTER_HPP

#include <string>
#include <fstream>
#include <ctime>
#include <cstddef>

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

        std::size_t maxBytes; //size limit for log rotation
        std::size_t retentionCount; //number of rotated logs to keep
        bool gzipArchives; //whether to gzip rotated logs
        
        struct LogTarget {
            std::string path;
            std::ofstream stream;
        };

        LogTarget mainLog;
        LogTarget systemLog;
        LogTarget auditLog;
        LogTarget commandsLog;

        Tintin_reporter();
        
        std::string formatTimestamp() const;
        std::string levelToString(LogLevel level) const;
        void        openLog(LogTarget& target, const std::string& path);
        void        closeLog(LogTarget& target);
        void        writeToFile(LogTarget& target, const std::string& entry);

        void        rotateIfNeeded(LogTarget& target);
        void        rotateLog(LogTarget& target);
        void        cleanupArchives(const std::string& basePath) const;
        std::string buildArchiveName(const std::string& basePath) const;

    public:
        static Tintin_reporter& getInstance();
        
        Tintin_reporter(const Tintin_reporter&) = delete;
        Tintin_reporter& operator=(const Tintin_reporter&) = delete;        
        ~Tintin_reporter();
        
        void initialize(const std::string& filePath);
        void initializeCategoryLogs(const std::string& baseDir);
        
        void log(LogLevel level, const std::string& message);
        void info(const std::string& message);
        void warning(const std::string& message);
        void error(const std::string& message);
        void debug(const std::string& message);
        void logUserInput(const std::string& input);
        void userLog(const std::string& message);
        
        //routes messages for specific files
        void logSystem(LogLevel level, const std::string& message);
        void logAudit(LogLevel level, const std::string& message);
        void logCommand(const std::string& message);
        
        void setLogLevel(LogLevel level);
        void setLogFile(const std::string& filePath);
        void setRotationPolicy(std::size_t maxBytes, std::size_t retentionCount, bool gzipArchives);
};

#endif
