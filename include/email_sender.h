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
    void dispatch(const json &args, double wait = 0, std::optional<std::chrono::system_clock::time_point> at = std::nullopt);
    void handle(const json &args, std::optional<json> credentials) override;
};








#endif // EMAIL_SENDER_H
