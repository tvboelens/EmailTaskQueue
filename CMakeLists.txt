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

# Add source files
set(SOURCES
    src/main.cpp
    #src/redis_queue.cpp
    src/email_sender.cpp
)

# Define the executable
add_executable(email_task_queue ${SOURCES})

# Link libraries
target_link_libraries(email_task_queue
    CURL::libcurl
    #hiredis
    Threads::Threads
)
