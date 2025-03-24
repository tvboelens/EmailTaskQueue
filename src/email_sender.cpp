#include "../include/email_sender.h"
#include <curl/curl.h>
//#include <iostream>
#include <spdlog/spdlog.h>

// Struct to track reading progress
struct UploadStatus
{
    size_t bytes_read;
    std::string *payload;
};

// Callback function for reading email content
size_t payload_source(void *ptr, size_t size, size_t nmemb, void *userp)
{
    UploadStatus *upload_ctx = static_cast<UploadStatus *>(userp);
    size_t max_size = size * nmemb;

    if (upload_ctx->bytes_read >= upload_ctx->payload->size())
    {
        return 0; // End of data
    }

    size_t to_copy = std::min(max_size, upload_ctx->payload->size() - upload_ctx->bytes_read);
    memcpy(ptr, upload_ctx->payload->c_str() + upload_ctx->bytes_read, to_copy);
    upload_ctx->bytes_read += to_copy;

    return to_copy;
}

SendEmail::SendEmail()
{
}

SendEmail::~SendEmail()
{
}

void SendEmail::dispatch(const json &args)
{
    Queueable::dispatch(args, "SendEmail");
}

void SendEmail::handle(const json &args, const json &credentials)
{
    std::string recipient{args["recipient"]};
    spdlog::info("Sending mail to {}...", recipient);
    send_email(args, credentials);
}

void SendEmail::send_email(const json &args, const json &credentials)
{
    const std::string from_email{credentials["smtp_user"]};
    const std::string smtp_server{credentials["smtp_server"]};
    const std::string smtp_user{credentials["smtp_user"]};
    const std::string smtp_password{credentials["smtp_password"]};
    const std::string to_email{args["subject"]};
    const std::string subject{args["subject"]};
    const std::string body{args["body"]};
    
    CURL *curl;
    CURLcode res;

    // Initialize curl
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();

    if (curl)
    {
        struct curl_slist *recipients = nullptr;

        // Set up the SMTP server settings
        curl_easy_setopt(curl, CURLOPT_URL, smtp_server.c_str());      // Set SMTP server
        curl_easy_setopt(curl, CURLOPT_MAIL_FROM, from_email.c_str()); // Set sender's email


        // Add recipient's email
        recipients = curl_slist_append(recipients, to_email.c_str());
        curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recipients);

        // Set the email body
        std::string email_data = "To: " + to_email + "\r\n" +
                                 "From: " + from_email + "\r\n" +
                                 "Subject: " + subject + "\r\n" +
                                 "\r\n" +
                                 body + "\r\n";

        // Create an UploadStatus struct to track data reading
        UploadStatus upload_ctx = {0, &email_data};

        // Set up reading function for email data
        curl_easy_setopt(curl, CURLOPT_READFUNCTION, payload_source);
        curl_easy_setopt(curl, CURLOPT_READDATA, &upload_ctx);
        curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);

        // Set the authentication for the SMTP server (username/password)
        curl_easy_setopt(curl, CURLOPT_USERNAME, smtp_user.c_str());
        curl_easy_setopt(curl, CURLOPT_PASSWORD, smtp_password.c_str());

        // Enable SSL/TLS (for secure connection)
        curl_easy_setopt(curl, CURLOPT_USE_SSL, (long)CURLUSESSL_ALL);

        spdlog::info("Sending email to {}", to_email.c_str());
        // std::cout << "Sending email to " << to_email.c_str() << std::endl;

        // Send the email
        res = curl_easy_perform(curl);

        // Check for errors
        if (res != CURLE_OK)
        {
            spdlog::error("Email sending failed: {}", curl_easy_strerror(res));
            // std::cerr << "Email sending failed: " << curl_easy_strerror(res) << std::endl;
        }
        else
        {
            spdlog::info("Email sent successfully!");
            // std::cout << "Email sent successfully!" << std::endl;
        }

        // Clean up
        curl_slist_free_all(recipients);
        curl_easy_cleanup(curl);
    }
    else
    {
        spdlog::error("Failed to initialize curl.");
        // std::cerr << "Failed to initialize curl." << std::endl;
    }

    // Cleanup global curl environment
    curl_global_cleanup();
}
