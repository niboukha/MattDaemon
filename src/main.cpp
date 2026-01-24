#include "Daemon.hpp"
#include <iostream>

int main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;
    std::cout << "Starting Matt_daemon..." << std::endl;
    
    Daemon daemon;
    daemon.daemonize();
    daemon.run();
    
    return 0;
}
