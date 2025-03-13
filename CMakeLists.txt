cmake_minimum_required(VERSION 3.10)
project(EmailTaskQueue)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)

# include necessary directories
include_directories(/opt/homebrew/Cellar/nlohmann-json/3.11.3/include)

# Find the required packages
find_package(CURL REQUIRED)
#find_package(Hiredis REQUIRED)
find_package(Threads REQUIRED)
find_package(SOCI REQUIRED COMPONENTS core sqlite3)
find_package(fmt REQUIRED)
find_package(nlohmann_json REQUIRED)

# Add source files
set(SOURCES
    src/main.cpp
    #src/redis_queue.cpp
    #src/email_sender.cpp
    src/worker.cpp
    #src/queueable.cpp
    src/job.cpp
    src/randomhex.cpp
)

# Define the executable
add_executable(email_task_queue ${SOURCES})

# Link libraries
target_link_libraries(email_task_queue PRIVATE
    CURL::libcurl
    #hiredis
    Threads::Threads
    SOCI::soci_core SOCI::soci_sqlite3
    fmt::fmt
    nlohmann_json::nlohmann_json
)
