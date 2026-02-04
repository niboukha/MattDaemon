#include "Server.hpp"


Server::Server() : server_fd(-1) {
    Tintin_reporter::getInstance().info("Matt_daemon: Server instance created");
}

Server::~Server() {
    stop();
}

void Server::HandelClient(Client& client, int client_index) {
    (void)client_index; // Unused parameter, can be removed if not needed later
    CommandExecutor shell;
    size_t pos;
    while ((pos = client.getBuffer().find('\n')) != std::string::npos) {
        printf("------------> ut from client %d\n", client.getFd());
        std::string line = client.getBuffer().substr(0, pos);
        std::string remaining = client.getBuffer().substr(pos + 1);  // Save before clearing
        
        client.clearBuffer();
        client.appendToBuffer(remaining);
        // Remove carriage return
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        
        printf("Client %d [state=%d]: '%s'\n", client.getFd(), client.getState(), line.c_str());
        
        switch (client.getState()) {
            case STATE_USERNAME:
                if (line.empty()) {
                    sendToClient(client.getFd(), "ERROR: Username cannot be empty!\nUSERNAME: ");
                } else {
                    client.setUsername(line);
                    sendToClient(client.getFd(), "PASSWORD: ");
                    client.setState(STATE_PASSWORD);
                }
                break;
                
            case STATE_PASSWORD:
                if (line.empty()) {
                    sendToClient(client.getFd(), "ERROR: Password cannot be empty!\nPASSWORD: ");
                } else if (client.Authenticate(client.getUsername(), line)) {
                    const char *success = "Authentication successful!\nMatt-daemon> ";
                    sendToClient(client.getFd(), success);
                    client.setAuthenticated(true);
                    client.setState(STATE_READY);
                } else {
                    const char *fail = "Authentication failed!\nUSERNAME: ";
                    sendToClient(client.getFd(), fail);
                    client.setState(STATE_USERNAME);
                }
                break;
            
            case STATE_READY:
                if (line == "quit" || line == "exit") {
                    sendToClient(client.getFd(), "Goodbye!\n");
                    close(client.getFd());
                    removeClient(client.getFd());
                    // clients.erase(clients.begin() + client_index);
                    client_index--;
                } else if (!line.empty()) {
                    std::string result = shell.executeCommand(line);
                    result += "\nMatt-daemon> ";
                    sendToClient(client.getFd(), result);
                } else {
                    sendToClient(client.getFd(), "Matt-daemon> ");
                }
                break;
        }
    }
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
        Tintin_reporter::getInstance().error("Matt_daemon: select failed");
        return true;
    }

    if (FD_ISSET(server_fd, &read_fds)) {
        acceptConnection();
    }

    // std::vector<Client> clients_snapshot = clients;
    for ( size_t i = 0; i < clients.size(); ++i) {
        Client& client = clients[i];
        int client_fd = client.getFd();
        if (FD_ISSET(client_fd, &read_fds)) {
            std::string message = receiveFromClient(client_fd);
            if (message.empty()) {
                    removeClient(client_fd);
                    i--; // Adjust index after removal
                } else {
                printf("Received from client %d: %s", client_fd, message.c_str());
                client.appendToBuffer(message);
                HandelClient(client, i);
                // std::stringstream ss(message);
                // std::string line;
                // while (std::getline(ss, line)) {
                //     if (line.empty()) {
                //         continue;
                //     }

                //     CommandParser parser;
                //     if (parser.parse(line) == 0 && parser.getCommand() == "quit") {
                //         Tintin_reporter::getInstance().info("Matt_daemon: Request quit.");
                //         return false;
                //     }

                //     Tintin_reporter::getInstance().userLog("Matt_daemon: User input: " + line);
                // }
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
    
    clients.erase(std::remove_if(clients.begin(), clients.end(), [client_fd](const Client& client) {
        return client.getFd() == client_fd;
    }), clients.end());
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
    
    clients.push_back(Client(client_fd));
    
    // Log client connection with FD number
    // std::ostringstream oss;
    // oss << "Client #" << client_fd << " connected";
    // Tintin_reporter::getInstance().info(oss.str());
    
    // Send a welcoming message to the client
    const char *welcome_msg = "Welcome to Matt_daemon!\nUsername: ";
    send(client_fd, welcome_msg, strlen(welcome_msg), MSG_NOSIGNAL);

    return client_fd;
}

int Server::getServerFd() const {
    return server_fd;
}

const std::vector<Client>& Server::getClients() const {
    return clients;
}
