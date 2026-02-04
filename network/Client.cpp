#include "Client.h"

Client::Client(/* args */) : fd(-1), state(STATE_USERNAME), authenticated(false) {}
Client::Client(int fd) : fd(fd), state(STATE_USERNAME), authenticated(false) {}
Client::~Client() {}

Client::Client(const Client &other) {
    fd = other.fd;
    state = other.state;
    username = other.username;
    buffer = other.buffer;
    authenticated = other.authenticated;
    failedCount = other.failedCount;
    firstFail = other.firstFail;
}

Client& Client::operator=(const Client &other) {
    if (this != &other) {
        fd = other.fd;
        state = other.state;
        username = other.username;
        buffer = other.buffer;
        authenticated = other.authenticated;
        failedCount = other.failedCount;
        firstFail = other.firstFail;
    }
    return *this;
}

int Client::getFd() const {
    return fd;
}

void Client::setFd(int fd) {
    this->fd = fd;
}

ClientState Client::getState() const {
    return state;
}

void Client::setState(ClientState state) {
    this->state = state;
}
std::string Client::getUsername() const {
    return username;
}
void Client::setUsername(const std::string &username) {
    this->username = username;
}

std::string Client::getBuffer() const {
    return buffer;
}
void Client::appendToBuffer(const std::string &data) {
    buffer += data;
}
void Client::clearBuffer() {
    buffer.clear();
}

bool Client::isAuthenticated() const {
    return authenticated;
}

void Client::setAuthenticated(bool auth) {
    this->authenticated = auth;
}

bool Client::Authenticate(const std::string& username, const std::string &password) {
    // Simple authentication logic (for demonstration purposes)
    if (username == USERNAME && password == PASSWORD) {      
        authenticated = true;
        return true;
    } else {
        time_t now = time(nullptr);

        if (failedCount == 0)
            firstFail = now;

        failedCount++;

        if (failedCount >= 5 && (now - firstFail) <= 60)
        {
            MailService().sendEmail("ALERT: 5 failed authentication attempts!",
                  "There have been 5 failed authentication attempts within the last minute.");
            failedCount = 0;
        }

        return false;
    }
}