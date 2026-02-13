#include <iostream>
#include <string>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

const char* DEST_IP = "127.0.0.1";
const int DEST_PORT = 9999;
const int BUFFER_SIZE = 1024;

int main() {
    // 创建UDP套接字
    int udpSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (udpSocket < 0) {
        std::cerr << "Error creating UDP socket" << std::endl;
        return 1;
    }
    
    // 设置目标地址
    struct sockaddr_in destAddr;
    destAddr.sin_family = AF_INET;
    destAddr.sin_port = htons(DEST_PORT);
    
    if (inet_pton(AF_INET, DEST_IP, &destAddr.sin_addr) <= 0) {
        std::cerr << "Invalid destination address" << std::endl;
        close(udpSocket);
        return 1;
    }
    
    std::cout << "UDP Sender - Send data to " << DEST_IP << ":" << DEST_PORT << std::endl;
    std::cout << "Enter messages (or 'quit' to exit):\n" << std::endl;
    
    std::string message;
    int packetCount = 0;
    
    try {
        while (true) {
            std::cout << "Enter message: ";
            std::getline(std::cin, message);
            
            if (message == "quit") {
                break;
            }
            
            if (message.empty()) {
                continue;
            }
            
            // 发送数据
            ssize_t bytesSent = sendto(udpSocket, message.c_str(), message.length(), 0,
                                       (struct sockaddr*)&destAddr, sizeof(destAddr));
            
            if (bytesSent < 0) {
                std::cerr << "Error sending data" << std::endl;
            } else {
                packetCount++;
                std::cout << "Packet #" << packetCount << " sent successfully! (" 
                          << bytesSent << " bytes)\n" << std::endl;
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }
    
    close(udpSocket);
    std::cout << "\nUDP Sender stopped." << std::endl;
    
    return 0;
}
