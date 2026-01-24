#include "Daemon.hpp"
#include <iostream>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <signal.h>
#include <cstring>
#include <cerrno>

Daemon::Daemon(const std::string& lockFile) 
    : lockFile_(lockFile), lockFd_(-1) {
}

Daemon::~Daemon() {
    removeLockFile();
}

void Daemon::createLockFile() {
    lockFd_ = open(lockFile_.c_str(), O_RDWR | O_CREAT, 0640);
    if (lockFd_ < 0) {
        std::cerr << "Cannot create lock file: " << strerror(errno) << std::endl;
        exit(1);
    }
    
    if (lockf(lockFd_, F_TLOCK, 0) < 0) {
        std::cerr << "Cannot lock file. Daemon already running?" << std::endl;
        exit(1);
    }
}

void Daemon::removeLockFile() {
    if (lockFd_ >= 0) {
        close(lockFd_);
        unlink(lockFile_.c_str());
    }
}

void Daemon::setupSignalHandlers() {
    signal(SIGCHLD, SIG_IGN);
    signal(SIGHUP, SIG_IGN);
}

void Daemon::daemonize() {
    pid_t pid;
    
    // Fork off the parent process
    pid = fork();
    
    if (pid < 0) {
        exit(EXIT_FAILURE);
    }
    
    // Exit the parent process
    if (pid > 0) {
        exit(EXIT_SUCCESS);
    }
    
    // Create new session
    if (setsid() < 0) {
        exit(EXIT_FAILURE);
    }
    
    // Setup signal handlers
    setupSignalHandlers();
    
    // Fork again
    pid = fork();
    
    if (pid < 0) {
        exit(EXIT_FAILURE);
    }
    
    if (pid > 0) {
        exit(EXIT_SUCCESS);
    }
    
    // Set new file permissions
    umask(0);
    
    // Change working directory to root
    chdir("/");
    
    // Close all open file descriptors
    for (int x = sysconf(_SC_OPEN_MAX); x >= 0; x--) {
        close(x);
    }
    
    // Create lock file
    createLockFile();
}

void Daemon::run() {
    std::cout << "Matt_daemon started" << std::endl;
    
    while (true) {
        // Main daemon loop
        sleep(1);
    }
}
