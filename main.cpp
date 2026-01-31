#include "daemon/Daemon.hpp"
#include "util/Tintin_reporter.hpp"
#include <unistd.h>
#include <iostream>
#include <cstdlib>

int main() {
    // Check if running as root
    if (geteuid() != 0) {
        std::cerr << "Error: This program must be run as root" << std::endl;
        return 1;
    }
    
    // Initialize logger
    Tintin_reporter::getInstance().initialize("/var/log/matt_daemon/matt_daemon.log");
    
    Daemon daemon;
    return daemon.run();
}

