#include "mailService.h"


MailService::~MailService() {}

MailService::MailService() {}


void MailService::sendEmail(const std::string &subject, const std::string &body) {
    std::ofstream emailFile("/tmp/email.txt");

    if (emailFile.is_open()) {
        emailFile << "Subject: " << subject << "\n";
        emailFile << "To: " << EMAIL_RECEIVER << "\n";
        emailFile << "From: " << EMAIL_SENDER << "\n\n";
        emailFile << body << "\n";
        emailFile.close();

        std::string command =  "curl --url \"smtps://smtp.gmail.com:465\" "
        "--ssl-reqd "
        "--mail-from " + std::string(EMAIL_SENDER) + " "
        "--mail-rcpt " + std::string(EMAIL_RECEIVER) + " "
        "--user \"" + std::string(EMAIL_SENDER) + ":" + std::string(APP_PASSWORD) + "\" "
        "-T /tmp/email.txt";
        system(command.c_str());
    } else {
        printf("Failed to open email file for writing.\n");
    }
}