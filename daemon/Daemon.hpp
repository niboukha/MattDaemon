#ifndef DAEMON_HPP
#define DAEMON_HPP

#include "../network/Server.hpp"
#include "../util/Tintin_reporter.hpp"
#include <string>

class Daemon {
    private:
        Server server;
        std::string lockFilePath;
        int lockFd;
        bool running;
        
        bool createLockFile();
        void removeLockFile();
        
        void setupSignalHandlers();
        
    public:
        Daemon();
        Daemon(const Daemon&) = delete;
        Daemon& operator=(const Daemon&) = delete;
        ~Daemon();
        
        void handleSignal(int signal);
        
        int run();
        void stop();
        void daemonize();
};

#endif

