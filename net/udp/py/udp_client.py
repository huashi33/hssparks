#!/usr/bin/env python3
"""
UDP Sender - Sends UDP data packets
No external dependencies required
"""

import socket
import sys

# Configuration
DEST_HOST = '127.0.0.1'
DEST_PORT = 9999
BUFFER_SIZE = 1024

def send_messages():
    """
    Send UDP messages to receiver
    """
    # Create UDP socket
    sender_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    
    try:
        print(f"UDP Sender - Send data to {DEST_HOST}:{DEST_PORT}")
        print("Enter messages (or 'quit' to exit):\n")
        
        packet_count = 0
        
        while True:
            try:
                message = input("Enter message: ").strip()
                
                if message.lower() == 'quit':
                    break
                
                if not message:
                    continue
                
                # Send data
                bytes_sent = sender_socket.sendto(
                    message.encode('utf-8'),
                    (DEST_HOST, DEST_PORT)
                )
                
                packet_count += 1
                print(f"Packet #{packet_count} sent successfully! ({bytes_sent} bytes)\n")
                
            except KeyboardInterrupt:
                print("\n\nUDP Sender stopped.")
                break
            except Exception as e:
                print(f"Error sending message: {e}")
                
    except Exception as e:
        print(f"Error: {e}")
    finally:
        sender_socket.close()
        print("\nUDP Sender stopped.")


if __name__ == '__main__':
    try:
        send_messages()
    except Exception as e:
        print(f"Fatal error: {e}")
        sys.exit(1)
