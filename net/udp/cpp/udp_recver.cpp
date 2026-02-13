#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

const int PORT = 9999;
const int BUFFER_SIZE = 1024;

int main() {
    // 创建UDP套接字
    int udpSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (udpSocket < 0) {
        std::cerr << "Error creating UDP socket" << std::endl;
        return 1;
    }
    
    // 设置套接字选项，允许地址复用
    int opt = 1;
    if (setsockopt(udpSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        std::cerr << "Error setting socket options" << std::endl;
        close(udpSocket);
        return 1;
    }
    
    // 绑定本地地址
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(PORT);
    
    if (bind(udpSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        std::cerr << "Error binding socket to port " << PORT << std::endl;
        close(udpSocket);
        return 1;
    }
    
    std::cout << "UDP Receiver listening on port " << PORT << std::endl;
    std::cout << "Waiting for data..." << std::endl;
    std::cout << "Press Ctrl+C to stop\n" << std::endl;
    
    char buffer[BUFFER_SIZE];
    struct sockaddr_in clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);
    int packetCount = 0;
    
    try {
        while (true) {
            // 接收数据
            memset(buffer, 0, BUFFER_SIZE);
            ssize_t bytesRead = recvfrom(udpSocket, buffer, BUFFER_SIZE - 1, 0,
                                         (struct sockaddr*)&clientAddr, &clientAddrLen);
            
            if (bytesRead > 0) {
                buffer[bytesRead] = '\0';
                packetCount++;
                
                // 显示发送者信息和接收的数据
                std::cout << "\n[Packet #" << packetCount << "]" << std::endl;
                std::cout << "From: " << inet_ntoa(clientAddr.sin_addr) 
                          << ":" << ntohs(clientAddr.sin_port) << std::endl;
                std::cout << "Bytes: " << bytesRead << std::endl;
                std::cout << "Data: " << buffer << std::endl;
                
            } else if (bytesRead < 0) {
                std::cerr << "Error receiving data" << std::endl;
                break;
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }
    
    close(udpSocket);
    std::cout << "\n\nUDP Receiver stopped." << std::endl;
    
    return 0;
}
