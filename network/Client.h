#ifndef CLIENT_H
#define CLIENT_H

#include <string>
#include <ctime>
#include "../mailService/mailService.h"
#include "../util/Tintin_reporter.hpp"

#define USERNAME "admin"
#define PASSWORD "1234"

enum ClientState {
    STATE_USERNAME,
    STATE_PASSWORD,
    STATE_READY
};

class Client
{
    private:
        int         fd;
        ClientState state;
        std::string username;
        std::string buffer;
        bool        authenticated;
        int         failedCount = 0;
        time_t      firstFail = 0;

    public:
        Client();
        Client(const Client &) ;
        Client& operator=(const Client &);
        ~Client();
        
        Client(int fd);

        int         getFd() const;
        ClientState getState() const;
        std::string getUsername() const;
        std::string getBuffer() const;

        void        setFd(int fd);
        void        setState(ClientState state);
        void        setUsername(const std::string &username);
        void        setAuthenticated(bool auth);
       
        void        appendToBuffer(const std::string &data);
        void        clearBuffer();
        
        bool        isAuthenticated() const;
        bool        Authenticate(const std::string& username, const std::string &password);
};


#endif
