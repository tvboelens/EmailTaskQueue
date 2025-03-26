# ------- Stage 1: Build -------
FROM ubuntu:latest AS builder

WORKDIR /EmailTaskQueue

# Update apps on the base image
RUN apt-get -y update && apt-get install -y

# Install dependencies
RUN apt-get -y install git clang cmake ninja-build libcurl4-openssl-dev libfmt-dev nlohmann-json3-dev \ 
    libasio-dev sqlite3 libsqlite3-dev libspdlog-dev 
 
# Build Crow from source
RUN git clone https://github.com/CrowCpp/Crow.git && \
    cd Crow && \
    mkdir build && cd build && \
    cmake .. -DCROW_BUILD_EXAMPLES=OFF -DCROW_BUILD_TESTS=OFF && \
    make install

COPY ./CMakeLists.txt /EmailTaskQueue
COPY ./include /EmailTaskQueue/include
COPY ./src /EmailTaskQueue/src
RUN mkdir build && \
    cmake -B build -DCMAKE_BUILD_TYPE=Release -GNinja .
RUN ninja -C build

# ------ Stage 2: Run -------
FROM ubuntu:latest

WORKDIR /EmailTaskQueue
# Update apps on the base image
RUN apt-get -y update && apt-get install -y
# Install dependencies
RUN apt-get -y install libcurl4-openssl-dev libfmt-dev sqlite3 libsqlite3-dev

COPY --from=builder /EmailTaskQueue/build/email_task_queue .
ENTRYPOINT ["./email_task_queue"]



