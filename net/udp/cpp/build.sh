#!/bin/bash

# Build script for UDP sender and receiver

echo "Compiling UDP Receiver..."
g++ -std=c++11 -o udp_recver udp_recver.cpp
if [ $? -ne 0 ]; then
    echo "Error: Failed to compile UDP receiver"
    exit 1
fi

echo "Compiling UDP Sender..."
g++ -std=c++11 -o udp_sender udp_sender.cpp
if [ $? -ne 0 ]; then
    echo "Error: Failed to compile UDP sender"
    exit 1
fi

echo "Build successful!"
echo "Usage:"
echo "  ./udp_recver          # Start the receiver (listening)"
echo "  ./udp_sender          # Start the sender (send data)"
