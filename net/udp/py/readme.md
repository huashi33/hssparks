# UDP Sender/Receiver - Python Version

一个使用Python标准库实现的UDP数据收发程序。

## 特性

- **无第三方库依赖**：只使用Python标准库
- **无连接通信**：典型的UDP特性，不需要建立连接
- **完整错误处理**：包括异常捕获和错误日志输出
- **双向通信**：支持接收器和发送器独立运行

## 文件说明

- `udp_server.py` - UDP接收器程序
  - 监听所有接口的9999端口
  - 接收并显示来自任意客户端的数据
  - 显示发送者IP地址和端口
  
- `udp_client.py` - UDP发送器程序
  - 向127.0.0.1:9999发送数据
  - 交互式输入消息
  - 显示发送状态

## 运行

### 启动接收器

```bash
python3 udp_server.py
```

输出示例：
```
UDP Receiver listening on port 9999
Waiting for data...
Press Ctrl+C to stop

[Packet #1]
From: 127.0.0.1:12345
Bytes: 11
Data: Hello UDP!
```

### 启动发送器

在另一个终端运行：

```bash
python3 udp_client.py
```

交互示例：
```
UDP Sender - Send data to 127.0.0.1:9999
Enter messages (or 'quit' to exit):

Enter message: Hello UDP!
Packet #1 sent successfully! (11 bytes)

Enter message: This is a test
Packet #2 sent successfully! (14 bytes)

Enter message: quit
```

## 架构说明

### UDP接收器（udp_server.py）

- 创建UDP套接字（SOCK_DGRAM）
- 绑定到所有接口（0.0.0.0）和端口9999
- 循环调用recvfrom()接收数据
- 显示发送者信息和数据内容
- 使用Ctrl+C停止

### UDP发送器（udp_client.py）

- 创建UDP套接字
- 设置目标地址（127.0.0.1:9999）
- 交互式读取用户输入
- 调用sendto()发送数据
- 输入'quit'退出

## 关键特性

- **无状态通信**：不需要先建立连接
- **灵活寻址**：接收器可以接收来自任意客户端的数据
- **低延迟**：相比TCP，UDP具有更低的延迟
- **资源轻量**：不维护连接状态
- **UTF-8编码**：支持中文等多字节字符

## 配置参数

在`udp_server.py`中可修改的常量：

```python
HOST = '0.0.0.0'             # 监听地址（所有接口）
PORT = 9999                  # 监听端口
BUFFER_SIZE = 1024           # 接收缓冲区大小
```

在`udp_client.py`中可修改的常量：

```python
DEST_HOST = '127.0.0.1'      # 目标IP地址
DEST_PORT = 9999             # 目标端口
BUFFER_SIZE = 1024           # 缓冲区大小
```

## 退出程序

- **接收器**：按Ctrl+C
- **发送器**：输入'quit'并按Enter键

## 多发送器测试

可以在多个终端同时启动发送器：

```bash
# 终端1：启动接收器
python3 udp_server.py

# 终端2、3、4等：启动多个发送器
python3 udp_client.py
```

接收器可以处理来自不同来源的UDP数据包，完全无连接。
