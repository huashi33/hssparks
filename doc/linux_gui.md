# Linux GUI 显示机制

## 1. 显示原理

Linux 的图形显示采用客户端/服务器架构。应用程序（客户端）不直接操作屏幕硬件，而是通过协议与**显示服务器**通信，由显示服务器负责渲染和输出到屏幕。

```
应用程序 (Qt/GTK/...)
      │  发送绘图指令
      ▼
显示服务器 (X Server / Wayland Compositor)
      │  调用驱动
      ▼
显卡驱动 → 物理屏幕
```

整个体系分三层：
- 协议层：定义应用和显示服务器之间如何通信（X11 协议 / Wayland 协议）
- 服务器层：实现协议、管理窗口、合成画面（Xorg / Wayland Compositor）
- 应用层：使用工具库（Qt、GTK）调用协议，不关心底层细节

---

## 2. 核心概念

### X11

X11（X Window System，第11版）是 Linux/Unix 图形系统的基础**协议**，诞生于1987年。

- 定义了应用程序和显示服务器之间的通信规范
- 天然支持**网络透明**：应用可以运行在远程机器，画面显示在本地（`DISPLAY=远程IP:0`）
- 几乎所有 Linux GUI 程序（Qt、GTK）底层都基于 X11 协议
- 缺点：协议设计老旧，安全性差，性能有瓶颈

```bash
# 查看当前 X11 连接信息
echo $DISPLAY        # 输出类似 :0 或 localhost:10.0
xlsclients           # 列出当前连接到 X Server 的客户端
```

---

### Wayland

Wayland 是用来**替代 X11** 的新一代显示协议，2008年开始开发。

- 架构更简洁：合成器（Compositor）直接承担显示服务器的角色，减少中间层
- 安全性更好：应用之间相互隔离，无法截取其他窗口内容
- 性能更好：减少了 X11 的冗余通信
- 兼容性：通过 `XWayland` 兼容运行 X11 应用

| 对比项 | X11 | Wayland |
|--------|-----|---------|
| 网络透明 | 原生支持 | 不支持（需借助其他工具） |
| 安全隔离 | 弱 | 强 |
| 性能 | 一般 | 更好 |
| 成熟度 | 极成熟 | 较新，部分场景仍有问题 |

目前 Ubuntu 22.04+、Fedora 等主流发行版默认使用 Wayland，但服务器和嵌入式场景仍以 X11 为主。

---

### Xorg

Xorg 是 X11 协议最主流的**服务器实现**（开源实现），也是目前 Linux 桌面最广泛使用的显示服务器。

- 实现了 X11 协议的服务端
- 管理键盘、鼠标、显示器等输入输出设备
- 加载显卡驱动（如 `nvidia`、`intel`、`modesetting`）
- 配置文件位于 `/etc/X11/xorg.conf`（现代系统通常自动检测，无需手动配置）

```bash
# 查看 Xorg 是否在运行
ps aux | grep Xorg

# 查看 Xorg 日志
cat /var/log/Xorg.0.log
```

---

### DISPLAY=:1

`DISPLAY` 是一个环境变量，告诉 X11 客户端程序**连接到哪个显示服务器**。

格式：`DISPLAY=[主机名]:显示编号[.屏幕编号]`

| 值 | 含义 |
|----|------|
| `:0` | 本机第0个显示器（物理屏幕，通常是登录桌面） |
| `:1` | 本机第1个显示器（通常是虚拟显示器，如 Xvfb 或 VNC） |
| `192.168.1.10:0` | 远程主机的第0个显示器 |

```bash
# 在指定显示器上运行程序
DISPLAY=:1 ./my_qt_app

# 或者先导出再运行
export DISPLAY=:1
./my_qt_app

# WSL 中连接到 Windows 宿主机的 X Server
export DISPLAY=$(cat /etc/resolv.conf | grep nameserver | awk '{print $2}'):0
```

---

### Xvfb

