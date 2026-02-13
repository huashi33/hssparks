# TCP Server/Client - Python Version

一个使用Python标准库实现的多线程TCP服务器和客户端程序。

## 特性

- **无第三方库依赖**：只使用Python标准库
- **多线程架构**：每个客户端连接都在独立的线程中处理
- **完整错误处理**：包括异常捕获和错误日志输出
- **回显服务**：服务器将接收到的消息回显给客户端
- **线程安全**：使用Lock保护共享资源

## 文件说明

- `tcp_server.py` - TCP服务器程序
  - 监听127.0.0.1:8888
  - 为每个连接创建新线程
  - 接收客户端消息并回显
  
- `tcp_client.py` - TCP客户端程序
  - 连接到服务器
  - 交互式发送消息
  - 接收服务器响应

## 运行

### 启动服务器

```bash
python3 tcp_server.py
```

输出示例：
```
Server binding to 127.0.0.1:8888
Server listening on port 8888
Press Ctrl+C to stop the server

New connection from 127.0.0.1:xxxxx
Client 1 connected. Socket: 4
Started thread for Client 1
```

### 启动客户端

在另一个终端运行：

```bash
python3 tcp_client.py
```

交互示例：
```
Connecting to server at 127.0.0.1:8888...
Connected to server!

Enter message (or 'quit' to exit): Hello World
Server response: Server received: Hello World

Enter message (or 'quit' to exit): Test Message
Server response: Server received: Test Message

Enter message (or 'quit' to exit): quit
Disconnected from server.
```

## 架构说明

### 服务器架构

1. **主线程**：
   - 创建服务器套接字
   - 绑定到127.0.0.1:8888
   - 进入接受连接循环
   - 为每个新连接启动工作线程

2. **工作线程**：
   - 为每个客户端连接独立运行
   - 循环接收客户端数据
   - 处理数据并发送回复
   - 客户端断开连接时退出

### 关键特性

- **Socket API**：使用Python socket模块
- **线程管理**：使用threading.Thread管理线程
- **线程安全**：使用threading.Lock保护client_counter
- **自动资源清理**：使用try/finally确保socket关闭

## 配置参数

在`tcp_server.py`中可修改的常量：

```python
HOST = '127.0.0.1'           # 监听地址
PORT = 8888                  # 监听端口
BACKLOG = 5                  # 等待连接队列长度
BUFFER_SIZE = 1024           # 接收缓冲区大小
```

在`tcp_client.py`中可修改的常量：

```python
SERVER_HOST = '127.0.0.1'    # 服务器地址
SERVER_PORT = 8888           # 服务器端口
BUFFER_SIZE = 1024           # 接收缓冲区大小
```

## 退出程序

- **客户端**：输入`quit`并按Enter键，或Ctrl+C
- **服务器**：按Ctrl+C

## 多客户端测试

在一个终端启动服务器：
```bash
python3 tcp_server.py
```

在多个其他终端启动客户端：
```bash
python3 tcp_client.py
```

服务器会为每个连接创建新线程，可同时处理多个客户端连接。
