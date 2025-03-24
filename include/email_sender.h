#ifndef EMAIL_SENDER_H
#define EMAIL_SENDER_H

#include <string>
#include "queueable.h"

class SendEmail: public Queueable
{
private:
    /* data */
public:
    SendEmail(/* args */);
    ~SendEmail();
    // Function to send an email using libcurl
    void send_email(const std::string &from_email,
                    const std::string &to_email,
                    const std::string &subject,
                    const std::string &body,
                    const std::string &smtp_server,
                    const std::string &smtp_user,
                    const std::string &smtp_password);
    void dispatch(const json &args);
    void handle(const json &args) override;
};








#endif // EMAIL_SENDER_H
