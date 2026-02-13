#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#define SOCKET_PATH "./uds_socket"

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <message>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int client_fd;
    struct sockaddr_un addr;
    char buffer[1024];

    // 1. 创建 Socket
    client_fd = socket(AF_UNIX, SOCK_STREAM, 0);

    // 2. 设置目标地址
    memset(&addr, 0, sizeof(struct sockaddr_un));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);

    // 3. 连接服务端 (阻塞直到成功)
    if (connect(client_fd, (struct sockaddr *)&addr, sizeof(struct sockaddr_un)) == -1) {
        perror("connect error");
        exit(EXIT_FAILURE);
    }

    // 4. 发送与接收
    write(client_fd, argv[1], strlen(argv[1]));
    
    ssize_t n = read(client_fd, buffer, sizeof(buffer) - 1);
    if (n > 0) {
        buffer[n] = '\0';
        printf("[Client] 收到服务端回传: %s\n", buffer);
    }

    close(client_fd);
    return 0;
}