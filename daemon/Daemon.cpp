#include "Daemon.hpp"
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <iostream>
#include <fstream>
#include <cstring>
#include <sstream>
#include <sys/file.h>

Daemon::Daemon() : server(), lockFilePath("/var/lock/matt_daemon.lock"), lockFd(-1), running(false) {}

Daemon::~Daemon() {
    stop();
    removeLockFile();
}

bool Daemon::createLockFile() {
    lockFd = open(lockFilePath.c_str(), O_RDWR | O_CREAT, 0644);
    if (lockFd < 0) {
        Tintin_reporter::getInstance().error("Matt_Daemon: Unable to open lock file: " + lockFilePath);
        return false;
    }

    if (flock(lockFd, LOCK_EX | LOCK_NB) < 0) {
        Tintin_reporter::getInstance().error("Matt_Daemon: Unable to lock file: " + lockFilePath);
        close(lockFd);
        lockFd = -1;
        return false;
    }

    ftruncate(lockFd, 0);
    std::string pidStr = std::to_string(getpid());
    write(lockFd, pidStr.c_str(), pidStr.size());
    return true;
}

void Daemon::removeLockFile() {
    if (lockFd != -1) {
        flock(lockFd, LOCK_UN);
        close(lockFd);
        unlink(lockFilePath.c_str());
        lockFd = -1;
    }
}

int Daemon::run() {
    Tintin_reporter& logger = Tintin_reporter::getInstance();
    
    logger.info("Matt_daemon: Started.");
    logger.info("Matt_daemon: Creating server.");
    
    if (!createLockFile()) {
        std::cerr << "Error: Matt_daemon instance already running" << std::endl;
        Tintin_reporter::getInstance().error("Matt_daemon: Quitting.");
        return 1;
    }
    
    logger.info("Matt_daemon: Server created.");

    // setupSignalHandlers();
    logger.info("Matt_daemon: Entering Daemon mode.");
    daemonize();
    
    logger.initialize("/var/log/matt_daemon/matt_daemon.log");
    logger.info("Matt_daemon: started. PID: " + std::to_string(getpid()));
    
    if (server.start() < 0) {
        logger.error("Matt_daemon: Failed to start server");
        return 1;
    }
    
    running = true;
    while (running) {
        if (!server.handleConnections()) {
            running = false;
        }
    }
    
    logger.info("Matt_daemon: Quitting.");
    return 0;
}

void Daemon::stop() {
    running = false;
    server.stop();
}

void Daemon::daemonize() {
    pid_t pid = fork();
    if (pid < 0) {
        Tintin_reporter::getInstance().error("Matt_Daemon: First fork failed");
        exit(EXIT_FAILURE);
    }
    if (pid > 0) {
        exit(EXIT_SUCCESS);
    }

    if (setsid() < 0) {
        Tintin_reporter::getInstance().error("Matt_Daemon: create new session failed");
        exit(EXIT_FAILURE);
    }

    pid = fork();
    if (pid < 0) {
        Tintin_reporter::getInstance().error("Matt_Daemon: Second fork failed");
        exit(EXIT_FAILURE);
    }
    if (pid > 0) {
        exit(EXIT_SUCCESS);
    }

    umask(0);
    chdir("/");

    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    int devNull = open("/dev/null", O_RDWR);
    if (devNull != -1) {
        dup2(devNull, STDIN_FILENO);
        dup2(devNull, STDOUT_FILENO);
        dup2(devNull, STDERR_FILENO);
        if (devNull > 2) close(devNull);
    }
}
