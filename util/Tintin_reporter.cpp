#include "Tintin_reporter.hpp"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <dirent.h>
#include <vector>
#include <algorithm>
#include <cstring>
#include <cstdlib>

namespace {
    std::string getDirName(const std::string& path) {
        size_t pos = path.find_last_of("/");
        if (pos == std::string::npos) {
            return ".";
        }
        return path.substr(0, pos);
    }

    std::string getBaseName(const std::string& path) {
        size_t pos = path.find_last_of("/");
        if (pos == std::string::npos) {
            return path;
        }
        return path.substr(pos + 1);
    }
}

Tintin_reporter::Tintin_reporter() : currentLevel(INFO), maxBytes(2 * 1024 * 1024), retentionCount(2), gzipArchives(true) {}

Tintin_reporter::~Tintin_reporter() {
    closeLog(mainLog);
    closeLog(systemLog);
    closeLog(auditLog);
    closeLog(commandsLog);
}

Tintin_reporter& Tintin_reporter::getInstance() {
    static Tintin_reporter instance;
    return instance;
}

void Tintin_reporter::initialize(const std::string& filePath) {
    logFilePath = filePath;
    openLog(mainLog, filePath);
    initializeCategoryLogs(getDirName(filePath));
}

void Tintin_reporter::initializeCategoryLogs(const std::string& baseDir) {
    std::string dir = baseDir.empty() ? "/var/log/matt_daemon" : baseDir;
    openLog(systemLog, dir + "/system.log");
    openLog(auditLog, dir + "/audit.log");
    openLog(commandsLog, dir + "/commands.log");
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

void Tintin_reporter::openLog(LogTarget& target, const std::string& path) {
    closeLog(target);
    target.path = path;

    std::string dir = getDirName(path);
    if (!dir.empty()) {
        mkdir(dir.c_str(), 0755);
    }

    target.stream.open(path, std::ios::out | std::ios::app);
    if (!target.stream.is_open()) {
        std::cerr << "Tintin_reporter: Unable to open log file: " << path << std::endl;
    }
}

void Tintin_reporter::closeLog(LogTarget& target) {
    if (target.stream.is_open()) {
        target.stream.close();
    }
}

void Tintin_reporter::writeToFile(LogTarget& target, const std::string& entry) {
    rotateIfNeeded(target);
    if (target.stream.is_open()) {
        target.stream << entry << std::endl;
        target.stream.flush();
    }
}

void Tintin_reporter::rotateIfNeeded(LogTarget& target) {
    if (maxBytes == 0 || target.path.empty()) {
        return;
    }

    struct stat st;
    if (stat(target.path.c_str(), &st) != 0) {
        return;
    }

    if (static_cast<std::size_t>(st.st_size) >= maxBytes) {
        rotateLog(target);
    }
}

void Tintin_reporter::rotateLog(LogTarget& target) {
    if (target.path.empty()) {
        return;
    }

    closeLog(target);

    std::string archivePath = buildArchiveName(target.path);
    if (rename(target.path.c_str(), archivePath.c_str()) != 0) {
        openLog(target, target.path);
        return;
    }

    if (gzipArchives) {
        std::string command = "gzip -f \"" + archivePath + "\"";
        std::system(command.c_str());
    }

    openLog(target, target.path);
    cleanupArchives(target.path);
}

std::string Tintin_reporter::buildArchiveName(const std::string& basePath) const {
    time_t now = time(nullptr);
    struct tm* timeinfo = localtime(&now);

    std::ostringstream oss;
    oss << basePath << "."
        << (timeinfo->tm_year + 1900) << "-"
        << std::setfill('0') << std::setw(2) << (timeinfo->tm_mon + 1) << "-"
        << std::setw(2) << timeinfo->tm_mday << "-T-"
        << std::setw(2) << timeinfo->tm_hour << "-"
        << std::setw(2) << timeinfo->tm_min << "-"
        << std::setw(2) << timeinfo->tm_sec
        << ".pid" << getpid();

    return oss.str();
}

void Tintin_reporter::cleanupArchives(const std::string& basePath) const {
    if (retentionCount == 0) {
        return;
    }

    std::string dir = getDirName(basePath);
    std::string baseName = getBaseName(basePath);

    DIR* dp = opendir(dir.c_str());
    if (!dp) {
        return;
    }

    std::vector<std::pair<time_t, std::string>> archives;
    struct dirent* entry;
    while ((entry = readdir(dp)) != nullptr) {
        if (entry->d_type != DT_REG && entry->d_type != DT_LNK && entry->d_type != DT_UNKNOWN) {
            continue;
        }

        std::string name = entry->d_name;
        if (name.find(baseName + ".") != 0) {
            continue;
        }

        std::string fullPath = dir + "/" + name;
        struct stat st;
        if (stat(fullPath.c_str(), &st) == 0) {
            archives.push_back(std::make_pair(st.st_mtime, fullPath));
        }
    }
    closedir(dp);

    if (archives.size() <= retentionCount) {
        return;
    }

    std::sort(archives.begin(), archives.end(),
              [](const std::pair<time_t, std::string>& a, const std::pair<time_t, std::string>& b) {
                  return a.first > b.first;
              });

    for (std::size_t i = retentionCount; i < archives.size(); ++i) {
        unlink(archives[i].second.c_str());
    }
}

void Tintin_reporter::log(LogLevel level, const std::string& message) {
    logSystem(level, message);
}

void Tintin_reporter::info(const std::string& message) {
    logSystem(INFO, message);
}

void Tintin_reporter::warning(const std::string& message) {
    logSystem(WARNING, message);
}

void Tintin_reporter::error(const std::string& message) {
    logSystem(ERROR, message);
}

void Tintin_reporter::debug(const std::string& message) {
    logSystem(DEBUG, message);
}

void Tintin_reporter::logUserInput(const std::string& input) {
    logCommand("Matt_daemon: User input: " + input);
}

void Tintin_reporter::userLog(const std::string& message) {
    logCommand(message);
}

void Tintin_reporter::logSystem(LogLevel level, const std::string& message) {
    if (level < currentLevel) {
        return;
    }

    std::ostringstream oss;
    oss << "[" << formatTimestamp() << "] "
        << "[ " << levelToString(level) << " ] "
        << "- " << message;

    writeToFile(mainLog, oss.str());
    writeToFile(systemLog, oss.str());
}

void Tintin_reporter::logAudit(LogLevel level, const std::string& message) {
    if (level < currentLevel) {
        return;
    }

    std::ostringstream oss;
    oss << "[" << formatTimestamp() << "] "
        << "[ " << levelToString(level) << " ] "
        << "- " << message;

    writeToFile(mainLog, oss.str());
    writeToFile(auditLog, oss.str());
}

void Tintin_reporter::logCommand(const std::string& message) {
    std::ostringstream oss;
    oss << "[" << formatTimestamp() << "] "
        << "[ LOG ] "
        << "- " << message;

    writeToFile(mainLog, oss.str());
    writeToFile(commandsLog, oss.str());
}

void Tintin_reporter::setLogLevel(LogLevel level) {
    currentLevel = level;
}

void Tintin_reporter::setLogFile(const std::string& filePath) {
    initialize(filePath);
}

void Tintin_reporter::setRotationPolicy(std::size_t maxBytes, std::size_t retentionCount, bool gzipArchives) {
    this->maxBytes = maxBytes;
    this->retentionCount = retentionCount;
    this->gzipArchives = gzipArchives;
}
