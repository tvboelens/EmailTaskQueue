# EmailTaskQueue

## Overview

This project is a job queue system implemented in C++ that allows email sending tasks to be queued, stored in a database, and executed by worker processes. The job queue is packaged in a Crow-based web application to process incoming requests. The system is designed to be lightweight and efficient, making use of database persistence for job storage, with plans to integrate Redis and multithreading in the future. 

## Features

- Queueable job system with unique job IDs
- Persistent storage of jobs in a database (currently using SQLite)
- Workers that process jobs (asynchronously in the future)

- Logging with spdlog for monitoring

- Platform-independent implementation

- Crow web application for handling job-related requests

## Dependencies

- C++17 or later

- CMake (for building the project)

- SQLite (current database backend)

- Redis (planned for future queue management)

- spdlog (for logging)
- [nlohmann_json](https://github.com/nlohmann/json) (for JSON parsing)

- libcurl (for email functionality)

- Crow (for handling web requests)

## Installation

### Building the Project

If you want to build using only CMake, runt the following commands:
``` 
mkdir build && cd build
cmake ..
make
```
If you want to use Ninja for faster building, run:
```
cmake -B build -GNinja .
ninja -C build
```
The target binary is then created in the `build/` folder.
### Usage

The app uses the SMTP protocol to send e-mails. Before running the following environment variables need to be set, so that the app has the right credentials.
```
SMTP_USER="your_email@example.com"
SMTP_SERVER="smtp.example.com"
SMTP_PW="your_password_or_app_specific_password"
```
In the current version database configuration is done in `src/main.cpp`. Similarly, the Crow app is configure to run at port 8080 and two workers are started. Change the code for other configurations.

#### Stopping the Application
Use either of the following two options:
- SIGINT (Ctrl+C): When you press Ctrl+C in the terminal, the system sends the SIGINT signal, which will trigger the handler and gracefully stop the server.
- SIGTERM: This signal can be sent using the `kill` command from another terminal to gracefully shut down the server.

Both signals are handled by the app to execute a clean exit.

### Job Structure

Each job consists of:

- id (Unique identifier)

- name (Job name)

- args (Serialized JSON arguments)

- queue (Queue category)

- created_at (Timestamp of creation)

- next_execution_at (Scheduled execution time, if applicable)

- last_executed_at (Timestamp of last execution)

- attempts (Retry count)

- state (Job state: waiting, running, failed, completed)

- error_details (If failed, logs error reason)

- reserved_by (Worker processing the job)

### Email Processing

This API allows users to submit an email task to the server via a POST request. The email task contains the recipient's email address, subject, and body of the email. The data is received in JSON format and processed by the server.

#### Endpoint
`POST /submit_email`

- URL: http://localhost:8080/submit_email
- Method: POST
- Content-Type: application/json

#### Request parameters
The request body must contain the following JSON fields:
- `recipient` (string, required): The recipient's email address.
- `subject` (string, required): The subject of the email.
- `body` (string, required): The body content of the email.

#### Example Request (Inline JSON)
```
curl -X POST http://localhost:8080/submit_email 
     -H "Content-Type: application/json" 
     -d '{"recipient":"test@example.com","subject":"Test Email","body":"This is a test email body."}'
```
#### Example Request (JSON File)

If you have the email data saved in a JSON file, you can send the request as follows:
```
curl -X POST http://localhost:8080/submit_email 
     -H "Content-Type: application/json" 
     -d @email.json
```

where `email.json` contains
```
{
  "recipient": "test@example.com",
  "subject": "Test Email",
  "body": "This is a test email body."
}
```

## Future Improvements


 - Introduce Redis-based queue management instead of storing jobs in an SQLite database

- Add multithreading for parallel job execution (e.g. thread pooling)

- Add configurable retry logic for failed jobs

## License

This project is licensed under the MIT License.
