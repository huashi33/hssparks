#!/usr/bin/env python3
"""
UDP Receiver - Receives UDP data packets
No external dependencies required
"""

import socket
import sys

# Configuration
HOST = '0.0.0.0'  # Listen on all interfaces
PORT = 9999
BUFFER_SIZE = 1024

def start_receiver():
    """
    Start UDP receiver and listen for incoming packets
    """
    # Create UDP socket
    receiver_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    
    # Allow address reuse
    receiver_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    
    try:
        # Bind to port
        receiver_socket.bind((HOST, PORT))
        print(f"UDP Receiver listening on port {PORT}")
        print("Waiting for data...")
        print("Press Ctrl+C to stop\n")
        
        packet_count = 0
        
        while True:
            # Receive data
            data, client_address = receiver_socket.recvfrom(BUFFER_SIZE)
            
            if data:
                packet_count += 1
                message = data.decode('utf-8', errors='replace')
                
                print(f"\n[Packet #{packet_count}]")
                print(f"From: {client_address[0]}:{client_address[1]}")
                print(f"Bytes: {len(data)}")
                print(f"Data: {message}")
                
    except KeyboardInterrupt:
        print("\n\nUDP Receiver stopped.")
    except Exception as e:
        print(f"Error: {e}")
    finally:
        receiver_socket.close()


if __name__ == '__main__':
    try:
        start_receiver()
    except Exception as e:
        print(f"Fatal error: {e}")
        sys.exit(1)
