#!/bin/bash

# Build script for TCP server and client

echo "Compiling TCP Server..."
g++ -std=c++11 -pthread -o tcp_server tcp_server.cpp
if [ $? -ne 0 ]; then
    echo "Error: Failed to compile TCP server"
    exit 1
fi

echo "Compiling TCP Client..."
g++ -std=c++11 -o tcp_client tcp_client.cpp
if [ $? -ne 0 ]; then
    echo "Error: Failed to compile TCP client"
    exit 1
fi

echo "Build successful!"
echo "Usage:"
echo "  ./tcp_server          # Start the server"
echo "  ./tcp_client          # Start the client"
