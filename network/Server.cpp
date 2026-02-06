#include "Server.hpp"


namespace {
    std::string trim_cmd(std::string& input) {
        size_t start = 0;
        while (start < input.size() && std::isspace(static_cast<unsigned char>(input[start]))) {
            ++start;
        }

        size_t end = input.size();
        while (end > start && std::isspace(static_cast<unsigned char>(input[end - 1]))) {
            --end;
        }

        return input.substr(start, end - start);
    }
}

Server::Server() : server_fd(-1) {
    Tintin_reporter::getInstance().logSystem(INFO, "Matt_daemon: Server instance created");
}

Server::~Server() {
    stop();
}

bool Server::HandleClient(Client& client, size_t &idx) {
    CommandExecutor shell;
    size_t          pos;

    while ((pos = client.getBuffer().find('\n')) != std::string::npos) {
        std::string line = client.getBuffer().substr(0, pos);
        std::string remaining = client.getBuffer().substr(pos + 1);
        
        client.clearBuffer();
        client.appendToBuffer(remaining);

        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
                
        switch (client.getState()) {
            case STATE_USERNAME:
                line = trim_cmd(line);
                if (line.empty()) {
                    Tintin_reporter::getInstance().logAudit(WARNING, "Matt_daemon: Client #" + std::to_string(client.getFd()) + " attempted to enter empty username");
                    sendToClient(client.getFd(), "ERROR: Username cannot be empty!\nUSERNAME: ");
                } else {
                    client.setUsername(line);
                    Tintin_reporter::getInstance().logAudit(INFO, "Matt_daemon: Client #" + std::to_string(client.getFd()) + " entered username: " + line);
                    sendToClient(client.getFd(), "PASSWORD: ");
                    client.setState(STATE_PASSWORD);
                }
                break;
                
            case STATE_PASSWORD:
                if (line.empty()) {
                    Tintin_reporter::getInstance().logAudit(WARNING, "Matt_daemon: Client #" + std::to_string(client.getFd()) + " attempted to enter empty password");
                } else if (client.Authenticate(client.getUsername(), line)) {
                    Tintin_reporter::getInstance().logAudit(INFO, "Matt_daemon: Client #" + std::to_string(client.getFd()) + " authenticated successfully");

                    const char *success = "Authentication successful!\nMatt-daemon> ";
                    sendToClient(client.getFd(), success);
                    client.setAuthenticated(true);
                    client.setState(STATE_READY);
                } else {
                    const char *fail = "Authentication failed!\nUSERNAME: ";
                    Tintin_reporter::getInstance().logAudit(ERROR, "Matt_daemon: Client #" + std::to_string(client.getFd()) + " failed authentication attempt");
                    sendToClient(client.getFd(), fail);
                    client.setState(STATE_USERNAME);
                }
                break;
            
            case STATE_READY:
                line = trim_cmd(line);
                if (line == "quit") {
                    Tintin_reporter::getInstance().logSystem(INFO, "Matt_daemon: Request quit.");
                    sendToClient(client.getFd(), "Goodbye!\n");
                    close(client.getFd());
                    removeClient(client.getFd());
                    idx--;
                    return false;
                } else if (!line.empty()) {
                    std::string result = shell.executeCommand(line);
                    Tintin_reporter::getInstance().logCommand("Matt_daemon: Client #" + std::to_string(client.getFd()) + " executed command: " + line);
                    result += "\nMatt-daemon> ";
                    sendToClient(client.getFd(), result);
                } else {
                    sendToClient(client.getFd(), "Matt-daemon> ");
                }
                break;
        }
    }
    return true;
}

int Server::start() {
    Tintin_reporter::getInstance().logSystem(INFO, "Matt_daemon: Starting server...");
    
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        Tintin_reporter::getInstance().logSystem(ERROR, "Unable to create socket");
        return -1;
    }

    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        Tintin_reporter::getInstance().logSystem(ERROR, "Unable to set SO_REUSEADDR");
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
        Tintin_reporter::getInstance().logSystem(ERROR, "Matt_daemon: Unable to bind socket");
        close(server_fd);
        server_fd = -1;
        return -1;
    }

    if (listen(server_fd, MAX_CLIENTS) < 0) {
        Tintin_reporter::getInstance().logSystem(ERROR, "Matt_daemon: Unable to listen on socket");
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
    
    Tintin_reporter::getInstance().logSystem(INFO, "Matt_daemon: Stopping server...");
    clients.clear();
    close(server_fd);
    server_fd = -1;
}

bool Server::handleConnections() {
    fd_set read_fds;
    FD_ZERO(&read_fds);

    FD_SET(server_fd, &read_fds);
    for (const Client& client : clients) FD_SET(client.getFd(), &read_fds);

    int max_fd = server_fd;
    for (const Client& client : clients) {
        int client_fd = client.getFd();
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
        Tintin_reporter::getInstance().logSystem(ERROR, "Matt_daemon: select failed");
        return true;
    }

    if (FD_ISSET(server_fd, &read_fds)) {
        acceptConnection();
    }

    for ( size_t i = 0; i < clients.size(); ++i) {
        Client& client = clients[i];
        int client_fd = client.getFd();
        if (FD_ISSET(client_fd, &read_fds)) {
            std::string message = receiveFromClient(client_fd);
            if (message.empty()) {
                    removeClient(client_fd);
                    i--;
                } else {
                    printf("Received from client %d: %s", client_fd, message.c_str());
                    client.appendToBuffer(message);
                    if (!HandleClient(client, i)) {
                        return false;
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
            Tintin_reporter::getInstance().logSystem(ERROR, oss.str());
        }
        return "";
    }
    buffer[bytes] = '\0';
    return std::string(buffer);
}

int Server::sendToClient(int client_fd, const std::string& message) {
    ssize_t bytes = send(client_fd, message.c_str(), message.size(), 0);
    if (bytes < 0) {
        std::ostringstream oss;
        oss << "Matt_daemon: send() failed to client #" << client_fd;
        Tintin_reporter::getInstance().logSystem(ERROR, oss.str());
        return -1;
    }
    return 0;
}

void Server::removeClient(int client_fd) {
    std::ostringstream oss;
    oss << "Matt_daemon: Client #" << client_fd << " disconnected";
    Tintin_reporter::getInstance().logSystem(INFO, oss.str());
    
    clients.erase(std::remove_if(clients.begin(), clients.end(), [client_fd](const Client& client) {
        return client.getFd() == client_fd;
    }), clients.end());

    close(client_fd);
}

void Server::acceptConnection() {
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    int client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
    
    if (client_fd < 0) {
        Tintin_reporter::getInstance().logSystem(ERROR, "Matt_daemon: Unable to accept new connection");
        return ;
    }

    if (clients.size() >= MAX_CLIENTS) {
        Tintin_reporter::getInstance().logSystem(WARNING, "Matt_daemon: Max clients reached, rejecting connection");
        const char *msg = "Server is full. Maximum clients reached, try again later.\n";
        //add send mail
        send(client_fd, msg, strlen(msg), MSG_NOSIGNAL);
        close(client_fd);
        return ;
    }
    
    clients.push_back(Client(client_fd));

    const char *welcome_msg = "Welcome to Matt_daemon!\nUSERNAME: ";
    send(client_fd, welcome_msg, strlen(welcome_msg), MSG_NOSIGNAL);
}
