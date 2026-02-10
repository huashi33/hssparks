import socket
import os

SERVER_ADDRESS = './uds_socket'

# 1. 确保环境清理：如果文件已存在则先删除
if os.path.exists(SERVER_ADDRESS):
    os.remove(SERVER_ADDRESS)

# 2. 创建 AF_UNIX 类型的流式 Socket
server = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)

# 3. 绑定地址并开始监听
server.bind(SERVER_ADDRESS)
server.listen(5)

print(f"[Server] 监听中: {SERVER_ADDRESS}")

try:
    while True:
        # 等待客户端连接 (阻塞调用)
        connection, client_address = server.accept()
        try:
            # 接收数据
            data = connection.recv(1024)
            if data:
                message = data.decode('utf-8')
                print(f"[Server] 收到数据: {message}")
                
                # 模拟处理并回传 (全双工：在同一个连接上发送)
                response = message.upper().encode('utf-8')
                connection.sendall(response)
        finally:
            connection.close()
finally:
    os.remove(SERVER_ADDRESS)