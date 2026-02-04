#ifndef CLIENT_H
#define CLIENT_H
#include <string>
#include <ctime>
#include "../mailService/mailService.h"

enum ClientState {
    STATE_USERNAME,
    STATE_PASSWORD,
    STATE_READY
};

#define USERNAME "admin"
#define PASSWORD "1234"

class Client
{
    private:
        int fd;
        ClientState state;
        std::string username;
        std::string buffer;
        bool authenticated;
        int failedCount = 0;
        time_t firstFail = 0;
    public:
        Client(/* args */);
        ~Client();
        Client(int fd);
        Client(const Client &) ;
        Client& operator=(const Client &);


        int getFd() const;
        void setFd(int fd);
        ClientState getState() const;
        void setState(ClientState state);
        std::string getUsername() const;
        void setUsername(const std::string &username);
        std::string getBuffer() const;
        void appendToBuffer(const std::string &data);
        void clearBuffer();
        bool isAuthenticated() const;
        void setAuthenticated(bool auth);
        bool Authenticate(const std::string& username, const std::string &password);
};


#endif // CLIENT_H