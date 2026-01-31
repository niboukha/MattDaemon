#include "Client.hpp"
#include <unistd.h>
#include <sys/socket.h>

Client::Client() : client_fd(-1) {}

Client::~Client() {
    if (client_fd != -1) {
        close(client_fd);
    }
}

