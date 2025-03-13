#include <iostream>
#include <fstream>
#include <soci/soci.h>
#include <soci/sqlite3/soci-sqlite3.h>
#include <nlohmann/json.hpp>
#include "../include/email_sender.h"
#include "../include/worker.h"
#include "../include/queueable.h"
#include <spdlog/spdlog.h>

using json = nlohmann::json;

void createJobsTable(soci::session &db)
{
    // Drop the existing table if it exists
    db << "DROP TABLE IF EXISTS jobs";

    // Create the new jobs table
    db << R"(
        CREATE TABLE jobs (
            id TEXT PRIMARY KEY,               -- Unique job ID (string)
            name TEXT NOT NULL,                -- Job name
            args TEXT NOT NULL,                -- JSON-encoded arguments
            queue TEXT DEFAULT 'default',      -- Job queue
            created_at DATETIME DEFAULT CURRENT_TIMESTAMP, -- Creation timestamp
            next_execution_at DATETIME,        -- When to execute next (nullable)
            last_executed_at DATETIME,         -- Last execution timestamp (nullable)
            attempts INTEGER DEFAULT 0,        -- Number of retry attempts
            state TEXT DEFAULT 'waiting',      -- Job state
            error_details TEXT,                -- Error message if failed
            reserved_by TEXT                   -- Worker ID processing this job
        )
    )";

    spdlog::info("Table 'jobs' created successfully!");
}

int main()
{
    try
    {
        // Open an SQLite database (or create it if it doesn't exist)
        soci::session db(soci::sqlite3, "database.db");

        // Create the jobs table
        createJobsTable(db);
    }
    catch (const std::exception &e)
    {
        spdlog::error("Database error: {}", e.what());
    }

    json args = {{"name", "some_name"},{"task", "email"}, {"recipient", "user@example.com"}};
    Queueable q;
    q.dispatch(args);

    Worker w;
    w.run();

    /*
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
    */

    return 0;
}
