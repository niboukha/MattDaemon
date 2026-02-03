#include "Server.hpp"
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <algorithm>
#include <sstream>
#include <errno.h>
#include "../util/Tintin_reporter.hpp"
#include "../parser/CommandParser.hpp"

Server::Server() : server_fd(-1) {
    Tintin_reporter::getInstance().info("Matt_daemon: Server instance created");
}

Server::~Server() {
    stop();
}

int Server::start() {
    Tintin_reporter::getInstance().info("Matt_daemon: Starting server...");
    
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        Tintin_reporter::getInstance().error("Unable to create socket");
        return -1;
    }

    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        Tintin_reporter::getInstance().error("Unable to set SO_REUSEADDR");
        close(server_fd);
        server_fd = -1;
        return -1;
    }

    struct sockaddr_in address;
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        Tintin_reporter::getInstance().error("Matt_daemon: Unable to bind socket");
        close(server_fd);
        server_fd = -1;
        return -1;
    }

    if (listen(server_fd, MAX_CLIENTS) < 0) {
        Tintin_reporter::getInstance().error("Matt_daemon: Unable to listen on socket");
        close(server_fd);
        server_fd = -1;
        return -1;
    }
    return 0;
}
 
void Server::stop() {
    if (server_fd == -1) {
        return;
    }
    
    Tintin_reporter::getInstance().info("Matt_daemon: Stopping server...");
    clients.clear();
    // Tintin_reporter::getInstance().info("Matt_daemon: Closing server socket");
    close(server_fd);
    server_fd = -1;
}

bool Server::handleConnections() {
    fd_set read_fds;
    FD_ZERO(&read_fds);

    FD_SET(server_fd, &read_fds);
    for (int client_fd : clients) FD_SET(client_fd, &read_fds);

    int max_fd = server_fd;
    for (int client_fd : clients) {
        if (client_fd > max_fd) max_fd = client_fd;
    }

    struct timeval timeout;
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;

    int activity = select(max_fd + 1, &read_fds, nullptr, nullptr, &timeout);
    if (activity < 0) {
        // EINTR means select() was interrupted by a signal (normal during shutdown)
        if (errno == EINTR) {
            return true;
        }
        Tintin_reporter::getInstance().error("Matt_daemon: select failed");
        return true;
    }

    if (FD_ISSET(server_fd, &read_fds)) {
        acceptConnection();
    }

    std::vector<int> clients_snapshot = clients;
    for (int client_fd : clients_snapshot) {
        if (FD_ISSET(client_fd, &read_fds)) {
            std::string message = receiveFromClient(client_fd);
            if (message.empty()) {
                removeClient(client_fd);
            } else {
                std::stringstream ss(message);
                std::string line;
                while (std::getline(ss, line)) {
                    if (line.empty()) {
                        continue;
                    }

                    CommandParser parser;
                    if (parser.parse(line) == 0 && parser.getCommand() == "quit") {
                        Tintin_reporter::getInstance().info("Matt_daemon: Request quit.");
                        return false;
                    }

                    Tintin_reporter::getInstance().userLog("Matt_daemon: User input: " + line);
                }
            }
        }
    }

    return true;
}

std::string Server::receiveFromClient(int client_fd) {
    char buffer[1024];
    ssize_t bytes = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
    if (bytes <= 0) {
        if (bytes < 0) {
            std::ostringstream oss;
            oss << "Matt_daemon: recv() error from client #" << client_fd;
            Tintin_reporter::getInstance().error(oss.str());
        }
        return "";
    }
    buffer[bytes] = '\0';
    return std::string(buffer);
}

// i will need it to send data to the clients 
int Server::sendToClient(int client_fd, const std::string& message) {
    ssize_t bytes = send(client_fd, message.c_str(), message.size(), 0);
    if (bytes < 0) {
        std::ostringstream oss;
        oss << "Matt_daemon: send() failed to client #" << client_fd;
        Tintin_reporter::getInstance().error(oss.str());
        return -1;
    }
    return 0;
}

void Server::removeClient(int client_fd) {
    std::ostringstream oss;
    oss << "Matt_daemon: Client #" << client_fd << " disconnected";
    Tintin_reporter::getInstance().info(oss.str());
    
    clients.erase(std::remove(clients.begin(), clients.end(), client_fd), clients.end());
    close(client_fd);
}

// keep it returning int for success/failure (in case I need it later)
int Server::acceptConnection() {
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    int client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
    
    if (client_fd < 0) {
        Tintin_reporter::getInstance().error("Matt_daemon: Unable to accept new connection");
        return -1;
    }

    if (clients.size() >= MAX_CLIENTS) {
        Tintin_reporter::getInstance().warning("Matt_daemon: Max clients reached, rejecting connection");
        const char *msg = "Server is full. Maximum clients reached, try again later.\n";
        send(client_fd, msg, strlen(msg), MSG_NOSIGNAL);
        close(client_fd);
        return -1;
    }
    
    clients.push_back(client_fd);
    
    // Log client connection with FD number
    // std::ostringstream oss;
    // oss << "Client #" << client_fd << " connected";
    // Tintin_reporter::getInstance().info(oss.str());
    
    return client_fd;
}

int Server::getServerFd() const {
    return server_fd;
}

const std::vector<int>& Server::getClients() const {
    return clients;
}
