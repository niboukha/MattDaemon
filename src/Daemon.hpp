#ifndef DAEMON_HPP
#define DAEMON_HPP

#include <string>

class Daemon {
public:
    Daemon(const std::string& lockFile = "/var/lock/matt_daemon.lock");
    ~Daemon();
    
    void daemonize();
    void run();
    
private:
    std::string lockFile_;
    int lockFd_;
    
    void createLockFile();
    void removeLockFile();
    void setupSignalHandlers();
};

#endif // DAEMON_HPP
