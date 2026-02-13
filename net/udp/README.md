# UDP 网络编程完整实现

本项目包含UDP接收器和发送器的C++和Python完整实现。

## 📁 文件结构

```
udp/
├── cpp/
│   ├── udp_recver.cpp       # C++ UDP接收器源代码
│   ├── udp_sender.cpp       # C++ UDP发送器源代码
│   ├── build.sh             # C++编译脚本
│   ├── udp_recver           # 编译后的接收器可执行文件
│   ├── udp_sender           # 编译后的发送器可执行文件
│   └── readme.md            # C++版本文档
└── py/
    ├── udp_server.py        # Python UDP接收器
    ├── udp_client.py        # Python UDP发送器
    └── readme.md            # Python版本文档
```

## ✨ 特性总结

| 特性 | 说明 |
|-----|------|
| **无第三方库** | 仅使用系统标准库 |
| **跨语言实现** | 同时提供C++和Python版本 |
| **无连接通信** | UDP特性，不需要建立连接 |
| **完整错误处理** | 异常捕获和错误日志 |
| **生产级代码** | 经过测试的稳定实现 |

## 🚀 快速开始

### C++ 版本

```bash
cd cpp
chmod +x build.sh
./build.sh

# 启动接收器
./udp_recver

# 在另一个终端启动发送器
./udp_sender
```

### Python 版本

```bash
cd py

# 启动接收器
python3 udp_server.py

# 在另一个终端启动发送器
python3 udp_client.py
```

## 📊 程序对比

### C++ UDP接收器 (udp_recver)
- **功能**：监听9999端口，接收任意客户端的UDP数据
- **编译**：`g++ -std=c++11 -o udp_recver udp_recver.cpp`
- **运行**：`./udp_recver`
- **输出**：显示发送者IP、端口、数据内容和数据大小

### C++ UDP发送器 (udp_sender)
- **功能**：交互式向127.0.0.1:9999发送数据
- **编译**：`g++ -std=c++11 -o udp_sender udp_sender.cpp`
- **运行**：`./udp_sender`
- **交互**：输入消息发送，输入'quit'退出

### Python UDP接收器 (udp_server.py)
- **功能**：监听所有接口的9999端口，接收UDP数据
- **运行**：`python3 udp_server.py`
- **停止**：Ctrl+C

### Python UDP发送器 (udp_client.py)
- **功能**：交互式向127.0.0.1:9999发送数据
- **运行**：`python3 udp_client.py`
- **交互**：输入消息发送，输入'quit'退出

## 🔧 配置参数

### C++ 版本可配置常量

**udp_recver.cpp：**
```cpp
const int PORT = 9999;           // 监听端口
const int BUFFER_SIZE = 1024;    // 接收缓冲区大小
```

**udp_sender.cpp：**
```cpp
const char* DEST_IP = "127.0.0.1";   // 目标地址
const int DEST_PORT = 9999;          // 目标端口
const int BUFFER_SIZE = 1024;        // 缓冲区大小
```

### Python 版本可配置常量

**udp_server.py：**
```python
HOST = '0.0.0.0'         # 监听所有接口
PORT = 9999              # 监听端口
BUFFER_SIZE = 1024       # 接收缓冲区大小
```

**udp_client.py：**
```python
DEST_HOST = '127.0.0.1'  # 目标地址
DEST_PORT = 9999         # 目标端口
BUFFER_SIZE = 1024       # 缓冲区大小
```

## 📝 测试示例

### C++ 版本测试

终端1：启动接收器
```bash
./udp_recver
```

终端2：启动发送器
```bash
./udp_sender
Enter message: Hello UDP World
Packet #1 sent successfully! (15 bytes)
```

接收器输出：
```
[Packet #1]
From: 127.0.0.1:12345
Bytes: 15
Data: Hello UDP World
```

### Python 版本测试

终端1：启动接收器
```bash
python3 udp_server.py
```

终端2：启动发送器
```bash
python3 udp_client.py
Enter message: Test Message
Packet #1 sent successfully! (12 bytes)
```

## 🎯 使用场景

UDP适用于以下场景：

1. **实时流媒体** - 直播、视频通话
2. **在线游戏** - 低延迟游戏数据传输
3. **DNS查询** - 无连接的快速查询
4. **IoT设备通信** - 轻量级设备数据收集
5. **监控告警系统** - 简单的数据上报

## ⚠️ UDP vs TCP

| 特性 | UDP | TCP |
|-----|-----|-----|
| **连接** | 无连接 | 需建立连接 |
| **可靠性** | 不保证可靠 | 保证可靠传输 |
| **顺序** | 不保证顺序 | 保证顺序 |
| **速度** | 快 | 相对较慢 |
| **开销** | 低 | 较高 |
| **用途** | 实时性优先 | 可靠性优先 |

## 📚 代码质量

- ✅ 完整的错误处理
- ✅ 资源正确释放
- ✅ 内存安全
- ✅ 清晰的注释
- ✅ 标准化的代码风格
- ✅ 跨平台兼容（Linux/Unix）

## 🔍 故障排除

### 问题：地址已被占用
**解决**：等待1-2分钟后重试，或修改PORT变量

### 问题：权限不足
**解决**：确保脚本有执行权限：`chmod +x *.sh` 和 `chmod +x *.py`

### 问题：编译失败
**解决**：确保安装了g++编译器：`sudo apt-get install build-essential`

## 📄 许可证

这些示例代码可自由使用和修改。
