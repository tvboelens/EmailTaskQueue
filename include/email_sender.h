#ifndef EMAIL_SENDER_H
#define EMAIL_SENDER_H

#include <string>
#include "queueable.h"

class SendEmail: public Queueable
{
private:
public:
    SendEmail();
    ~SendEmail();
    // Function to send an email using libcurl
    void send_email(const json &args, const json &credentials);
    void dispatch(const json &args);
    void handle(const json &args, const json &credentials);
};








#endif // EMAIL_SENDER_H
