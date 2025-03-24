#include <iostream>
#include <fstream>
#include <sqlite3.h>
#include <nlohmann/json.hpp>
#include "../include/email_sender.h"
#include "../include/worker.h"
#include "../include/queueable.h"
#include <spdlog/spdlog.h>
#include <chrono>
#include <thread>
#include <crow.h>

using json = nlohmann::json;

bool createJobsTable(sqlite3 *db)
{
    // Drop the existing table if it exists
    const char *sql = "DROP TABLE IF EXISTS jobs";
    sqlite3_stmt *stmt;
    char *errMsg = nullptr;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK)
    {
        spdlog::error("Failed to prepare statement: {}", sqlite3_errmsg(db));
        sqlite3_close(db);
        return false;
    }
    if (sqlite3_step(stmt) != SQLITE_DONE)
    {
        spdlog::error("Failed to drop jobs table: {}", sqlite3_errmsg(db));
    }
    else
    {
        spdlog::info("Job saved to database: {}");
    }
    
    // Create the new jobs table
    sql= R"(
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

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK)
    {
        spdlog::error("Failed to prepare statement: {}", sqlite3_errmsg(db));
        sqlite3_close(db);
        return false;
    }
    if (sqlite3_step(stmt) != SQLITE_DONE)
    {
        spdlog::error("Failed to create jobs table: {}", sqlite3_errmsg(db));
    }
    else
    {
        spdlog::info("Table 'jobs' created successfully!");
    }
    return true;
}

int main()
{
    
    
    // Open an SQLite database (or create it if it doesn't exist)
    sqlite3 *db;

    if (sqlite3_open("database.db", &db) != SQLITE_OK)
    {
        spdlog::error("Failed to open database.");
        return 1;
    }
    else
    {// Create the jobs table
        if (createJobsTable(db)==false){
        return 1;}
    }

    crow::SimpleApp app;

    CROW_ROUTE(app, "/submit_email").methods("POST"_method)
    ([](const crow::request &req){
        // Parse the JSON body of the POST request
        auto json_data = json::parse(req.body);

        std::string recipient = json_data["recipient"];
        std::string subject = json_data["subject"];
        std::string body = json_data["body"];

        if(recipient.empty())
        {
            spdlog::error("Missing recipient");
            return crow::response(400, "Missing recipient");
        }
        
        if(subject.empty())
        {
            spdlog::error("Missing subject");
            return crow::response(400, "Missing subject");
        }

        if(body.empty())
        {
            spdlog::error("Missing body");
            return crow::response(400, "Missing body");
        }

        SendEmail q;
        q.dispatch(json_data);
        // Send a response
        return crow::response(200, "Email task submitted successfully"); 
    });


    QueueableFactory factory = []()
    { return std::make_unique<Queueable>(); };
    QueueableRegistry registry;
    QueueableFactory logfactory = []()
    { return std::make_unique<LogQueueable>(); };
    QueueableFactory emailfactory = []() { return std::make_unique<SendEmail>() ; };
    registry.registerQueueable("Queueable", factory);
    registry.registerQueueable("LogQueueable", logfactory);
    registry.registerQueueable("SendEmail", emailfactory);
    // sleep(30);

    /* json args = {{"name", "some_name"}, {"task", "email"}, {"recipient", "user@example.com"}};
    // Queueable q;
    LogQueueable lq;
    for (int i = 0; i <= 5; ++i)
    {
        std::string job_no = std::to_string(i);
        args = {{"job_number", job_no}};
        lq.dispatch(args);
    }
    //q.dispatch(args);
    //q.dispatch(args);
    //q.dispatch(args); */

    Worker w1(registry);
    Worker w2(registry);

    std::thread workerThread1(&Worker::run, &w1);
    std::thread workerThread2(&Worker::run, &w2);

    app.port(8080).multithreaded().run();

    workerThread1.join();
    workerThread2.join();

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
