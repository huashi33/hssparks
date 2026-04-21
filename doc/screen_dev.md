# 嵌入式 Linux 开发板屏幕驱动开发指南

> 开发板：Orange Pi Zero 2（全志 H616）/ Orange Pi 5 Max（瑞芯微 RK3588）
> 本文覆盖嵌入式领域常见的 6 类屏幕接口，每种以一个典型型号为例，详细说明硬件接线与软件驱动开发。

---

## 目录

1. [屏幕接口类型总览](#1-屏幕接口类型总览)
2. [SPI 接口屏 — ST7789V 1.3" 240×240 TFT](#2-spi-接口屏--st7789v-13-240240-tft)
3. [I2C 接口屏 — SSD1306 0.96" 128×64 OLED](#3-i2c-接口屏--ssd1306-096-12864-oled)
4. [并行 RGB 接口屏 — AT070TN92 7" 800×480 TFT](#4-并行-rgb-接口屏--at070tn92-7-800480-tft)
5. [MIPI DSI 接口屏 — ILI9881C 5" 720×1280 IPS](#5-mipi-dsi-接口屏--ili9881c-5-7201280-ips)
6. [LVDS 接口屏 — HJ070NA-13A 7" 1024×600 IPS](#6-lvds-接口屏--hj070na-13a-7-1024600-ips)
7. [HDMI 接口屏 — 通用 7" 1024×600 HDMI 显示屏](#7-hdmi-接口屏--通用-7-1024600-hdmi-显示屏)
8. [各接口对比总结](#8-各接口对比总结)

---

## 1. 屏幕接口类型总览

| 接口类型 | 典型分辨率 | 速率 | 信号线数 | 典型应用 | 驱动复杂度 |
|---------|-----------|------|---------|---------|-----------|
| SPI | ≤320×240 | ≤50MHz | 4~6 根 | 小型仪表、IoT 设备 | ★★☆☆☆ |
| I2C | ≤128×64 | ≤400kHz | 2 根 | OLED 小屏、状态显示 | ★☆☆☆☆ |
| 并行 RGB | ≤1024×600 | 像素时钟≤50MHz | 20~30 根 | 中型 TFT 面板 | ★★★☆☆ |
| MIPI DSI | ≤2K/4K | 1~2.5Gbps/lane | 差分对 2~10 根 | 手机屏、高分辨率面板 | ★★★★☆ |
| LVDS | ≤1920×1200 | ≤1Gbps | 差分对 8~10 根 | 工控屏、车载屏 | ★★★☆☆ |
| HDMI | ≤4K@60 | TMDS | 标准接口 | 通用显示器、教学 | ★☆☆☆☆ |

### 开发板接口支持情况

| 接口 | Orange Pi Zero 2 (H616) | Orange Pi 5 Max (RK3588) |
|------|------------------------|--------------------------|
| SPI | ✅ 40pin GPIO 引出 SPI1 | ✅ 40pin GPIO 引出 SPI |
| I2C | ✅ I2C1/I2C2 | ✅ I2C2/I2C3/I2C5 |
| 并行 RGB | ⚠️ H616 有 RGB 接口但 Zero 2 未引出 | ❌ RK3588 不支持并行 RGB |
| MIPI DSI | ❌ Zero 2 未引出 DSI | ✅ 板载 30pin DSI 连接器 |
| LVDS | ❌ | ⚠️ 需通过 DSI-to-LVDS 桥接芯片 |
| HDMI | ✅ 1× HDMI 2.0（4K@60） | ✅ 2× HDMI 2.1（8K@60） |

---

## 2. SPI 接口屏 — ST7789V 1.3" 240×240 TFT

### 2.1 屏幕简介

- 驱动 IC：Sitronix ST7789V
- 尺寸：1.3 英寸（也有 1.14"、1.54"、2.0" 等变体）
- 分辨率：240×240（或 240×320）
- 接口：4 线 SPI（SCLK、MOSI、CS、DC），无需 MISO
- 色深：RGB565（16bit）/ RGB666（18bit）
- 工作电压：2.4V~3.6V（IO），2.4V~3.6V（模拟）
- SPI 时钟：最高 62.5MHz（写入），实际建议 40~50MHz
- 背光：LED 背光，需外部 PWM 或 GPIO 控制

### 2.2 硬件连接

#### 2.2.1 Orange Pi Zero 2 引脚分配

Orange Pi Zero 2 的 40pin GPIO 引出了 SPI1 控制器：

```
ST7789V 模块引脚        Orange Pi Zero 2 (40pin)        说明
─────────────────────────────────────────────────────────────
VCC  ──────────────── Pin 1  (3.3V)                    电源
GND  ──────────────── Pin 6  (GND)                     地
SCL (SCLK) ────────── Pin 23 (SPI1_CLK / PC0)          SPI 时钟
SDA (MOSI) ────────── Pin 19 (SPI1_MOSI / PC2)         SPI 数据
CS   ──────────────── Pin 24 (SPI1_CS0 / PC3)          片选
DC (A0/RS) ────────── Pin 18 (PC7 / GPIO)              数据/命令选择
RST  ──────────────── Pin 22 (PC8 / GPIO)              复位
BLK  ──────────────── Pin 16 (PC4 / GPIO 或 PWM)       背光控制
```

#### 2.2.2 接线示意图

```
Orange Pi Zero 2                          ST7789V 模块
┌──────────────────┐                     ┌──────────┐
│ Pin1   3.3V      ├─────────────────────┤ VCC      │
│ Pin6   GND       ├─────────────────────┤ GND      │
│ Pin23  SPI1_CLK  ├─────────────────────┤ SCL      │
│ Pin19  SPI1_MOSI ├─────────────────────┤ SDA      │
│ Pin24  SPI1_CS0  ├─────────────────────┤ CS       │
│ Pin18  PC7(GPIO) ├─────────────────────┤ DC       │
│ Pin22  PC8(GPIO) ├─────────────────────┤ RST      │
│ Pin16  PC4(GPIO) ├─────────────────────┤ BLK      │
└──────────────────┘                     └──────────┘
```

#### 2.2.3 硬件注意事项

1. 电平匹配：ST7789V 模块通常自带 3.3V LDO，可直接接 3.3V；如果模块标注 5V 输入，内部有降压
2. SPI 走线：SCLK 和 MOSI 走线尽量短（<15cm），避免与其他高速信号平行走线
3. 背光电流：LED 背光典型电流 20~40mA，GPIO 直驱可能不够，建议通过三极管（如 S8050）驱动
4. 无需 MISO：ST7789V 的 SPI 写入不需要 MISO 线，读取寄存器时才需要（调试用）

### 2.3 软件驱动开发

#### 2.3.1 方案选择

在嵌入式 Linux 上驱动 SPI 屏有三种方案：

| 方案 | 说明 | 适用场景 |
|------|------|---------|
| fbtft（内核模块） | 内核自带的 SPI TFT 驱动框架 | 快速验证，已有 ST7789V 支持 |
| DRM tiny（drm_mipi_dbi） | 现代 DRM 子系统的 SPI 小屏驱动 | 推荐，主线内核方向 |
| 用户态 spidev | 通过 /dev/spidevX.Y 在用户态直接操作 | 裸驱动开发学习 |

本文以 fbtft + DRM tiny 两种方案分别说明。

#### 2.3.2 方案一：fbtft 驱动（快速上手）

fbtft 是 Linux 内核 staging 目录下的 SPI/并行 TFT 驱动框架，已内置 ST7789V 支持。

##### 设备树配置

在 Orange Pi Zero 2 的设备树（`sun50i-h616-orangepi-zero2.dts`）中添加：

```dts
&spi1 {
    status = "okay";
    #address-cells = <1>;
    #size-cells = <0>;
    pinctrl-names = "default";
    pinctrl-0 = <&spi1_pins &spi1_cs0_pin>;

    st7789v: st7789v@0 {
        compatible = "sitronix,st7789v";
        reg = <0>;                          /* CS0 */
        spi-max-frequency = <40000000>;     /* 40MHz */
        rotate = <0>;                       /* 旋转角度：0/90/180/270 */
        width = <240>;
        height = <240>;
        fps = <60>;
        buswidth = <8>;                     /* 8-bit SPI */

        dc-gpios = <&pio 2 7 GPIO_ACTIVE_HIGH>;     /* PC7 */
        reset-gpios = <&pio 2 8 GPIO_ACTIVE_LOW>;   /* PC8 */
        led-gpios = <&pio 2 4 GPIO_ACTIVE_HIGH>;    /* PC4 背光 */

        debug = <0>;                        /* 调试级别 0~7 */
    };
};
```

##### 内核配置

```
CONFIG_STAGING=y
CONFIG_FB_TFT=y
CONFIG_FB_TFT_ST7789V=y
# 或者使用通用 fbtft_device 模块
CONFIG_FB_TFT_FBTFT_DEVICE=m
```

##### 编译与加载

```bash
# 编译内核（使用 Orange Pi Zero 2 SDK）
cd linux-orangepi
export ARCH=arm64
export CROSS_COMPILE=aarch64-linux-gnu-
make sun50i_defconfig
# 启用 fbtft
scripts/config --enable CONFIG_STAGING
scripts/config --enable CONFIG_FB_TFT
scripts/config --module CONFIG_FB_TFT_ST7789V
make -j$(nproc) Image dtbs modules

# 部署后加载模块
sudo modprobe fb_st7789v

# 验证
ls /dev/fb*
# 应出现 /dev/fb1（或 fb0 如果没有其他 framebuffer）
cat /dev/urandom > /dev/fb1    # 屏幕应显示随机噪点
```

#### 2.3.3 方案二：DRM tiny 驱动（推荐）

DRM tiny 是基于 `drm_mipi_dbi` 的现代驱动框架，支持 DRM/KMS，是主线内核推荐方案。

内核中已有 `panel-mipi-dbi-spi` 通用驱动，也有专用的 `st7789_panel` 驱动。

##### 设备树配置（DRM 方式）

```dts
&spi1 {
    status = "okay";
    #address-cells = <1>;
    #size-cells = <0>;

    display@0 {
        compatible = "jianda,jd-t18003-t01";  /* ST7789V 兼容 */
        reg = <0>;
        spi-max-frequency = <32000000>;

        dc-gpios = <&pio 2 7 GPIO_ACTIVE_HIGH>;
        reset-gpios = <&pio 2 8 GPIO_ACTIVE_LOW>;
        backlight = <&backlight_lcd>;

        rotation = <0>;
    };
};

backlight_lcd: backlight-lcd {
    compatible = "gpio-backlight";
    gpios = <&pio 2 4 GPIO_ACTIVE_HIGH>;
    default-on;
};
```

##### 内核配置

```
CONFIG_DRM=y
CONFIG_DRM_SIMPLEDRM=y
CONFIG_DRM_SUN4I=y          # Allwinner 显示引擎
CONFIG_DRM_MIPI_DBI=y
CONFIG_DRM_TINY_ST7789V=y   # 或 CONFIG_TINYDRM_ST7789V
CONFIG_BACKLIGHT_GPIO=y
```

#### 2.3.4 方案三：用户态 spidev 裸驱动（学习用）

通过 `/dev/spidevX.Y` 直接在用户态控制 ST7789V，适合理解 SPI 屏驱动原理。

##### 设备树启用 spidev

```dts
&spi1 {
    status = "okay";
    #address-cells = <1>;
    #size-cells = <0>;

    spidev@0 {
        compatible = "rohm,dh2228fv";  /* spidev 通用 compatible */
        reg = <0>;
        spi-max-frequency = <40000000>;
    };
};
```

##### 用户态驱动代码（C 语言）

```c
/*
 * st7789v_spidev.c — 用户态 SPI 驱动 ST7789V 示例
 * 编译：aarch64-linux-gnu-gcc -o st7789v_spidev st7789v_spidev.c
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>

#define SPI_DEVICE  "/dev/spidev1.0"
#define GPIO_BASE   "/sys/class/gpio"
#define DC_GPIO     71      /* PC7 = 2*32 + 7 = 71 */
#define RST_GPIO    72      /* PC8 = 2*32 + 8 = 72 */
#define BLK_GPIO    68      /* PC4 = 2*32 + 4 = 68 */

#define WIDTH       240
#define HEIGHT      240

static int spi_fd;

/* GPIO 操作辅助函数 */
static void gpio_export(int pin)
{
    char buf[64];
    int fd = open(GPIO_BASE "/export", O_WRONLY);
    if (fd < 0) return;
    int len = snprintf(buf, sizeof(buf), "%d", pin);
    write(fd, buf, len);
    close(fd);
}

static void gpio_direction(int pin, const char *dir)
{
    char path[128];
    snprintf(path, sizeof(path), GPIO_BASE "/gpio%d/direction", pin);
    int fd = open(path, O_WRONLY);
    if (fd < 0) return;
    write(fd, dir, strlen(dir));
    close(fd);
}

static void gpio_set(int pin, int value)
{
    char path[128];
    snprintf(path, sizeof(path), GPIO_BASE "/gpio%d/value", pin);
    int fd = open(path, O_WRONLY);
    if (fd < 0) return;
    write(fd, value ? "1" : "0", 1);
    close(fd);
}

/* SPI 写入 */
static void spi_write(const uint8_t *data, size_t len)
{
    struct spi_ioc_transfer tr = {
        .tx_buf = (unsigned long)data,
        .len = len,
        .speed_hz = 40000000,
        .bits_per_word = 8,
    };
    ioctl(spi_fd, SPI_IOC_MESSAGE(1), &tr);
}

/* 发送命令（DC=0） */
static void st7789_cmd(uint8_t cmd)
{
    gpio_set(DC_GPIO, 0);
    spi_write(&cmd, 1);
}

/* 发送数据（DC=1） */
static void st7789_data(const uint8_t *data, size_t len)
{
    gpio_set(DC_GPIO, 1);
    spi_write(data, len);
}

static void st7789_data_byte(uint8_t val)
{
    st7789_data(&val, 1);
}

/* 硬件复位 */
static void st7789_reset(void)
{
    gpio_set(RST_GPIO, 1);
    usleep(10000);
    gpio_set(RST_GPIO, 0);
    usleep(10000);
    gpio_set(RST_GPIO, 1);
    usleep(120000);
}

/* ST7789V 初始化序列 */
static void st7789_init(void)
{
    st7789_reset();

    st7789_cmd(0x11);           /* Sleep Out */
    usleep(120000);

    st7789_cmd(0x36);           /* Memory Access Control */
    st7789_data_byte(0x00);     /* 正常方向 */

    st7789_cmd(0x3A);           /* Pixel Format */
    st7789_data_byte(0x55);     /* RGB565 */

    st7789_cmd(0xB2);           /* Porch Setting */
    st7789_data_byte(0x0C);
    st7789_data_byte(0x0C);
    st7789_data_byte(0x00);
    st7789_data_byte(0x33);
    st7789_data_byte(0x33);

    st7789_cmd(0xB7);           /* Gate Control */
    st7789_data_byte(0x35);

    st7789_cmd(0xBB);           /* VCOM Setting */
    st7789_data_byte(0x19);

    st7789_cmd(0xC0);           /* LCM Control */
    st7789_data_byte(0x2C);

    st7789_cmd(0xC2);           /* VDV and VRH Command Enable */
    st7789_data_byte(0x01);

    st7789_cmd(0xC3);           /* VRH Set */
    st7789_data_byte(0x12);

    st7789_cmd(0xC4);           /* VDV Set */
    st7789_data_byte(0x20);

    st7789_cmd(0xC6);           /* Frame Rate Control */
    st7789_data_byte(0x0F);     /* 60Hz */

    st7789_cmd(0xD0);           /* Power Control 1 */
    st7789_data_byte(0xA4);
    st7789_data_byte(0xA1);

    st7789_cmd(0x21);           /* Display Inversion On */
    st7789_cmd(0x29);           /* Display On */
}

/* 设置显示窗口 */
static void st7789_set_window(uint16_t x0, uint16_t y0,
                               uint16_t x1, uint16_t y1)
{
    uint8_t data[4];

    st7789_cmd(0x2A);           /* Column Address Set */
    data[0] = x0 >> 8; data[1] = x0 & 0xFF;
    data[2] = x1 >> 8; data[3] = x1 & 0xFF;
    st7789_data(data, 4);

    st7789_cmd(0x2B);           /* Row Address Set */
    data[0] = y0 >> 8; data[1] = y0 & 0xFF;
    data[2] = y1 >> 8; data[3] = y1 & 0xFF;
    st7789_data(data, 4);

    st7789_cmd(0x2C);           /* Memory Write */
}

/* 填充纯色 */
static void st7789_fill(uint16_t color)
{
    st7789_set_window(0, 0, WIDTH - 1, HEIGHT - 1);

    uint16_t *buf = malloc(WIDTH * HEIGHT * 2);
    for (int i = 0; i < WIDTH * HEIGHT; i++)
        buf[i] = color;

    gpio_set(DC_GPIO, 1);
    spi_write((uint8_t *)buf, WIDTH * HEIGHT * 2);
    free(buf);
}

int main(void)
{
    /* 初始化 GPIO */
    gpio_export(DC_GPIO);
    gpio_export(RST_GPIO);
    gpio_export(BLK_GPIO);
    usleep(100000);
    gpio_direction(DC_GPIO, "out");
    gpio_direction(RST_GPIO, "out");
    gpio_direction(BLK_GPIO, "out");

    /* 打开背光 */
    gpio_set(BLK_GPIO, 1);

    /* 打开 SPI 设备 */
    spi_fd = open(SPI_DEVICE, O_RDWR);
    if (spi_fd < 0) {
        perror("open spi");
        return 1;
    }

    uint8_t mode = SPI_MODE_0;
    uint8_t bits = 8;
    uint32_t speed = 40000000;
    ioctl(spi_fd, SPI_IOC_WR_MODE, &mode);
    ioctl(spi_fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
    ioctl(spi_fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);

    /* 初始化屏幕 */
    st7789_init();

    /* 显示红色 */
    st7789_fill(0xF800);   /* RGB565: Red */
    sleep(1);

    /* 显示绿色 */
    st7789_fill(0x07E0);   /* RGB565: Green */
    sleep(1);

    /* 显示蓝色 */
    st7789_fill(0x001F);   /* RGB565: Blue */

    close(spi_fd);
    return 0;
}
```

### 2.4 调试方法

```bash
# 检查 SPI 设备是否存在
ls /dev/spidev*

# 查看 SPI 控制器状态
cat /sys/class/spi_master/spi1/statistics/transfers

# fbtft 调试信息
dmesg | grep -i "fbtft\|st7789\|fb_"

# 测试 framebuffer
fbset -i -fb /dev/fb1
cat /dev/urandom > /dev/fb1

# 使用 fbi 显示图片（需安装 fbi）
fbi -d /dev/fb1 -T 1 -noverbose test.png
```

---

## 3. I2C 接口屏 — SSD1306 0.96" 128×64 OLED

### 3.1 屏幕简介

- 驱动 IC：Solomon Systech SSD1306
- 尺寸：0.96 英寸（也有 0.91"、1.3" 等变体）
- 分辨率：128×64（单色）
- 接口：I2C（也有 SPI 版本，本文以 I2C 为例）
- I2C 地址：0x3C（SA0=0）或 0x3D（SA0=1）
- 工作电压：3.3V~5V（模块自带 LDO）
- 显示类型：OLED 自发光，无需背光
- 刷新率：最高约 60fps（I2C 400kHz 下实际约 20fps）

### 3.2 硬件连接

#### 3.2.1 Orange Pi Zero 2 引脚分配

```
SSD1306 模块引脚        Orange Pi Zero 2 (40pin)        说明
─────────────────────────────────────────────────────────────
VCC  ──────────────── Pin 1  (3.3V)                    电源
GND  ──────────────── Pin 9  (GND)                     地
SCL  ──────────────── Pin 5  (I2C1_SCL / PH6)          I2C 时钟
SDA  ──────────────── Pin 3  (I2C1_SDA / PH5)          I2C 数据
```

> 仅需 4 根线，是最简单的屏幕接口。

#### 3.2.2 接线示意图

```
Orange Pi Zero 2                          SSD1306 模块
┌──────────────────┐                     ┌──────────┐
│ Pin1   3.3V      ├─────────────────────┤ VCC      │
│ Pin9   GND       ├─────────────────────┤ GND      │
│ Pin5   I2C1_SCL  ├─────────────────────┤ SCL      │
│ Pin3   I2C1_SDA  ├─────────────────────┤ SDA      │
└──────────────────┘                     └──────────┘
```

#### 3.2.3 硬件注意事项

1. 上拉电阻：I2C 总线需要上拉电阻（4.7kΩ 到 VCC），大多数 SSD1306 模块已内置
2. 走线长度：I2C 走线不宜超过 30cm（400kHz 下），超过需降低速率或加缓冲器
3. 地址冲突：同一 I2C 总线上不能有两个相同地址的设备，双屏需一个接 0x3C 一个接 0x3D
4. 电平：H616 的 I2C IO 电平为 3.3V，SSD1306 模块兼容 3.3V/5V

### 3.3 软件驱动开发

#### 3.3.1 方案选择

| 方案 | 说明 | 适用场景 |
|------|------|---------|
| ssd1306 内核驱动 | DRM 子系统下的 ssd1306 驱动 | 推荐，主线内核已支持 |
| 用户态 i2c-dev | 通过 /dev/i2c-X 在用户态操作 | 学习 I2C 协议 |
| Python luma.oled | Python 库，基于 i2c-dev | 快速原型开发 |

#### 3.3.2 方案一：内核 DRM 驱动（推荐）

Linux 主线内核已包含 `ssd1306` 的 DRM 驱动（`drivers/gpu/drm/tiny/ssd1306.c`）。

##### 设备树配置

```dts
&i2c1 {
    status = "okay";
    clock-frequency = <400000>;     /* 400kHz Fast Mode */

    ssd1306: oled@3c {
        compatible = "solomon,ssd1306fb-i2c";
        reg = <0x3c>;
        solomon,width = <128>;
        solomon,height = <64>;
        solomon,page-offset = <0>;
        solomon,com-invdir;             /* COM 扫描方向，根据屏幕方向调整 */
        solomon,com-seq;                /* COM 顺序扫描 */
    };
};
```

##### 内核配置

```
CONFIG_DRM=y
CONFIG_DRM_SSD1307=y            # 注意：ssd1306 和 ssd1307 共用此驱动
CONFIG_I2C_CHARDEV=y            # 可选，用于 i2c-tools 调试
```

##### 验证

```bash
# 检查 I2C 设备
i2cdetect -y 1
# 应在 0x3c 位置显示设备

# 查看 DRM connector
cat /sys/class/drm/card0-Unknown-1/status

# framebuffer 测试
cat /dev/urandom > /dev/fb0
```

#### 3.3.3 方案二：用户态 I2C 驱动（学习用）

```c
/*
 * ssd1306_i2c.c — 用户态 I2C 驱动 SSD1306 示例
 * 编译：aarch64-linux-gnu-gcc -o ssd1306_i2c ssd1306_i2c.c
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>

#define I2C_DEVICE  "/dev/i2c-1"
#define SSD1306_ADDR 0x3C
#define WIDTH   128
#define HEIGHT  64
#define PAGES   (HEIGHT / 8)

static int i2c_fd;
static uint8_t framebuffer[WIDTH * PAGES];  /* 128×8 = 1024 bytes */

/* 发送命令 */
static void ssd1306_cmd(uint8_t cmd)
{
    uint8_t buf[2] = { 0x00, cmd };  /* Co=0, D/C#=0 (command) */
    write(i2c_fd, buf, 2);
}

/* 发送数据 */
static void ssd1306_data(const uint8_t *data, size_t len)
{
    uint8_t *buf = malloc(len + 1);
    buf[0] = 0x40;                   /* Co=0, D/C#=1 (data) */
    memcpy(buf + 1, data, len);
    write(i2c_fd, buf, len + 1);
    free(buf);
}

/* SSD1306 初始化 */
static void ssd1306_init(void)
{
    ssd1306_cmd(0xAE);      /* Display OFF */
    ssd1306_cmd(0xD5);      /* Set Display Clock Divide Ratio */
    ssd1306_cmd(0x80);
    ssd1306_cmd(0xA8);      /* Set Multiplex Ratio */
    ssd1306_cmd(0x3F);      /* 64 行 */
    ssd1306_cmd(0xD3);      /* Set Display Offset */
    ssd1306_cmd(0x00);
    ssd1306_cmd(0x40);      /* Set Display Start Line = 0 */
    ssd1306_cmd(0x8D);      /* Charge Pump Setting */
    ssd1306_cmd(0x14);      /* Enable charge pump */
    ssd1306_cmd(0x20);      /* Set Memory Addressing Mode */
    ssd1306_cmd(0x00);      /* Horizontal Addressing Mode */
    ssd1306_cmd(0xA1);      /* Set Segment Re-map (左右镜像) */
    ssd1306_cmd(0xC8);      /* Set COM Output Scan Direction (上下翻转) */
    ssd1306_cmd(0xDA);      /* Set COM Pins Hardware Configuration */
    ssd1306_cmd(0x12);
    ssd1306_cmd(0x81);      /* Set Contrast Control */
    ssd1306_cmd(0xCF);
    ssd1306_cmd(0xD9);      /* Set Pre-charge Period */
    ssd1306_cmd(0xF1);
    ssd1306_cmd(0xDB);      /* Set VCOMH Deselect Level */
    ssd1306_cmd(0x40);
    ssd1306_cmd(0xA4);      /* Entire Display ON (follow RAM) */
    ssd1306_cmd(0xA6);      /* Set Normal Display (非反显) */
    ssd1306_cmd(0xAF);      /* Display ON */
}

/* 刷新显示 */
static void ssd1306_flush(void)
{
    ssd1306_cmd(0x21);      /* Set Column Address */
    ssd1306_cmd(0x00);      /* Start = 0 */
    ssd1306_cmd(0x7F);      /* End = 127 */
    ssd1306_cmd(0x22);      /* Set Page Address */
    ssd1306_cmd(0x00);      /* Start = 0 */
    ssd1306_cmd(0x07);      /* End = 7 */

    ssd1306_data(framebuffer, sizeof(framebuffer));
}

/* 画点 */
static void ssd1306_pixel(int x, int y, int on)
{
    if (x < 0 || x >= WIDTH || y < 0 || y >= HEIGHT) return;
    if (on)
        framebuffer[x + (y / 8) * WIDTH] |= (1 << (y % 8));
    else
        framebuffer[x + (y / 8) * WIDTH] &= ~(1 << (y % 8));
}

int main(void)
{
    i2c_fd = open(I2C_DEVICE, O_RDWR);
    if (i2c_fd < 0) { perror("open i2c"); return 1; }

    if (ioctl(i2c_fd, I2C_SLAVE, SSD1306_ADDR) < 0) {
        perror("ioctl I2C_SLAVE");
        return 1;
    }

    ssd1306_init();

    /* 清屏 */
    memset(framebuffer, 0, sizeof(framebuffer));
    ssd1306_flush();

    /* 画一个矩形边框 */
    for (int x = 0; x < WIDTH; x++) {
        ssd1306_pixel(x, 0, 1);
        ssd1306_pixel(x, HEIGHT - 1, 1);
    }
    for (int y = 0; y < HEIGHT; y++) {
        ssd1306_pixel(0, y, 1);
        ssd1306_pixel(WIDTH - 1, y, 1);
    }

    /* 画对角线 */
    for (int i = 0; i < HEIGHT; i++) {
        ssd1306_pixel(i * WIDTH / HEIGHT, i, 1);
        ssd1306_pixel(WIDTH - 1 - i * WIDTH / HEIGHT, i, 1);
    }

    ssd1306_flush();
    printf("SSD1306 display test done.\n");

    close(i2c_fd);
    return 0;
}
```

#### 3.3.4 方案三：Python luma.oled（快速原型）

```bash
# 安装
pip3 install luma.oled

# 启用 I2C（确保设备树已配置）
ls /dev/i2c-*
```

```python
#!/usr/bin/env python3
"""SSD1306 OLED 显示测试 — luma.oled"""
from luma.core.interface.serial import i2c
from luma.oled.device import ssd1306
from PIL import Image, ImageDraw, ImageFont

serial = i2c(port=1, address=0x3C)
device = ssd1306(serial, width=128, height=64)

with Image.new("1", (128, 64)) as img:
    draw = ImageDraw.Draw(img)
    draw.rectangle((0, 0, 127, 63), outline="white")
    draw.text((10, 20), "Hello OPi!", fill="white")
    device.display(img)
```

### 3.4 调试方法

```bash
# 扫描 I2C 总线
i2cdetect -y 1

# 读取 SSD1306 寄存器（不推荐频繁读取）
i2cget -y 1 0x3c 0x00

# 内核日志
dmesg | grep -i "ssd130\|oled\|i2c"

# 检查 I2C 总线速率
cat /sys/class/i2c-adapter/i2c-1/of_node/clock-frequency
```

---

## 4. 并行 RGB 接口屏 — AT070TN92 7" 800×480 TFT

### 4.1 屏幕简介

- 驱动 IC：内置行/列驱动（无需外部驱动 IC）
- 尺寸：7 英寸
- 分辨率：800×480
- 接口：24-bit 并行 RGB（R[7:0] G[7:0] B[7:0] + HSYNC + VSYNC + DE + PCLK）
- 色深：RGB888（16.7M 色）
- 工作电压：3.3V（逻辑），LED 背光需独立驱动（典型 9.6V/20mA×4 串）
- 像素时钟：典型 33.3MHz
- 触摸：可选配电阻式或电容式触摸屏

> 注意：Orange Pi Zero 2（H616）的 RGB 接口未引出到排针，此部分以通用 Allwinner 平台（如 A64/H3/H5）为例说明原理。
> Orange Pi 5 Max（RK3588）不支持并行 RGB 输出。

### 4.2 硬件连接

#### 4.2.1 典型 RGB 接口引脚（50pin FPC）

AT070TN92 使用 50pin 0.5mm FPC 连接器，典型引脚定义：

| Pin | 信号 | Pin | 信号 |
|-----|------|-----|------|
| 1 | GND | 26 | B3 |
| 2 | VCC (3.3V) | 27 | B4 |
| 3 | VCC (3.3V) | 28 | B5 |
| 4 | R0 | 29 | B6 |
| 5 | R1 | 30 | B7 |
| 6 | R2 | 31 | GND |
| 7 | R3 | 32 | DCLK (Pixel Clock) |
| 8 | R4 | 33 | DE (Data Enable) |
| 9 | R5 | 34 | HSYNC |
| 10 | R6 | 35 | VSYNC |
| 11 | R7 | 36 | GND |
| 12 | GND | 37~40 | NC |
| 13 | G0 | 41 | NC |
| 14 | G1 | 42 | LED_A (背光阳极) |
| 15 | G2 | 43 | LED_A |
| 16 | G3 | 44 | LED_A |
| 17 | G4 | 45 | GND |
| 18 | G5 | 46~48 | NC |
| 19 | G6 | 49 | LED_K (背光阴极) |
| 20 | G7 | 50 | LED_K |
| 21 | GND | | |
| 22 | B0 | | |
| 23 | B1 | | |
| 24 | B2 | | |
| 25 | GND | | |

#### 4.2.2 接线框图

```
SoC (Allwinner)                              AT070TN92
┌──────────────────┐                        ┌──────────────┐
│ LCD_D[23:16]     ├── R[7:0] ─────────────┤ R0~R7        │
│ LCD_D[15:8]      ├── G[7:0] ─────────────┤ G0~G7        │
│ LCD_D[7:0]       ├── B[7:0] ─────────────┤ B0~B7        │
│ LCD_CLK          ├── DCLK ───────────────┤ DCLK         │
│ LCD_DE           ├── DE ─────────────────┤ DE           │
│ LCD_HSYNC        ├── HSYNC ──────────────┤ HSYNC        │
│ LCD_VSYNC        ├── VSYNC ──────────────┤ VSYNC        │
│                  │                        │              │
│ 3.3V             ├───────────────────────┤ VCC          │
│ GND              ├───────────────────────┤ GND          │
│                  │                        │              │
│ PWM / GPIO       ├──[背光驱动电路]───────┤ LED_A/LED_K  │
└──────────────────┘                        └──────────────┘
```

#### 4.2.3 背光驱动电路

AT070TN92 的 LED 背光为 4 串 LED（每串约 3.2V，总计约 9.6V），需要恒流驱动：

```
方案一：专用背光驱动 IC（推荐）
┌─────────┐
│ AP3036  │── LED_A (Pin42~44)
│ (升压IC) │── LED_K (Pin49~50)
│ EN ←────│── GPIO（使能）
│ DIM ←───│── PWM（调光）
└─────────┘

方案二：简易电路（仅用于测试）
VCC_12V ──[100Ω 限流电阻]──── LED_A
                                LED_K ──── GND
```

#### 4.2.4 硬件注意事项

1. 信号线数量多：RGB888 需要 24 根数据线 + 4 根控制线，PCB 布线复杂
2. 像素时钟完整性：DCLK 走线需注意阻抗匹配（50Ω），加串联电阻（33Ω）
3. 背光安全：LED 背光必须使用恒流驱动，直接接电源会烧毁 LED
4. EMI：大量并行高速信号会产生较强 EMI，注意屏蔽和滤波

### 4.3 软件驱动开发

#### 4.3.1 设备树配置（Allwinner 平台示例）

```dts
/* 以 Allwinner H3/H5/A64 为例 */

/* LCD 控制器 */
&de {
    status = "okay";
};

&tcon0 {
    status = "okay";
    pinctrl-names = "default";
    pinctrl-0 = <&lcd_rgb888_pins>;

    port {
        tcon0_out_lcd: endpoint {
            remote-endpoint = <&panel_input>;
        };
    };
};

/* Panel 定义 */
panel: panel {
    compatible = "innolux,at070tn92";   /* 内核已有此 panel 驱动 */
    /* 或使用通用 simple-panel：
     * compatible = "panel-dpi";
     */

    power-supply = <&reg_vcc3v3>;
    enable-gpios = <&pio 3 24 GPIO_ACTIVE_HIGH>;  /* PD24 使能 */
    backlight = <&backlight>;

    port {
        panel_input: endpoint {
            remote-endpoint = <&tcon0_out_lcd>;
        };
    };

    /* 如果使用 panel-dpi，需要手动指定时序 */
    /*
    panel-timing {
        clock-frequency = <33300000>;
        hactive = <800>;
        vactive = <480>;
        hfront-porch = <210>;
        hback-porch = <46>;
        hsync-len = <20>;
        vfront-porch = <22>;
        vback-porch = <23>;
        vsync-len = <10>;
        hsync-active = <0>;
        vsync-active = <0>;
        de-active = <1>;
        pixelclk-active = <1>;
    };
    */
};

/* 背光 */
backlight: backlight {
    compatible = "pwm-backlight";
    pwms = <&pwm 0 50000 0>;       /* PWM0, 20kHz */
    brightness-levels = <0 4 8 16 32 64 128 255>;
    default-brightness-level = <6>;
    power-supply = <&reg_vcc5v>;
};
```

#### 4.3.2 内核配置

```
CONFIG_DRM=y
CONFIG_DRM_SUN4I=y              # Allwinner 显示引擎
CONFIG_DRM_SUN8I_MIXER=y        # Display Engine 2.0 mixer
CONFIG_DRM_SUN8I_TCON_TOP=y     # TCON 顶层
CONFIG_DRM_PANEL_SIMPLE=y       # 通用 simple panel 驱动
CONFIG_DRM_PANEL_INNOLUX_AT070TN92=y  # 或专用驱动
CONFIG_BACKLIGHT_PWM=y
```

#### 4.3.3 时序参数详解

AT070TN92 的关键时序参数（来自 datasheet）：

```
                    ┌──────── hactive (800) ────────┐
                    │                                │
    ┌───┐  ┌───────┤                                ├───────┐
    │HBP│  │ HSYNC │        有效像素数据              │  HFP  │
    │46 │  │  20   │                                │  210  │
    └───┘  └───────┤                                ├───────┘
                    │                                │
                    └────────────────────────────────┘
                    htotal = 800 + 210 + 20 + 46 = 1076

    垂直方向同理：
    vactive = 480, VFP = 22, VSYNC = 10, VBP = 23
    vtotal = 480 + 22 + 10 + 23 = 535

    像素时钟 = htotal × vtotal × fps = 1076 × 535 × 60 ≈ 33.3MHz
```

### 4.4 调试方法

```bash
# 查看显示引擎状态
cat /sys/kernel/debug/dri/0/summary

# 查看 connector 和 mode
modetest -M sun4i-drm -c

# 测试显示
modetest -M sun4i-drm -s <connector_id>@<crtc_id>:800x480

# 检查背光
cat /sys/class/backlight/backlight/brightness
echo 128 > /sys/class/backlight/backlight/brightness

# 检查 pinctrl（确认 RGB 引脚已配置）
cat /sys/kernel/debug/pinctrl/1c20800.pinctrl/pinmux-pins | grep lcd
```

---

## 5. MIPI DSI 接口屏 — ILI9881C 5" 720×1280 IPS

### 5.1 屏幕简介

- 驱动 IC：Ilitek ILI9881C
- 尺寸：5 英寸（也有 5.5"、6" 等变体）
- 分辨率：720×1280（HD）
- 接口：MIPI DSI（4-lane）
- 色深：RGB888（24bit）
- 工作电压：IOVCC 1.8V，VCI 2.8V
- 模式：Video Mode（Burst Mode）
- 背光：LED 背光，PWM 调光
- 触摸：通常配 GT911 或 FT5426 电容触摸屏（I2C 接口）

### 5.2 硬件连接（Orange Pi 5 Max）

#### 5.2.1 使用板载 30pin MIPI DSI 连接器

Orange Pi 5 Max 板载 30pin DSI FPC 座，ILI9881C 屏幕模组通常也是 30~40pin FPC。
需要一块 FPC 转接板或直接购买适配 Orange Pi 的屏幕模组。

```
Orange Pi 5 Max 30pin DSI          ILI9881C 屏幕模组（典型 40pin）
┌──────────────────┐              ┌──────────────────┐
│ Pin1,2  3.3V     ├──────────────┤ VCC (3.3V)       │
│ Pin3,4  1.8V     ├──────────────┤ IOVCC (1.8V)     │
│ Pin5    RST      ├──────────────┤ RESET            │
│ Pin7    GND      ├──────────────┤ GND              │
│                  │              │                  │
│ Pin8    D0N      ├──────────────┤ DSI_D0N          │
│ Pin9    D0P      ├──────────────┤ DSI_D0P          │
│ Pin11   D1N      ├──────────────┤ DSI_D1N          │
│ Pin12   D1P      ├──────────────┤ DSI_D1P          │
│ Pin14   CLKN     ├──────────────┤ DSI_CLKN         │
│ Pin15   CLKP     ├──────────────┤ DSI_CLKP         │
│ Pin17   D2N      ├──────────────┤ DSI_D2N          │
│ Pin18   D2P      ├──────────────┤ DSI_D2P          │
│ Pin20   D3N      ├──────────────┤ DSI_D3N          │
│ Pin21   D3P      ├──────────────┤ DSI_D3P          │
│                  │              │                  │
│ Pin23   TP_INT   ├──────────────┤ CTP_INT          │
│ Pin24   TP_RST   ├──────────────┤ CTP_RST          │
│ Pin25   TP_SDA   ├──────────────┤ CTP_SDA          │
│ Pin26   TP_SCL   ├──────────────┤ CTP_SCL          │
│                  │              │                  │
│ Pin29   BL_PWM   ├──────────────┤ LED_PWM          │
│ Pin30   BL_EN    ├──────────────┤ LED_EN           │
└──────────────────┘              └──────────────────┘
```

#### 5.2.2 硬件注意事项

1. Lane 数匹配：ILI9881C 使用 4-lane DSI，Orange Pi 5 Max 的 DSI0 支持 4-lane，完全匹配
2. 差分对走线：MIPI 差分对需等长匹配，对内误差 <5mil，对间误差 <100mil
3. 阻抗控制：差分阻抗 100Ω，单端阻抗 50Ω
4. 电源去耦：VCI 和 IOVCC 各加 10μF + 100nF 去耦电容
5. 触摸屏 I2C：GT911 触摸 IC 的 I2C 地址由 INT 引脚上电时的电平决定（0x5D 或 0x14）

### 5.3 软件驱动开发

#### 5.3.1 设备树配置（RK3588 / Orange Pi 5 Max）

```dts
/* 电源 regulator */
vcc3v3_lcd: vcc3v3-lcd {
    compatible = "regulator-fixed";
    regulator-name = "vcc3v3_lcd";
    regulator-min-microvolt = <3300000>;
    regulator-max-microvolt = <3300000>;
    regulator-boot-on;
    regulator-always-on;
};

vcc1v8_lcd: vcc1v8-lcd {
    compatible = "regulator-fixed";
    regulator-name = "vcc1v8_lcd";
    regulator-min-microvolt = <1800000>;
    regulator-max-microvolt = <1800000>;
    regulator-boot-on;
    regulator-always-on;
};

/* 背光 */
backlight_lcd: backlight-lcd {
    compatible = "pwm-backlight";
    pwms = <&pwm2 0 25000 0>;      /* PWM2, 40kHz */
    brightness-levels = <
        0  20  20  21  21  22  22  23
        23  24  24  25  25  26  26  27
        27  28  28  29  29  30  30  31
        31  32  32  33  33  34  34  35
        35  36  36  37  37  38  38  39
        40  41  42  43  44  45  46  47
        48  49  50  51  52  53  54  55
        56  57  58  59  60  61  62  63
        64  65  66  67  68  69  70  71
        72  73  74  75  76  77  78  79
        80  81  82  83  84  85  86  87
        88  89  90  91  92  93  94  95
        96  97  98  99 100 101 102 103
        104 105 106 107 108 109 110 111
        112 113 114 115 116 117 118 119
        120 121 122 123 124 125 126 127
        128 129 130 131 132 133 134 135
        136 137 138 139 140 141 142 143
        144 145 146 147 148 149 150 151
        152 153 154 155 156 157 158 159
        160 161 162 163 164 165 166 167
        168 169 170 171 172 173 174 175
        176 177 178 179 180 181 182 183
        184 185 186 187 188 189 190 191
        192 193 194 195 196 197 198 199
        200 201 202 203 204 205 206 207
        208 209 210 211 212 213 214 215
        216 217 218 219 220 221 222 223
        224 225 226 227 228 229 230 231
        232 233 234 235 236 237 238 239
        240 241 242 243 244 245 246 247
        248 249 250 251 252 253 254 255
    >;
    default-brightness-level = <200>;
    enable-gpios = <&gpio4 RK_PA3 GPIO_ACTIVE_HIGH>;
    status = "okay";
};

/* MIPI DCPHY0 */
&mipi_dcphy0 {
    status = "okay";
};

/* DSI0 控制器 */
&dsi0 {
    status = "okay";
    #address-cells = <1>;
    #size-cells = <0>;
    rockchip,lane-rate = <1000>;    /* 1Gbps per lane */

    panel@0 {
        compatible = "ilitek,ili9881c";
        reg = <0>;
        reset-gpios = <&gpio2 RK_PC1 GPIO_ACTIVE_LOW>;
        vci-supply = <&vcc3v3_lcd>;
        iovcc-supply = <&vcc1v8_lcd>;
        backlight = <&backlight_lcd>;

        port {
            panel_in_dsi0: endpoint {
                remote-endpoint = <&dsi0_out_panel>;
            };
        };
    };

    ports {
        port@1 {
            reg = <1>;
            dsi0_out_panel: endpoint {
                remote-endpoint = <&panel_in_dsi0>;
            };
        };
    };
};

/* VOP2 路由 */
&vp2 {
    status = "okay";
};

&route_dsi0 {
    status = "okay";
    connect = <&vp2_out_dsi0>;
};

/* 触摸屏（GT911） */
&i2c5 {
    status = "okay";
    clock-frequency = <400000>;

    gt911: touchscreen@5d {
        compatible = "goodix,gt911";
        reg = <0x5d>;
        interrupt-parent = <&gpio2>;
        interrupts = <RK_PC2 IRQ_TYPE_EDGE_FALLING>;
        reset-gpios = <&gpio2 RK_PC3 GPIO_ACTIVE_LOW>;
        irq-gpios = <&gpio2 RK_PC2 GPIO_ACTIVE_HIGH>;
        touchscreen-size-x = <720>;
        touchscreen-size-y = <1280>;
    };
};
```

#### 5.3.2 内核 Panel 驱动

ILI9881C 在主线内核中已有驱动（`drivers/gpu/drm/panel/panel-ilitek-ili9881c.c`），但可能不包含你的屏幕模组的初始化序列。通常需要根据屏厂提供的 init code 修改。

##### 驱动核心结构

```c
// drivers/gpu/drm/panel/panel-ilitek-ili9881c.c（关键部分）

/* ILI9881C 使用分页寄存器，通过 0xFF 命令切换页面 */
/* Page 切换命令 */
#define ILI9881C_SWITCH_PAGE(page) \
    {0xFF, 0x98, 0x81, (page)}

/* 初始化序列示例（需根据屏厂 init code 填写） */
static const struct ili9881c_instr ili9881c_init[] = {
    /* 切换到 Page 3 */
    ILI9881C_SWITCH_PAGE(0x03),
    /* GIP 设置 */
    {0x01, 0x00},
    {0x02, 0x00},
    {0x03, 0x73},
    {0x04, 0x00},
    {0x05, 0x00},
    {0x06, 0x0A},
    {0x07, 0x00},
    {0x08, 0x00},
    /* ... 省略大量寄存器配置 ... */

    /* 切换到 Page 4 — 电源设置 */
    ILI9881C_SWITCH_PAGE(0x04),
    {0x6C, 0x15},
    {0x6E, 0x2A},
    {0x6F, 0x33},
    {0x3A, 0x94},
    {0x8D, 0x1A},
    {0x87, 0xBA},
    {0x26, 0x76},
    {0xB2, 0xD1},
    /* ... */

    /* 切换到 Page 1 — 电压设置 */
    ILI9881C_SWITCH_PAGE(0x01),
    {0x22, 0x0A},
    {0x31, 0x00},
    {0x53, 0x78},
    {0x55, 0x78},
    {0x50, 0xA8},
    {0x51, 0xA8},
    /* Gamma 正向 */
    {0xA0, 0x08},
    {0xA1, 0x1A},
    /* ... */

    /* 切换到 Page 0 — 用户命令 */
    ILI9881C_SWITCH_PAGE(0x00),
    {0x35, 0x00},       /* TE On */
};

/* 显示模式定义 */
static const struct drm_display_mode ili9881c_default_mode = {
    .clock       = 62000,           /* 62MHz pixel clock */
    .hdisplay    = 720,
    .hsync_start = 720 + 10,       /* HFP */
    .hsync_end   = 720 + 10 + 20, /* HSW */
    .htotal      = 720 + 10 + 20 + 30, /* HBP */
    .vdisplay    = 1280,
    .vsync_start = 1280 + 10,     /* VFP */
    .vsync_end   = 1280 + 10 + 10, /* VSW */
    .vtotal      = 1280 + 10 + 10 + 20, /* VBP */
    .width_mm    = 62,
    .height_mm   = 110,
    .type        = DRM_MODE_TYPE_DRIVER | DRM_MODE_TYPE_PREFERRED,
};

/* probe 函数中的 DSI 配置 */
static int ili9881c_probe(struct mipi_dsi_device *dsi)
{
    /* ... */
    dsi->lanes = 4;
    dsi->format = MIPI_DSI_FMT_RGB888;
    dsi->mode_flags = MIPI_DSI_MODE_VIDEO |
                      MIPI_DSI_MODE_VIDEO_BURST |
                      MIPI_DSI_MODE_LPM |
                      MIPI_DSI_MODE_NO_EOT_PACKET;
    /* ... */
}
```

#### 5.3.3 内核配置

```
CONFIG_DRM=y
CONFIG_DRM_ROCKCHIP=y
CONFIG_ROCKCHIP_VOP2=y
CONFIG_ROCKCHIP_DW_MIPI_DSI=y
CONFIG_PHY_ROCKCHIP_SAMSUNG_DCPHY=y
CONFIG_DRM_PANEL_ILITEK_ILI9881C=y
CONFIG_TOUCHSCREEN_GOODIX=y         # GT911 触摸驱动
CONFIG_BACKLIGHT_PWM=y
```

### 5.4 调试方法

```bash
# 查看 DSI connector 状态
cat /sys/class/drm/card0-DSI-1/status
cat /sys/class/drm/card0-DSI-1/modes

# 查看 VOP 绑定
cat /sys/kernel/debug/dri/0/summary

# 内核日志
dmesg | grep -i "ili9881\|dsi\|mipi\|panel\|dcphy"

# 触摸屏调试
cat /dev/input/event*    # 触摸后查看事件
evtest /dev/input/event2 # 详细触摸事件

# 背光调节
echo 128 > /sys/class/backlight/backlight-lcd/brightness

# modetest 测试
modetest -M rockchip -s <connector_id>@<crtc_id>:720x1280
```

---

## 6. LVDS 接口屏 — HJ070NA-13A 7" 1024×600 IPS

### 6.1 屏幕简介

- 面板型号：Innolux HJ070NA-13A（群创光电）
- 尺寸：7 英寸
- 分辨率：1024×600
- 接口：单通道 LVDS（1-ch, 18-bit 或 24-bit）
- 色深：RGB666（262K 色）
- 工作电压：3.3V（逻辑），LED 背光独立供电
- 像素时钟：典型 51.2MHz
- LVDS 信号：4 对差分数据 + 1 对差分时钟（共 10 根线）
- 连接器：30pin 或 40pin FPC

### 6.2 LVDS 接口原理

LVDS（Low-Voltage Differential Signaling）将并行 RGB 数据编码为串行差分信号：

```
并行 RGB 数据                    LVDS 编码                    差分对
R[7:0] ─┐                    ┌──────────┐
G[7:0] ──┼── 7:1 串行化 ────│ LVDS TX  ├── TX0+/TX0-  (Data 0)
B[7:0] ──┤                   │ (如DS90C385)├── TX1+/TX1-  (Data 1)
HSYNC ───┤                   │          ├── TX2+/TX2-  (Data 2)
VSYNC ───┤                   │          ├── TX3+/TX3-  (Data 3，24-bit 时使用)
DE ──────┘                   │          ├── TXCLK+/TXCLK- (Clock)
                              └──────────┘
```

单通道 18-bit LVDS 使用 3 对数据 + 1 对时钟（8 根线）
单通道 24-bit LVDS 使用 4 对数据 + 1 对时钟（10 根线）

### 6.3 硬件连接（Orange Pi 5 Max + DSI-to-LVDS 桥接）

RK3588 没有原生 LVDS 输出，需要通过 DSI-to-LVDS 桥接芯片转换。

#### 6.3.1 桥接方案

常用 DSI-to-LVDS 桥接芯片：

| 芯片 | 厂商 | 输入 | 输出 | 特点 |
|------|------|------|------|------|
| SN65DSI83 | TI | 1~2 lane DSI | 单通道 LVDS | 最常用，成本低 |
| SN65DSI84 | TI | 1~4 lane DSI | 单通道 LVDS | 支持更高分辨率 |
| SN65DSI85 | TI | 1~4 lane DSI | 双通道 LVDS | 高分辨率双通道 |
| LT8912B | Lontium | 4 lane DSI | LVDS + HDMI | 多功能 |
| TC358775 | Toshiba | 4 lane DSI | 双通道 LVDS | 高端方案 |

本文以 SN65DSI83 为例。

#### 6.3.2 系统框图

```
Orange Pi 5 Max          SN65DSI83 转接板          HJ070NA-13A
┌──────────┐           ┌──────────────┐          ┌──────────┐
│ DSI0     │           │              │          │          │
│ D0P/D0N  ├───────────┤ DSI_D0+/-    │          │          │
│ D1P/D1N  ├───────────┤ DSI_D1+/-    │          │          │
│ CLKP/CLKN├───────────┤ DSI_CLK+/-   │          │          │
│          │           │              │          │          │
│          │           │ LVDS_A0+/-  ├──────────┤ RX0+/-   │
│          │           │ LVDS_A1+/-  ├──────────┤ RX1+/-   │
│          │           │ LVDS_A2+/-  ├──────────┤ RX2+/-   │
│          │           │ LVDS_ACLK+/-├──────────┤ RXCLK+/- │
│          │           │              │          │          │
│ I2C      ├───────────┤ SCL/SDA      │          │          │
│ RST GPIO ├───────────┤ EN           │          │          │
│ 3.3V     ├───────────┤ VCC          ├──────────┤ VCC      │
│ GND      ├───────────┤ GND          ├──────────┤ GND      │
└──────────┘           └──────────────┘          └──────────┘
```

#### 6.3.3 SN65DSI83 转接板设计要点

1. 电源：SN65DSI83 需要 1.8V（内核）和 3.3V（IO），从 Orange Pi 5 Max DSI 连接器取电
2. I2C 配置：SN65DSI83 通过 I2C 配置寄存器（地址 0x2C）
3. EN 引脚：使能引脚接 GPIO 控制
4. LVDS 输出：差分对走线阻抗 100Ω，等长匹配
5. 去耦电容：每个电源引脚加 100nF + 10μF

#### 6.3.4 LVDS 连接器引脚（HJ070NA-13A 30pin）

| Pin | 信号 | Pin | 信号 |
|-----|------|-----|------|
| 1 | GND | 16 | GND |
| 2 | VCC (3.3V) | 17 | GND |
| 3 | VCC (3.3V) | 18 | RX0- |
| 4 | NC | 19 | RX0+ |
| 5 | NC | 20 | GND |
| 6 | NC | 21 | RX1- |
| 7 | GND | 22 | RX1+ |
| 8 | LVDS_MODE | 23 | GND |
| 9 | NC | 24 | RX2- |
| 10 | NC | 25 | RX2+ |
| 11 | NC | 26 | GND |
| 12 | NC | 27 | RXCLK- |
| 13 | NC | 28 | RXCLK+ |
| 14 | NC | 29 | GND |
| 15 | GND | 30 | NC |

### 6.4 软件驱动开发

#### 6.4.1 设备树配置

SN65DSI83 在 Linux 内核中已有驱动（`drivers/gpu/drm/bridge/ti-sn65dsi83.c`）。

```dts
/* DSI0 控制器 */
&dsi0 {
    status = "okay";
    #address-cells = <1>;
    #size-cells = <0>;
    rockchip,lane-rate = <500>;

    bridge@0 {
        compatible = "ti,sn65dsi83";
        reg = <0>;
        enable-gpios = <&gpio2 RK_PB5 GPIO_ACTIVE_HIGH>;

        /* DSI 输入端口 */
        ports {
            #address-cells = <1>;
            #size-cells = <0>;

            port@0 {
                reg = <0>;
                sn65dsi83_in: endpoint {
                    remote-endpoint = <&dsi0_out_bridge>;
                    data-lanes = <1 2>;     /* 使用 2 lane */
                };
            };

            port@2 {
                reg = <2>;
                sn65dsi83_out: endpoint {
                    remote-endpoint = <&panel_in_lvds>;
                };
            };
        };
    };

    ports {
        port@1 {
            reg = <1>;
            dsi0_out_bridge: endpoint {
                remote-endpoint = <&sn65dsi83_in>;
            };
        };
    };
};

/* LVDS Panel */
panel_lvds: panel-lvds {
    compatible = "innolux,hj070na-13a";  /* 或 "panel-lvds" 通用 */
    width-mm = <150>;
    height-mm = <90>;
    data-mapping = "jeida-18";          /* JEIDA 18-bit 或 "vesa-24" */
    backlight = <&backlight_lcd>;
    power-supply = <&vcc3v3_lcd>;

    panel-timing {
        clock-frequency = <51200000>;   /* 51.2MHz */
        hactive = <1024>;
        vactive = <600>;
        hfront-porch = <160>;
        hback-porch = <160>;
        hsync-len = <10>;
        vfront-porch = <12>;
        vback-porch = <23>;
        vsync-len = <10>;
        hsync-active = <0>;
        vsync-active = <0>;
        de-active = <1>;
        pixelclk-active = <1>;
    };

    port {
        panel_in_lvds: endpoint {
            remote-endpoint = <&sn65dsi83_out>;
        };
    };
};
```

#### 6.4.2 内核配置

```
CONFIG_DRM=y
CONFIG_DRM_ROCKCHIP=y
CONFIG_ROCKCHIP_VOP2=y
CONFIG_ROCKCHIP_DW_MIPI_DSI=y
CONFIG_PHY_ROCKCHIP_SAMSUNG_DCPHY=y
CONFIG_DRM_TI_SN65DSI83=y          # DSI-to-LVDS 桥接驱动
CONFIG_DRM_PANEL_LVDS=y            # 通用 LVDS panel
CONFIG_DRM_PANEL_SIMPLE=y          # 或 simple panel（含 hj070na-13a）
CONFIG_BACKLIGHT_PWM=y
```

#### 6.4.3 SN65DSI83 寄存器配置（驱动自动完成）

SN65DSI83 的关键寄存器（供理解，驱动会自动配置）：

```
寄存器地址    名称                    典型值    说明
0x09        CLK_SRC                0x01     使用 DSI CLK
0x0A        CLK_DIV                0x05     时钟分频
0x0B        PLL_EN                 0x01     PLL 使能
0x10        DSI_LANES              0x26     2 lane DSI
0x12        DSI_EQ                 0x00     均衡设置
0x18        LVDS_VCOM              0x38     LVDS 共模电压
0x19        LVDS_LANE              0x00     单通道 LVDS
0x1A        LVDS_CM_ADJ            0x03     共模调整
0x1B        LVDS_DE_NEG            0x00     DE 极性
0x20        CHA_ACTIVE_LINE_LOW    0x80     水平有效像素低 8 位
0x21        CHA_ACTIVE_LINE_HIGH   0x04     水平有效像素高位 (1024=0x400)
0x24        CHA_VERT_ACTIVE_LOW    0x58     垂直有效行低 8 位
0x25        CHA_VERT_ACTIVE_HIGH   0x02     垂直有效行高位 (600=0x258)
0x28        CHA_SYNC_DELAY_LOW     0x20     同步延迟
0x2C        CHA_HSYNC_WIDTH_LOW    0x0A     HSYNC 宽度
0x30        CHA_VSYNC_WIDTH_LOW    0x0A     VSYNC 宽度
0x34        CHA_HBP                0xA0     HBP
0x36        CHA_VBP                0x17     VBP
0x38        CHA_HFP                0xA0     HFP
0x3A        CHA_VFP                0x0C     VFP
0x3C        CHA_TEST_PATTERN       0x00     测试图案（调试用）
0x0D        PLL_LOCK               只读     PLL 锁定状态
```

### 6.5 调试方法

```bash
# 检查 SN65DSI83 I2C 通信
i2cdetect -y 5     # 应在 0x2c 看到设备

# 读取 PLL 锁定状态
i2cget -y 5 0x2c 0x0d
# 返回 0x80 表示 PLL 已锁定

# 启用测试图案（调试用）
i2cset -y 5 0x2c 0x3c 0x11    # 彩条测试图案

# 内核日志
dmesg | grep -i "sn65dsi\|lvds\|bridge\|panel"

# DRM 状态
cat /sys/kernel/debug/dri/0/summary
modetest -M rockchip -c
```

---

## 7. HDMI 接口屏 — 通用 7" 1024×600 HDMI 显示屏

### 7.1 屏幕简介

- 典型型号：微雪 7inch HDMI LCD (C)、树莓派官方 7" HDMI 屏等
- 尺寸：7 英寸
- 分辨率：1024×600（也有 800×480、1280×800 等变体）
- 接口：标准 HDMI（Mini HDMI 或 Micro HDMI）
- 色深：RGB888（16.7M 色）
- 触摸：USB 电容触摸（HID 协议，免驱）
- 供电：5V/1A（通过 USB 或独立供电）
- 特点：即插即用，无需编写驱动，适合快速开发和教学

> HDMI 屏是最简单的方案 — 不需要写任何驱动代码，接上就能用。
> 但理解其工作原理对嵌入式显示开发仍有价值。

### 7.2 硬件连接

#### 7.2.1 Orange Pi Zero 2 连接

```
Orange Pi Zero 2                          7" HDMI 显示屏
┌──────────────────┐                     ┌──────────────┐
│ HDMI 接口        ├── HDMI 线 ──────────┤ HDMI 输入    │
│ USB Host         ├── USB 线 ───────────┤ USB 触摸     │
│ 5V 电源          │                     │              │
└──────────────────┘                     │ 5V/USB 供电  │
                                         └──────────────┘
```

#### 7.2.2 Orange Pi 5 Max 连接

Orange Pi 5 Max 有 2 个 HDMI 2.1 接口，支持最高 8K@60Hz：

```
Orange Pi 5 Max                           7" HDMI 显示屏
┌──────────────────┐                     ┌──────────────┐
│ HDMI0 (8K@60)    ├── HDMI 线 ──────────┤ HDMI 输入    │
│ 或 HDMI1         │                     │              │
│ USB 3.0 Host     ├── USB 线 ───────────┤ USB 触摸     │
└──────────────────┘                     └──────────────┘
```

#### 7.2.3 硬件注意事项

1. HDMI 线材：短距离（<1m）用普通 HDMI 线即可；长距离需用带信号放大的线材
2. 供电：部分 HDMI 屏需要独立 5V 供电，不能仅靠 HDMI 接口取电
3. 分辨率兼容：部分廉价 HDMI 屏的 EDID 信息不标准，可能需要手动配置分辨率
4. 触摸校准：USB HID 触摸通常免驱，但可能需要校准坐标映射

### 7.3 软件配置

HDMI 屏不需要编写驱动，但可能需要配置分辨率和触摸。

#### 7.3.1 分辨率配置

##### Orange Pi Zero 2（Allwinner H616）

编辑 `/boot/orangepiEnv.txt`：

```bash
# 查看当前支持的分辨率
cat /sys/class/drm/card0-HDMI-A-1/modes

# 如果屏幕分辨率未自动识别，手动指定
# 编辑 /boot/orangepiEnv.txt
disp_mode=1024x600p60
```

或通过内核命令行参数（`/boot/boot.cmd` 或 `/boot/armbianEnv.txt`）：

```bash
# 强制 HDMI 输出分辨率
extraargs=video=HDMI-A-1:1024x600@60
```

##### Orange Pi 5 Max（RK3588）

```bash
# 查看 HDMI connector 状态
cat /sys/class/drm/card0-HDMI-A-1/status    # connected
cat /sys/class/drm/card0-HDMI-A-1/modes     # 列出支持的分辨率

# 使用 xrandr 设置分辨率（桌面环境下）
xrandr --output HDMI-1 --mode 1024x600 --rate 60

# 如果 1024x600 不在标准模式列表中，需要添加自定义模式
cvt 1024 600 60
# 输出类似：Modeline "1024x600_60.00" 49.00 1024 1072 1168 1312 600 603 613 624 -hsync +vsync

xrandr --newmode "1024x600_60" 49.00 1024 1072 1168 1312 600 603 613 624 -hsync +vsync
xrandr --addmode HDMI-1 "1024x600_60"
xrandr --output HDMI-1 --mode "1024x600_60"
```

#### 7.3.2 EDID 覆盖（解决不识别问题）

部分廉价 HDMI 屏的 EDID 芯片数据有误，需要手动覆盖：

```bash
# 读取当前 EDID
cat /sys/class/drm/card0-HDMI-A-1/edid > /tmp/current_edid.bin
edid-decode /tmp/current_edid.bin

# 如果 EDID 有问题，可以使用自定义 EDID
# 1. 创建自定义 EDID 二进制文件（使用 edid-generator 工具）
# 2. 放到 /lib/firmware/edid/ 目录
# 3. 通过内核参数加载
#    drm.edid_firmware=HDMI-A-1:edid/custom_1024x600.bin
```

#### 7.3.3 USB 触摸屏配置

USB HID 触摸屏通常免驱，Linux 内核的 `hid-multitouch` 驱动自动处理：

```bash
# 检查触摸设备
lsusb                                    # 查看 USB 设备
cat /proc/bus/input/devices | grep -A5 "Touch"
ls /dev/input/event*

# 测试触摸事件
evtest /dev/input/eventX                 # X 为触摸设备编号

# 如果触摸坐标不准，使用 xinput_calibrator 校准（X11 环境）
apt install xinput-calibrator
xinput_calibrator

# Wayland 环境下使用 libinput
libinput list-devices
```

#### 7.3.4 触摸坐标旋转

如果屏幕旋转了但触摸坐标没跟着转：

```bash
# X11 环境
# 查看触摸设备 ID
xinput list
# 设置坐标变换矩阵（以旋转 90° 为例）
xinput set-prop <device_id> "Coordinate Transformation Matrix" 0 1 0 -1 0 1 0 0 1

# 常用旋转矩阵：
# 0°:   1 0 0  0 1 0  0 0 1
# 90°:  0 1 0 -1 0 1  0 0 1
# 180°: -1 0 1  0 -1 1  0 0 1
# 270°: 0 -1 1  1 0 0  0 0 1
```

### 7.4 HDMI 工作原理（供理解）

```
应用程序
    │
    ▼
DRM/KMS 子系统
    │
    ▼
VOP2 (RK3588) / DE2 (H616)     ← 显示引擎，将 framebuffer 转为视频流
    │
    ▼
HDMI 控制器                      ← 将视频流编码为 TMDS 信号
    │
    ▼
HDMI PHY                         ← 物理层，驱动差分信号
    │
    ▼
HDMI 连接器 ──── HDMI 线 ──── 屏幕 HDMI 接收器
                                     │
                                     ▼
                                  屏幕内部 TCON ──── LCD 面板
```

HDMI 信号包含：
- 3 对 TMDS 数据通道（传输 RGB 像素数据）
- 1 对 TMDS 时钟通道
- DDC 通道（I2C，用于读取 EDID）
- CEC 通道（消费电子控制，可选）
- HPD（热插拔检测）

### 7.5 调试方法

```bash
# 查看 HDMI 状态
cat /sys/class/drm/card0-HDMI-A-1/status
cat /sys/class/drm/card0-HDMI-A-1/modes

# 查看显示引擎状态
cat /sys/kernel/debug/dri/0/summary

# 内核日志
dmesg | grep -i "hdmi\|drm\|display"

# modetest 测试
modetest -M rockchip -c    # 列出 connector
modetest -M rockchip -s <connector_id>@<crtc_id>:1024x600

# 查看 EDID 信息
edid-decode /sys/class/drm/card0-HDMI-A-1/edid

# framebuffer 测试
cat /dev/urandom > /dev/fb0
```

---

## 8. 各接口对比总结

### 8.1 选型决策表

| 需求场景 | 推荐接口 | 推荐屏幕 | 适用开发板 |
|---------|---------|---------|-----------|
| IoT 状态显示、极简 UI | I2C (SSD1306) | 0.96" OLED | 两者均可 |
| 小型仪表盘、传感器数据 | SPI (ST7789V) | 1.3" TFT | 两者均可 |
| 工控触摸屏、中等分辨率 | 并行 RGB (AT070TN92) | 7" TFT | 需 RGB 接口的 SoC |
| 手机屏复用、高分辨率 | MIPI DSI (ILI9881C) | 5" IPS | Orange Pi 5 Max |
| 工业屏、车载屏 | LVDS (HJ070NA-13A) | 7" IPS | OPi 5 Max + 桥接 |
| 快速开发、教学演示 | HDMI | 7" HDMI 屏 | 两者均可 |

### 8.2 开发难度与成本对比

```
开发难度（驱动开发工作量）

HDMI     ▓░░░░░░░░░  几乎为零，即插即用
I2C      ▓▓░░░░░░░░  简单，协议简单，数据量小
SPI      ▓▓▓░░░░░░░  中等偏低，需要初始化序列
RGB      ▓▓▓▓░░░░░░  中等，信号线多，时序配置
LVDS     ▓▓▓▓▓▓░░░░  较高，需要桥接芯片
MIPI DSI ▓▓▓▓▓▓▓░░░  高，协议复杂，初始化序列长

硬件成本（屏幕 + 转接）

I2C      ▓░░░░░░░░░  ~5 元（0.96" OLED 模块）
SPI      ▓▓░░░░░░░░  ~10 元（1.3" TFT 模块）
HDMI     ▓▓▓▓░░░░░░  ~80 元（7" HDMI 屏）
RGB      ▓▓▓▓░░░░░░  ~60 元（7" TFT 裸屏 + 转接板）
MIPI DSI ▓▓▓▓▓░░░░░  ~100 元（5" MIPI 屏模组）
LVDS     ▓▓▓▓▓▓░░░░  ~120 元（7" LVDS 屏 + 桥接板）
```

### 8.3 学习路径建议

如果你是嵌入式屏幕驱动开发的初学者，建议按以下顺序学习：

```
第一步：HDMI 屏
  └─ 理解 DRM/KMS 框架、connector/encoder/crtc 概念
      │
第二步：I2C OLED (SSD1306)
  └─ 学习 I2C 协议、用户态设备操作、framebuffer 概念
      │
第三步：SPI TFT (ST7789V)
  └─ 学习 SPI 协议、DC/RS 信号、屏幕初始化序列、fbtft 框架
      │
第四步：并行 RGB TFT (AT070TN92)
  └─ 理解像素时钟、行场同步、DE 信号、时序参数计算
      │
第五步：MIPI DSI (ILI9881C)
  └─ 学习 MIPI DSI 协议（LP/HS 模式、DCS 命令）、DRM panel 驱动框架
      │
第六步：LVDS (HJ070NA-13A + SN65DSI83)
  └─ 学习 LVDS 编码、桥接芯片配置、多级显示通路
```

### 8.4 通用调试工具清单

```bash
# DRM/KMS 调试
modetest -M <driver>            # 测试显示模式
cat /sys/kernel/debug/dri/0/summary  # 显示通路总览

# I2C 调试
i2cdetect -y <bus>              # 扫描 I2C 总线
i2cget / i2cset                 # 读写 I2C 寄存器

# SPI 调试
spi-pipe / spi-config           # SPI 工具（spi-tools 包）

# GPIO 调试
gpiodetect / gpioinfo / gpioset # libgpiod 工具
cat /sys/kernel/debug/gpio      # 内核 GPIO 状态

# 输入设备调试
evtest                          # 触摸/按键事件测试

# 通用
dmesg | grep -i "drm\|panel\|dsi\|spi\|i2c"
fbset -i -fb /dev/fbX           # framebuffer 信息
```

---

## 9. 参考资料

### 开发板资料
- [Orange Pi Zero 2 官方页面](http://www.orangepi.org/html/hardWare/computerAndMicrocontrollers/details/Orange-Pi-Zero-2.html) — 原理图、用户手册、系统镜像
- [Orange Pi 5 Max 官方页面](http://www.orangepi.org/html/hardWare/computerAndMicrocontrollers/details/Orange-Pi-5-Max.html) — 原理图、用户手册、系统镜像
- [Orange Pi Linux 内核源码](https://github.com/orangepi-xunlong/linux-orangepi) — 各分支对应不同 SoC

### 屏幕驱动 IC Datasheet
- ST7789V — Sitronix 官网或 LCD Wiki
- SSD1306 — Solomon Systech 官网
- ILI9881C — Ilitek 官网（需 NDA）或屏厂提供
- SN65DSI83 — [TI 官网](https://www.ti.com/product/SN65DSI83)

### Linux 内核文档
- DRM/KMS 框架：`Documentation/gpu/drm-kms.rst`
- DRM Panel 驱动：`Documentation/gpu/drm-kms-helpers.rst`
- Device Tree Bindings：`Documentation/devicetree/bindings/display/`
- fbtft 框架：`drivers/staging/fbtft/README`

### 协议规范
- MIPI DSI 规范：MIPI Alliance（需会员）
- LVDS 规范：ANSI/TIA/EIA-644
- SPI 规范：各 SoC 厂商 TRM
- I2C 规范：NXP UM10204

### 社区资源
- [Linux Sunxi Wiki](https://linux-sunxi.org/) — Allwinner 平台社区 Wiki
- [Rockchip Wiki](http://opensource.rock-chips.com/) — Rockchip 开源资料
- [LCD Wiki](http://www.lcdwiki.com/) — 各类 LCD 模块资料和示例代码
- [Embedded Linux Wiki - Display](https://elinux.org/Displays) — 嵌入式 Linux 显示相关
