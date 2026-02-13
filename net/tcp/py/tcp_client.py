#!/usr/bin/env python3
"""
TCP Client - Interactive client for testing the TCP server
No external dependencies required
"""

import socket
import sys

# Configuration
SERVER_HOST = '127.0.0.1'
SERVER_PORT = 8888
BUFFER_SIZE = 1024

def connect_to_server():
    """
    Connect to the TCP server and send/receive messages interactively
    """
    # Create client socket
    client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    
    try:
        # Connect to server
        print(f"Connecting to server at {SERVER_HOST}:{SERVER_PORT}...")
        client_socket.connect((SERVER_HOST, SERVER_PORT))
        print(f"Connected to server!\n")
        
        # Interactive message loop
        while True:
            try:
                message = input("Enter message (or 'quit' to exit): ").strip()
                
                if message.lower() == 'quit':
                    break
                
                if not message:
                    continue
                
                # Send message to server
                client_socket.sendall(message.encode('utf-8'))
                
                # Receive response from server
                response = client_socket.recv(BUFFER_SIZE)
                
                if not response:
                    print("Server disconnected.")
                    break
                
                print(f"Server response: {response.decode('utf-8', errors='replace')}\n")
                
            except KeyboardInterrupt:
                print("\n\nDisconnecting...")
                break
            except Exception as e:
                print(f"Error: {e}")
                break
                
    except ConnectionRefusedError:
        print(f"Error: Could not connect to server at {SERVER_HOST}:{SERVER_PORT}")
        print("Make sure the server is running.")
    except Exception as e:
        print(f"Error: {e}")
    finally:
        client_socket.close()
        print("Disconnected from server.")


if __name__ == '__main__':
    try:
        connect_to_server()
    except Exception as e:
        print(f"Fatal error: {e}")
        sys.exit(1)
