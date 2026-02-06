#ifndef SERVER_HPP
#define SERVER_HPP

#include <vector>
#include <string>
#include <sys/select.h>
#include "Client.h"
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
#include "../CommandExecutor/CommandExecutor.h"

#define PORT 4242
#define MAX_CLIENTS 3

class Server {
    private:
        std::vector<Client> clients;
        int                 server_fd;

    public:
        Server();
        Server(const Server&) = delete;
        Server& operator=(const Server&) = delete;
        ~Server();

        void    HandleClient(Client& client, size_t &idx);
        
        int     start();
        void    stop();
        bool    handleConnections();
        
        std::string receiveFromClient(int client_fd);
        int         sendToClient(int client_fd, const std::string& message);
        void        removeClient(int client_fd);
        void        acceptConnection();
};

#endif
