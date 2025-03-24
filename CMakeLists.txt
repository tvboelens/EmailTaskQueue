cmake_minimum_required(VERSION 3.10)
project(EmailTaskQueue)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)

# include necessary directories
include_directories(/opt/homebrew/include)

# Find the required packages
find_package(CURL REQUIRED)
#find_package(Hiredis REQUIRED)
find_package(Threads REQUIRED)
find_package(fmt REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(SQLite3 REQUIRED)
find_package(Crow REQUIRED)

# Add source files
set(SOURCES
    src/main.cpp
    #src/redis_queue.cpp
    #src/email_sender.cpp
    src/worker.cpp
    src/queueable.cpp
    src/job.cpp
    src/randomhex.cpp
    src/chronotostring.cpp
)

# Define the executable
add_executable(email_task_queue ${SOURCES})

# Link libraries
target_link_libraries(email_task_queue PRIVATE
    CURL::libcurl
    #hiredis
    Threads::Threads
    fmt::fmt
    nlohmann_json::nlohmann_json
    SQLite::SQLite3
    Crow::Crow
)
