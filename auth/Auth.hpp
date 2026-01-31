#ifndef AUTH_HPP
#define AUTH_HPP

#include <string>

class Auth {
public:
    Auth();
    ~Auth();
    
    bool authenticate(const std::string& credentials);
    bool isAuthorized(const std::string& user);
    
private:
    // TODO: Add authentication logic
};

#endif // AUTH_HPP
