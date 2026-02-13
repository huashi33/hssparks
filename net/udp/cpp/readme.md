# UDP Sender/Receiver 程序

一个使用C++标准库实现的UDP数据收发程序。

## 特性

- **无第三方库依赖**：只使用C++标准库和POSIX系统调用
- **无连接通信**：典型的UDP特性，不需要建立连接
- **完整错误处理**：包括异常捕获和错误日志输出
- **双向通信**：支持接收器和发送器独立运行

## 文件说明

- `udp_recver.cpp` - UDP接收器程序
  - 监听端口9999
  - 接收并显示来自任意客户端的数据
  - 显示发送者IP地址和端口
  
- `udp_sender.cpp` - UDP发送器程序
  - 向127.0.0.1:9999发送数据
  - 交互式输入消息
  - 显示发送状态

- `build.sh` - 编译脚本

## 编译

```bash
chmod +x build.sh
./build.sh
```

或手动编译：

```bash
g++ -std=c++11 -o udp_recver udp_recver.cpp
g++ -std=c++11 -o udp_sender udp_sender.cpp
```

## 使用

### 启动接收器

在一个终端运行：

```bash
./udp_recver
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
./udp_sender
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

### UDP接收器（udp_recver）

- 创建UDP套接字（SOCK_DGRAM）
- 绑定到INADDR_ANY（所有网络接口）和端口9999
- 循环调用recvfrom()接收数据
- 显示发送者信息和数据内容
- 使用Ctrl+C停止

### UDP发送器（udp_sender）

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

## 配置参数

在`udp_recver.cpp`中可修改的常量：

```cpp
const int PORT = 9999;           // 监听端口
const int BUFFER_SIZE = 1024;    // 接收缓冲区大小
```

在`udp_sender.cpp`中可修改的常量：

```cpp
const char* DEST_IP = "127.0.0.1";   // 目标IP地址
const int DEST_PORT = 9999;          // 目标端口
const int BUFFER_SIZE = 1024;        // 缓冲区大小
```

## 退出程序

- **接收器**：按Ctrl+C
- **发送器**：输入'quit'并按Enter键

## 多发送器测试

可以在多个终端同时启动发送器：

```bash
# 终端1：启动接收器
./udp_recver

# 终端2、3、4等：启动多个发送器
./udp_sender
```

接收器可以处理来自不同来源的UDP数据包，完全无连接。

## UDP vs TCP对比

| 特性 | UDP | TCP |
|-----|-----|-----|
| 连接 | 无连接 | 有连接 |
| 可靠性 | 不保证 | 保证可靠 |
| 速度 | 快 | 相对较慢 |
| 延迟 | 低 | 较高 |
| 用途 | 流媒体、DNS、游戏 | 文件传输、邮件 |
