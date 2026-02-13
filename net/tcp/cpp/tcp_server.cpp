#include <iostream>
#include <thread>
#include <vector>
#include <string>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <atomic>
#include <memory>

const int PORT = 8888;
const int BACKLOG = 5;
const int BUFFER_SIZE = 1024;

std::atomic<bool> running(true);
std::vector<std::shared_ptr<std::thread>> threads;

// 处理客户端连接的线程函数
void handleClient(int clientSocket, int clientId) {
    std::cout << "Client " << clientId << " connected. Socket: " << clientSocket << std::endl;
    
    char buffer[BUFFER_SIZE];
    ssize_t bytesRead;
    
    try {
        while (running) {
            // 接收客户端数据
            bytesRead = recv(clientSocket, buffer, BUFFER_SIZE - 1, 0);
            
            if (bytesRead > 0) {
                buffer[bytesRead] = '\0';
                std::cout << "Client " << clientId << " sent: " << buffer << std::endl;
                
                // 回显数据给客户端
                std::string response = "Server received: " + std::string(buffer);
                send(clientSocket, response.c_str(), response.length(), 0);
                
            } else if (bytesRead == 0) {
                std::cout << "Client " << clientId << " disconnected." << std::endl;
                break;
            } else {
                std::cerr << "Error reading from client " << clientId << std::endl;
                break;
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Exception in handleClient: " << e.what() << std::endl;
    }
    
    // 关闭连接
    close(clientSocket);
    std::cout << "Client " << clientId << " connection closed." << std::endl;
}

// 主服务器函数
void startServer() {
    // 创建服务器套接字
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0) {
        std::cerr << "Error creating server socket" << std::endl;
        return;
    }
    
    // 设置套接字选项，允许地址复用
    int opt = 1;
    if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        std::cerr << "Error setting socket options" << std::endl;
        close(serverSocket);
        return;
    }
    
    // 绑定地址
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(PORT);
    
    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        std::cerr << "Error binding socket to port " << PORT << std::endl;
        close(serverSocket);
        return;
    }
    
    // 监听连接
    if (listen(serverSocket, BACKLOG) < 0) {
        std::cerr << "Error listening on socket" << std::endl;
        close(serverSocket);
        return;
    }
    
    std::cout << "Server listening on port " << PORT << std::endl;
    
    int clientId = 0;
    
    // 接受客户端连接
    while (running) {
        struct sockaddr_in clientAddr;
        socklen_t clientAddrLen = sizeof(clientAddr);
        
        int clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientAddrLen);
        
        if (clientSocket < 0) {
            if (running) {
                std::cerr << "Error accepting connection" << std::endl;
            }
            continue;
        }
        
        clientId++;
        
        // 为每个客户端创建新线程
        auto clientThread = std::make_shared<std::thread>(handleClient, clientSocket, clientId);
        threads.push_back(clientThread);
        
        std::cout << "Started thread for Client " << clientId << std::endl;
    }
    
    // 等待所有线程完成
    for (auto& t : threads) {
        if (t && t->joinable()) {
            t->join();
        }
    }
    
    close(serverSocket);
    std::cout << "Server stopped." << std::endl;
}

int main() {
    std::cout << "Starting TCP Server..." << std::endl;
    
    try {
        startServer();
    } catch (const std::exception& e) {
        std::cerr << "Exception in main: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
