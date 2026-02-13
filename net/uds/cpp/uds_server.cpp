#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <signal.h>

#define SOCKET_PATH "./uds_socket"
#define BUFFER_SIZE 1024

// 1. 定义清理函数
void cleanup() {
    printf("\n[Server] 正在清理并删除 Socket 文件...\n");
    unlink(SOCKET_PATH);
}

// 2. 信号处理函数：当按下 Ctrl+C 时触发
void handle_signal(int sig) {
    // 退出程序，会自动触发 atexit 注册的 cleanup
    exit(0); 
}

int main() {
    int server_fd, client_fd;
    struct sockaddr_un addr;
    char buffer[BUFFER_SIZE];

    // 3. 注册退出清理钩子
    // 无论程序是执行完 exit(0) 还是从 main 返回，都会运行 cleanup
    atexit(cleanup);

    // 4. 捕获中断信号 (Ctrl+C)
    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);
    signal(SIGKILL, handle_signal);

    // 创建 Socket
    if ((server_fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        perror("socket error");
        exit(EXIT_FAILURE);
    }

    memset(&addr, 0, sizeof(struct sockaddr_un));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);

    // 启动前先尝试清理，防止残留文件导致 bind 失败
    unlink(SOCKET_PATH);

    if (bind(server_fd, (struct sockaddr *)&addr, sizeof(struct sockaddr_un)) == -1) {
        perror("bind error");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 5) == -1) {
        perror("listen error");
        exit(EXIT_FAILURE);
    }

    printf("[Server] 运行中... 按下 Ctrl+C 退出并自动清理\n");

    while (1) {
        if ((client_fd = accept(server_fd, NULL, NULL)) == -1) {
            perror("accept error");
            continue;
        }

        ssize_t n = read(client_fd, buffer, sizeof(buffer) - 1);
        if (n > 0) {
            buffer[n] = '\0';
            // 业务逻辑：转大写并回传
            for (int i = 0; i < n; i++) {
                if (buffer[i] >= 'a' && buffer[i] <= 'z') buffer[i] -= 32;
            }
            write(client_fd, buffer, n);
        }
        close(client_fd);
    }

    return 0;
}