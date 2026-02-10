import socket
import sys

SERVER_ADDRESS = './uds_socket'

if len(sys.argv) < 2:
    print(f"Usage: {sys.argv[0]} <message>")
    sys.exit(1)

message = sys.argv[1]

# 1. 创建 Socket
client = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)

try:
    # 2. 连接服务端
    client.connect(SERVER_ADDRESS)
    
    # 3. 发送数据
    print(f"[Client] 发送: {message}")
    client.sendall(message.encode('utf-8'))
    
    # 4. 阻塞读取返回结果
    data = client.recv(1024)
    print(f"[Client] 收到结果: {data.decode('utf-8')}")
    
finally:
    client.close()