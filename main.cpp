#include "daemon/Daemon.hpp"
#include "util/Tintin_reporter.hpp"
#include <unistd.h>
#include <iostream>
#include <cstdlib>

int main() {
    if (geteuid() != 0) {
        std::cerr << "Error: This program must be run as root" << std::endl;
        return 1;
    }
        
    Daemon daemon;
    return daemon.run();
}

    