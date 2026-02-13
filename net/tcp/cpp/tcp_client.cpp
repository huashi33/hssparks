#include <iostream>
#include <string>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

const char* SERVER_IP = "127.0.0.1";
const int SERVER_PORT = 8888;
const int BUFFER_SIZE = 1024;

int main() {
    // 创建客户端套接字
    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket < 0) {
        std::cerr << "Error creating client socket" << std::endl;
        return 1;
    }
    
    // 设置服务器地址
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(SERVER_PORT);
    
    if (inet_pton(AF_INET, SERVER_IP, &serverAddr.sin_addr) <= 0) {
        std::cerr << "Invalid server address" << std::endl;
        close(clientSocket);
        return 1;
    }
    
    // 连接到服务器
    if (connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        std::cerr << "Error connecting to server at " << SERVER_IP << ":" << SERVER_PORT << std::endl;
        close(clientSocket);
        return 1;
    }
    
    std::cout << "Connected to server at " << SERVER_IP << ":" << SERVER_PORT << std::endl;
    
    char buffer[BUFFER_SIZE];
    std::string message;
    
    // 交互式通信循环
    while (true) {
        std::cout << "Enter message (or 'quit' to exit): ";
        std::getline(std::cin, message);
        
        if (message == "quit") {
            break;
        }
        
        if (message.empty()) {
            continue;
        }
        
        // 发送消息到服务器
        if (send(clientSocket, message.c_str(), message.length(), 0) < 0) {
            std::cerr << "Error sending message to server" << std::endl;
            break;
        }
        
        // 接收服务器响应
        memset(buffer, 0, BUFFER_SIZE);
        ssize_t bytesRead = recv(clientSocket, buffer, BUFFER_SIZE - 1, 0);
        
        if (bytesRead > 0) {
            buffer[bytesRead] = '\0';
            std::cout << "Server response: " << buffer << std::endl;
        } else if (bytesRead == 0) {
            std::cout << "Server disconnected." << std::endl;
            break;
        } else {
            std::cerr << "Error receiving from server" << std::endl;
            break;
        }
    }
    
    close(clientSocket);
    std::cout << "Disconnected from server." << std::endl;
    
    return 0;
}
