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

static Daemon* g_daemon_instance = nullptr;

Daemon::Daemon() : server(), lockFilePath("/var/lock/matt_daemon.lock"), lockFd(-1), running(false) {
    g_daemon_instance = this;
}

Daemon::~Daemon() {
    stop();
    removeLockFile();
}

bool Daemon::createLockFile() {
    lockFd = open(lockFilePath.c_str(), O_RDWR | O_CREAT, 0644);
    if (lockFd < 0) {
        // Tintin_reporter::getInstance().error("Matt_Daemon: Unable to open lock file: " + lockFilePath);
        return false;
    }

    if (flock(lockFd, LOCK_EX | LOCK_NB) < 0) {
        // Tintin_reporter::getInstance().error("Matt_Daemon: Unable to lock file: " + lockFilePath);
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

static void signal_handler(int signum) {
    if (g_daemon_instance) {
        g_daemon_instance->handleSignal(signum);
    }
}

void Daemon::setupSignalHandlers(){
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    signal(SIGHUP, signal_handler);
    signal(SIGABRT, signal_handler);
    signal(SIGSEGV, signal_handler);
}

void Daemon::handleSignal(int signum) {
    std::string sigName;
    switch(signum) {
        case SIGINT:
            sigName = "SIGINT";
            break;
        case SIGTERM:
            sigName = "SIGTERM";
            break;
        case SIGHUP:
            sigName = "SIGHUP";
            break;
        case SIGABRT:
            sigName = "SIGABRT";
            break;
        case SIGSEGV:
            sigName = "SIGSEGV";
            break;
        default:
            sigName = "UNKNOWN";
            break;
    }
    
    Tintin_reporter::getInstance().info("Matt_daemon: Signal handler: " + sigName + " received.");
    stop();
}

int Daemon::run() {
    Tintin_reporter& logger = Tintin_reporter::getInstance();
    
    logger.info("Matt_daemon: Started.");

    if (!createLockFile()) {
        std::cerr << "Can't open :/var/lock/matt_daemon.lock" << std::endl;
        logger.error("Matt_daemon: Error file locked.");
        logger.info("Matt_daemon: Quitting.");
        return 1;
    }
    
    logger.info("Matt_daemon: Entering Daemon mode.");
    daemonize();
    
    logger.initialize("/var/log/matt_daemon/matt_daemon.log");
    
    setupSignalHandlers();
    logger.info("Matt_daemon: started. PID: " + std::to_string(getpid()));
    
    logger.info("Matt_daemon: Creating server.");
    if (server.start() < 0) {
        logger.error("Matt_daemon: Failed to start server");
        return 1;
    }
    logger.info("Matt_daemon: Server created.");
    
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
    g_daemon_instance = nullptr;
    running = false;
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
