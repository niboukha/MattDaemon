#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>

class Client {
    private:
        int client_fd;

    public:
        Client();
        ~Client();
};

#endif