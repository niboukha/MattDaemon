#ifndef SERVER_HPP
#define SERVER_HPP

#include <vector>
#include <string>
#include <sys/select.h>

#define PORT 4242
#define MAX_CLIENTS 3

class Server {
private:
    std::vector<int>    clients;
    int                 server_fd;

public:
    Server();
    Server(const Server&) = delete;
    Server& operator=(const Server&) = delete;
    ~Server();
    
    int     start();
    void    stop();
    bool    handleConnections();
    
    std::string receiveFromClient(int client_fd);
    int         sendToClient(int client_fd, const std::string& message);
    void        removeClient(int client_fd);
    int         acceptConnection();
    
    int                     getServerFd() const;
    const std::vector<int>& getClients() const;
};

#endif
