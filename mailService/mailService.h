#ifndef MAIL_SERVICE_H
#define MAIL_SERVICE_H
#include <string>
#include <cstdio>
#include <fstream>

#define  EMAIL_SENDER    "ms.souadhicham@gmail.com"
#define  EMAIL_RECEIVER  "ms.souadhicham@gmail.com"
#define  APP_PASSWORD     "gdts xckp pxgi irbl"

class MailService {
    public:
    ~MailService();
    MailService();
    MailService(const MailService&) = delete;
    MailService& operator=(const MailService&) = delete;
    
    void sendEmail(const std::string &subject, const std::string &body);
};
#endif // MAIL_SERVICE_H