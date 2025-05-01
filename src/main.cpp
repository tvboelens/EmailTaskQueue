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

    json credentials;

    const char *smtp_user = std::getenv("SMTP_USER");
    if (smtp_user == nullptr)
    {
        spdlog::error("SMTP_USER not defined");
        return 1;
    }
    const char *smtp_password = std::getenv("SMTP_PW");
    if (smtp_password == nullptr)
    {
        spdlog::error("SMTP_PW not defined");
        return 1;
    }
    const char *smtp_server = std::getenv("SMTP_SERVER");
    if (smtp_server == nullptr)
    {
        spdlog::error("SMTP_SERVER not defined");
        return 1;
    }

    credentials["smtp_user"] = smtp_user;
    credentials["smtp_password"] = smtp_password;
    credentials["smtp_server"] = smtp_server;

    // Crow web app
    crow::SimpleApp app;

    CROW_ROUTE(app, "/submit_email").methods("POST"_method)([](const crow::request &req)
                                                            {
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
        return crow::response(200, "Email task submitted successfully"); });

    // Register the Queueable (sub)classes
    QueueableRegistry registry;
    //    QueueableFactory factory = []()
    //   { return std::make_unique<Queueable>(); };
    //    registry.registerQueueable("Queueable", factory);

        QueueableFactory logfactory = []()
        { return std::make_unique<LogQueueable>(); };
        RetryStrategy log_retry_strategy{-1, 4, std::nullopt, nullptr};
        registry.registerQueueable("LogQueueable", logfactory);
        registry.registerRetryStrategy("LogQueueable", log_retry_strategy);
        QueueableFactory emailfactory = []()
        { return std::make_unique<SendEmail>(); };
        registry.registerQueueable("SendEmail", emailfactory);
        // sleep(30);

        json args = {{"name", "some_name"}, {"task", "email"}, {"recipient", "user@example.com"}};
        // Queueable q;
        LogQueueable lq;
        for (int i = 0; i <= 20; ++i)
        {
            std::string job_no = std::to_string(i);
            args = {{"job_number", job_no}};
            lq.dispatch(args);
        }
    //q.dispatch(args);
    //q.dispatch(args);
    //q.dispatch(args); 

    Worker w1(registry, credentials);
    Worker w2(registry, credentials);

    std::thread workerThread1(&Worker::run, &w1);
    std::thread workerThread2(&Worker::run, &w2);

    app.port(8080).multithreaded().run();
    // The app will run until stopped
    spdlog::info("Stopping application...");
    // Set stopWorkers flag to true so that workers will exit the loop in the run() method
    stopWorkers = true;

    workerThread1.join();
    workerThread2.join();

    spdlog::info("Application exited cleanly");
    return 0;
}
