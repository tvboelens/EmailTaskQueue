# EmailTaskQueue

## Overview

This project is a job queue system implemented in C++ that allows email sending tasks to be queued, stored in a database, and executed by worker processes. The system is designed to be lightweight and efficient, making use of database persistence for job storage, with plans to integrate Redis and multithreading in the future. In the future the job queue will be packaged in a Crow-based web application to process incoming requests.

## Features

- Queueable job system with unique job IDs
- Persistent storage of jobs in a database (currently using SQLite)
- Workers that process jobs (asynchronously in the future)

- Logging with spdlog for monitoring

- Platform-independent implementation

- Crow web application for handling job-related requests (in the future)

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

In the current version database configuration is done in `src/main.cpp`. Similarly, some dummy jobs are dispatched in `src/main.cpp` and a worker is started there as well. Change the code for other configurations.

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

Currently, jobs only log activity, but in the future, they will be used to send emails. There is an existing email-sending implementation in `src/email_sender.cpp`, which will be refined to fit within the job processing framework. (TODO: documentation for integration user credentials)

## Future Improvements


 - Introduce Redis-based queue management instead of storing jobs in an SQLite database

- Add multithreading for parallel job execution

- Add configurable retry logic for failed jobs

- Refine email-sending functionality to integrate seamlessly with job processing
- Implement a Crow app in order to handle web requests to enqueue jobs.

## License

This project is licensed under the MIT License.
