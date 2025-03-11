#include <iostream>
#include <fstream>
#include <nlohmann/json.hpp>
#include "../include/email_sender.h"

using json = nlohmann::json;

int main()
{
    // Load JSON data from the file
    std::ifstream input_file("../data/email_data.json");
    if (!input_file.is_open())
    {
        std::cerr << "Failed to open email_data.json file." << std::endl;
        return 1;
    }

    std::ifstream pw_input_file("../data/email_pw.json");
    if (!pw_input_file.is_open())
    {
        std::cerr << "Failed to open email_pw.json file." << std::endl;
        return 1;
    }

    // Parse the JSON data
    json email_data;
    input_file >> email_data;

    json email_pw;
    pw_input_file >> email_pw;

    // Extract the email details from the JSON file
    const std::string from_email = email_data["from_email"];
    const std::string to_email = email_data["to_email"];
    const std::string subject = email_data["subject"];
    const std::string body = email_data["body"];
    const std::string smtp_server = email_data["smtp_server"];
    const std::string smtp_user = email_data["smtp_user"];
    const std::string smtp_password = email_pw["smtp_password"];

    // Call the function to send an email
    send_email(from_email, to_email, subject, body, smtp_server, smtp_user, smtp_password);

    return 0;
}
