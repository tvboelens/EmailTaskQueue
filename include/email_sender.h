#ifndef EMAIL_SENDER_H
#define EMAIL_SENDER_H

#include <string>

// Function to send an email using libcurl
void send_email(const std::string& from_email,
                const std::string& to_email,
                const std::string& subject,
                const std::string& body,
                const std::string& smtp_server,
                const std::string& smtp_user,
                const std::string& smtp_password);

#endif // EMAIL_SENDER_H
