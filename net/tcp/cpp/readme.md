# TCP Server/Client 程序

一个使用C++标准库实现的多线程TCP服务器和客户端程序。

## 特性

- **无第三方库依赖**：只使用C++标准库和POSIX系统调用
- **多线程架构**：每个客户端连接都在独立的线程中处理
- **完整错误处理**：包括异常捕获和错误日志输出
- **回显服务**：服务器将接收到的消息回显给客户端

## 文件说明

- `tcp_server.cpp` - TCP服务器程序
  - 监听端口8888
  - 为每个连接创建新线程
  - 接收客户端消息并回显
  
- `tcp_client.cpp` - TCP客户端程序
  - 连接到服务器
  - 交互式发送消息
  - 接收服务器响应

- `build.sh` - 编译脚本

## 编译

```bash
chmod +x build.sh
./build.sh
```

或手动编译：

```bash
g++ -std=c++11 -pthread -o tcp_server tcp_server.cpp
g++ -std=c++11 -o tcp_client tcp_client.cpp
```

## 使用

### 启动服务器

```bash
./tcp_server
```

输出示例：
```
Starting TCP Server...
Server listening on port 8888
```

### 启动客户端

在另一个终端运行：

```bash
./tcp_client
```

输入消息进行通信：
```
Connected to server at 127.0.0.1:8888
Enter message (or 'quit' to exit): Hello
Server response: Server received: Hello
```

## 架构说明

### 服务器架构

1. **主线程**：
   - 创建服务器套接字
   - 绑定到端口8888
   - 进入接受连接循环
   - 为每个新连接启动工作线程

2. **工作线程**：
   - 为每个客户端连接独立运行
   - 循环接收客户端数据
   - 处理数据并发送回复
   - 客户端断开连接时退出

### 关键特性

- **Socket API**：使用POSIX标准的socket接口
- **线程管理**：使用std::thread和std::shared_ptr管理线程生命周期
- **资源管理**：自动关闭socket和资源清理
- **原子标志**：使用std::atomic<bool>控制服务器状态

## 配置参数

在`tcp_server.cpp`中可修改的常量：

```cpp
const int PORT = 8888;           // 监听端口
const int BACKLOG = 5;           // 等待连接队列长度
const int BUFFER_SIZE = 1024;    // 接收缓冲区大小
```

在`tcp_client.cpp`中可修改的常量：

```cpp
const char* SERVER_IP = "127.0.0.1";   // 服务器地址
const int SERVER_PORT = 8888;          // 服务器端口
const int BUFFER_SIZE = 1024;          // 接收缓冲区大小
```

## 退出程序

- **客户端**：输入`quit`并按Enter键
- **服务器**：按Ctrl+C

## 测试示例

在第一个终端启动服务器：
```bash
./tcp_server
```

在第二、三个终端启动多个客户端：
```bash
./tcp_client
```

服务器会为每个连接创建新线程，可同时处理多个客户端连接。