Xvfb（X Virtual Framebuffer）是一个**虚拟显示服务器**，在内存中模拟屏幕，不需要物理显示器。

适用场景：
- 服务器/WSL 环境运行 GUI 程序（无物理屏幕）
- CI/CD 流水线中运行 GUI 自动化测试
- 配合 VNC 或截图工具使用

```bash
# 安装
sudo apt install xvfb

# 启动虚拟显示器（:1号，分辨率1280x800，24位色）
Xvfb :1 -screen 0 1280x800x24 &

# 在虚拟显示器上运行 Qt 程序
DISPLAY=:1 ./my_qt_app

# 截图（需要 scrot 或 imagemagick）
DISPLAY=:1 scrot screenshot.png

# 停止
kill $(pgrep Xvfb)
```

---

### VNC

VNC（Virtual Network Computing）是一种**远程桌面协议**，将显示内容通过网络传输到远程客户端。

工作原理：
```
VNC Server（运行在 Linux）
    │  捕获屏幕画面
    │  监听 TCP 端口（默认 5900+显示编号）
    ▼
网络传输
    ▼
VNC Client（运行在 Windows/Mac/手机）
    │  显示画面
    │  发送键盘/鼠标事件
```

常用 VNC 服务端：

| 工具 | 特点 |
|------|------|
| `tightvncserver` | 轻量，老牌，压缩效果好 |
| `tigervnc` | 功能完善，支持 NLA 认证 |
| `x11vnc` | 共享已有 X 会话（包括 Xvfb），灵活 |
| `noVNC` | 基于 WebSocket，浏览器直接访问，无需客户端 |

---

## 3. WSL 中运行 Qt 程序并用 VNC 查看

### 方案一：Xvfb + x11vnc（轻量，推荐）

不需要完整桌面环境，直接把 Qt 程序画面通过 VNC 暴露出来。

```bash
# 安装依赖
sudo apt install -y xvfb x11vnc

# 1. 启动虚拟显示器
Xvfb :1 -screen 0 1280x800x24 &

# 2. 启动 VNC 服务（无密码，仅监听本地）
x11vnc -display :1 -nopw -listen localhost -forever &

# 3. 运行 Qt 程序
DISPLAY=:1 ./hw

# 如果程序用了 OpenGL 但 Xvfb 不支持硬件加速，加软件渲染
DISPLAY=:1 LIBGL_ALWAYS_SOFTWARE=1 ./hw
```

### 方案二：tightvncserver（完整桌面）

```bash
# 安装
sudo apt install -y tightvncserver xfce4

# 设置 VNC 密码
vncpasswd

# 配置启动脚本
mkdir -p ~/.vnc
echo "startxfce4 &" > ~/.vnc/xstartup
chmod +x ~/.vnc/xstartup

# 启动 VNC（端口 = 5900 + 显示编号，:1 对应 5901）
vncserver :1 -geometry 1280x800 -depth 24

# 停止
vncserver -kill :1
```

### WSL2 端口转发到 Windows 主机

WSL2 有独立 IP，需要在 Windows 做端口转发（PowerShell 管理员）：

```powershell
# 查看 WSL 的 IP
wsl hostname -I

# 添加端口转发（将主机 5900 转发到 WSL 的 5900）
netsh interface portproxy add v4tov4 `
    listenport=5900 listenaddress=0.0.0.0 `
    connectport=5900 connectaddress=<WSL的IP>

# 查看已有转发规则
netsh interface portproxy show all

# 删除规则
netsh interface portproxy delete v4tov4 listenport=5900 listenaddress=0.0.0.0
```

然后在 Windows 用 VNC Viewer / TigerVNC 连接 `127.0.0.1:5900`。

---

## 4. 构建并运行示例 Qt 程序

```bash
cd test/qt/hw
mkdir -p build && cd build
cmake ..
make

# 用虚拟显示器运行
DISPLAY=:1 LIBGL_ALWAYS_SOFTWARE=1 ./hw
```
