#!/usr/bin/env python3
"""
TCP Server - Multi-threaded implementation
Accepts client connections and handles each in a separate thread
No external dependencies required
"""

import socket
import threading
import sys
from threading import Lock

# Configuration
HOST = '127.0.0.1'
PORT = 8888
BACKLOG = 5
BUFFER_SIZE = 1024

# Global variables for thread safety
client_counter = 0
counter_lock = Lock()

def handle_client(client_socket, client_id):
    """
    Handle a single client connection in a separate thread
    
    Args:
        client_socket: The socket connection to the client
        client_id: Unique identifier for this client
    """
    print(f"Client {client_id} connected. Socket: {client_socket.fileno()}")
    
    try:
        while True:
            # Receive data from client
            data = client_socket.recv(BUFFER_SIZE)
            
            if not data:
                print(f"Client {client_id} disconnected.")
                break
            
            message = data.decode('utf-8', errors='replace')
            print(f"Client {client_id} sent: {message}")
            
            # Send response back to client
            response = f"Server received: {message}"
            client_socket.sendall(response.encode('utf-8'))
            
    except Exception as e:
        print(f"Error handling client {client_id}: {e}")
    finally:
        client_socket.close()
        print(f"Client {client_id} connection closed.")


def start_server():
    """
    Start the TCP server and accept client connections
    Each connection is handled in a separate thread
    """
    global client_counter
    
    # Create server socket
    server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    
    # Allow address reuse
    server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    
    try:
        # Bind to address
        server_socket.bind((HOST, PORT))
        print(f"Server binding to {HOST}:{PORT}")
        
        # Listen for connections
        server_socket.listen(BACKLOG)
        print(f"Server listening on port {PORT}")
        print("Press Ctrl+C to stop the server\n")
        
        while True:
            try:
                # Accept client connection
                client_socket, client_address = server_socket.accept()
                
                with counter_lock:
                    client_counter += 1
                    current_client_id = client_counter
                
                print(f"New connection from {client_address[0]}:{client_address[1]}")
                
                # Create a new thread for this client
                client_thread = threading.Thread(
                    target=handle_client,
                    args=(client_socket, current_client_id),
                    daemon=False
                )
                client_thread.start()
                print(f"Started thread for Client {current_client_id}\n")
                
            except Exception as e:
                print(f"Error accepting connection: {e}")
                
    except KeyboardInterrupt:
        print("\n\nServer stopping...")
    except Exception as e:
        print(f"Server error: {e}")
    finally:
        server_socket.close()
        print("Server stopped.")


if __name__ == '__main__':
    try:
        start_server()
    except Exception as e:
        print(f"Fatal error: {e}")
        sys.exit(1)
