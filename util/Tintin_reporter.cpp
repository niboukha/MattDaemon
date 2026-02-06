#include "Tintin_reporter.hpp"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <sys/stat.h>

Tintin_reporter::Tintin_reporter() : currentLevel(INFO) {}

Tintin_reporter::~Tintin_reporter() {
    if (logFile.is_open()) {
        logFile.close();
    }
}

Tintin_reporter& Tintin_reporter::getInstance() {
    static Tintin_reporter instance;
    return instance;
}

void Tintin_reporter::initialize(const std::string& filePath = "/var/log/matt_daemon/matt_daemon.log") {
    if (logFile.is_open()) {
        logFile.close();
    }
    
    logFilePath = filePath;
    
    // Create directory if it doesn't exist
    size_t pos = logFilePath.find_last_of("/");
    if (pos != std::string::npos) {
        std::string dir = logFilePath.substr(0, pos);
        mkdir(dir.c_str(), 0755);
    }
    
    logFile.open(logFilePath, std::ios::out | std::ios::app);
    
    if (!logFile.is_open()) {
        std::cerr << "Tintin_reporter: Unable to open log file: " << logFilePath << std::endl;
    }
}

std::string Tintin_reporter::formatTimestamp() const {
    time_t now = time(nullptr);
    struct tm* timeinfo = localtime(&now);
    
    std::ostringstream oss;
    oss << std::setfill('0')
        << std::setw(2) << timeinfo->tm_mday << "/"
        << std::setw(2) << (timeinfo->tm_mon + 1) << "/"
        << (timeinfo->tm_year + 1900) << "-"
        << std::setw(2) << timeinfo->tm_hour << ":"
        << std::setw(2) << timeinfo->tm_min << ":"
        << std::setw(2) << timeinfo->tm_sec;
    
    return oss.str();
}

std::string Tintin_reporter::levelToString(LogLevel level) const {
    switch (level) {
        case DEBUG:   return "DEBUG";
        case INFO:    return "INFO";
        case WARNING: return "WARNING";
        case ERROR:   return "ERROR";
        case LOG:     return "LOG";
        default:      return "UNKNOWN";
    }
}

void Tintin_reporter::writeToFile(const std::string& entry) {
    if (logFile.is_open()) {
        logFile << entry << std::endl;
        logFile.flush();
    }
}

void Tintin_reporter::log(LogLevel level, const std::string& message) {
    if (level < currentLevel) {
        return;
    }
    
    std::ostringstream oss;
    oss << "[" << formatTimestamp() << "] "
        << "[ " << levelToString(level) << " ] "
        << "- " << message;
    
    writeToFile(oss.str());
}

void Tintin_reporter::info(const std::string& message) {
    log(INFO, message);
}

void Tintin_reporter::warning(const std::string& message) {
    log(WARNING, message);
}

void Tintin_reporter::error(const std::string& message) {
    log(ERROR, message);
}

void Tintin_reporter::debug(const std::string& message) {
    log(DEBUG, message);
}

void Tintin_reporter::logUserInput(const std::string& input) {
    std::ostringstream oss;
    oss << "[" << formatTimestamp() << "] "
        << "[ LOG ] "
        << "- Matt_daemon: User input: " << input;
    writeToFile(oss.str());
}

// void Tintin_reporter::logMessageReceived(const std::string& client, const std::string& content) {
//     log(INFO, "Message received from " + client + ": " + content);
// }

// void Tintin_reporter::logMessageStored(int messageId, const std::string& sender) {
//     log(INFO, "Message #" + std::to_string(messageId) + " from " + sender + " stored");
// }

// void Tintin_reporter::logClientConnection(const std::string& clientInfo) {
//     log(INFO, "Client connected: " + clientInfo);
// }

// void Tintin_reporter::logClientDisconnection(const std::string& clientInfo) {
//     log(INFO, "Client disconnected: " + clientInfo);
// }

// void Tintin_reporter::logDaemonStart() {
//     log(INFO, "Daemon started");
// }

// void Tintin_reporter::logDaemonStop() {
//     log(INFO, "Daemon stopped");
// }

void Tintin_reporter::userLog(const std::string& message) {
    std::ostringstream oss;
    oss << "[" << formatTimestamp() << "] "
        << "[ LOG ] "
        << "- " << message;
    writeToFile(oss.str());
}

void Tintin_reporter::setLogLevel(LogLevel level) {
    currentLevel = level;
}

void Tintin_reporter::setLogFile(const std::string& filePath) {
    initialize(filePath);
}
