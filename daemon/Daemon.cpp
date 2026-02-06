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
        return false;
    }

    if (flock(lockFd, LOCK_EX | LOCK_NB) < 0) {
        close(lockFd);
        lockFd = -1;
        return false;
    }
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
    signal(SIGQUIT, signal_handler);
    signal(SIGILL, signal_handler);
    signal(SIGTRAP, signal_handler);
    signal(SIGABRT, signal_handler);
    signal(SIGFPE, signal_handler);
    signal(SIGBUS, signal_handler);
    signal(SIGSEGV, signal_handler);
    signal(SIGSYS, signal_handler);
    signal(SIGPIPE, signal_handler);
    signal(SIGALRM, signal_handler);
    signal(SIGURG, signal_handler);
    signal(SIGTSTP, signal_handler);
    signal(SIGCONT, signal_handler);
    signal(SIGCHLD, signal_handler);
    signal(SIGTTIN, signal_handler);
    signal(SIGTTOU, signal_handler);
    signal(SIGIO, signal_handler);
    signal(SIGXCPU, signal_handler);
    signal(SIGXFSZ, signal_handler);
    signal(SIGVTALRM, signal_handler);
    signal(SIGPROF, signal_handler);
    signal(SIGWINCH, signal_handler);
    signal(SIGUSR1, signal_handler);
    signal(SIGUSR2, signal_handler);
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
        case SIGQUIT:
            sigName = "SIGQUIT";
            break;
        case SIGILL:
            sigName = "SIGILL";
            break;
        case SIGTRAP:
            sigName = "SIGTRAP";
            break;
        case SIGABRT:
            sigName = "SIGABRT";
            break;
        case SIGFPE:
            sigName = "SIGFPE";
            break;
        case SIGBUS:
            sigName = "SIGBUS";
            break;
        case SIGSEGV:
            sigName = "SIGSEGV";
            break;
        case SIGSYS:
            sigName = "SIGSYS";
            break;
        case SIGPIPE:
            sigName = "SIGPIPE";
            break;
        case SIGALRM:
            sigName = "SIGALRM";
            break;
        case SIGURG:
            sigName = "SIGURG";
            break;
        case SIGTSTP:
            sigName = "SIGTSTP";
            break;
        case SIGCONT:
            sigName = "SIGCONT";
            break;
        case SIGCHLD:
            sigName = "SIGCHLD";
            break;
        case SIGTTIN:
            sigName = "SIGTTIN";
            break;
        case SIGTTOU:
            sigName = "SIGTTOU";
            break;
        case SIGIO:
            sigName = "SIGIO";
            break;
        case SIGXCPU:
            sigName = "SIGXCPU";
            break;
        case SIGXFSZ:
            sigName = "SIGXFSZ";
            break;
        case SIGVTALRM:
            sigName = "SIGVTALRM";
            break;
        case SIGPROF:
            sigName = "SIGPROF";
            break;
        case SIGWINCH:
            sigName = "SIGWINCH";
            break;
        case SIGUSR1:
            sigName = "SIGUSR1";
            break;
        case SIGUSR2:
            sigName = "SIGUSR2";
            break;
        default:
            sigName = "UNKNOWN";
            break;
    }
    
    Tintin_reporter::getInstance().info("Matt_daemon: Signal handler: " + sigName + " received.");
    stop();
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

    if (lockFd != -1) {
        ftruncate(lockFd, 0);
        std::string pidStr = std::to_string(getpid());
        write(lockFd, pidStr.c_str(), pidStr.size());
    }
    
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

