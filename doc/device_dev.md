# 嵌入式Linux开发板外设硬件驱动开发指南

> 适用开发板：Orange Pi Zero 2 (全志H616)、Orange Pi 5 Max (瑞芯微RK3588)

---

## 目录

1. [常见嵌入式外设硬件分类](#1-常见嵌入式外设硬件分类)
2. [GPIO — LED灯 (驱动入门)](#2-gpio--led灯)
3. [I2C — OLED显示屏 SSD1306](#3-i2c--oled显示屏-ssd1306)
4. [SPI — TFT液晶屏 ST7789](#4-spi--tft液晶屏-st7789)
5. [UART — GPS模块 NEO-6M](#5-uart--gps模块-neo-6m)
6. [PWM — 舵机 SG90](#6-pwm--舵机-sg90)
7. [ADC — 温度传感器 NTC热敏电阻](#7-adc--温度传感器-ntc热敏电阻)
8. [1-Wire — 温度传感器 DS18B20](#8-1-wire--温度传感器-ds18b20)
9. [I2C — 气压传感器 BMP280](#9-i2c--气压传感器-bmp280)
10. [SPI — CAN总线控制器 MCP2515](#10-spi--can总线控制器-mcp2515)
11. [USB — USB摄像头 (UVC)](#11-usb--usb摄像头-uvc)
12. [I2C — 实时时钟 DS3231](#12-i2c--实时时钟-ds3231)
13. [GPIO中断 — 红外接收 VS1838B](#13-gpio中断--红外接收-vs1838b)
14. [I2S — 音频DAC模块 PCM5102A](#14-i2s--音频dac模块-pcm5102a)
15. [以太网/SPI — W5500网络模块](#15-以太网spi--w5500网络模块)
16. [SDIO — WiFi模块 (板载)](#16-sdio--wifi模块)
17. [MIPI CSI — 摄像头模块 IMX415](#17-mipi-csi--摄像头模块-imx415)
18. [GPIO/I2C — 继电器模块](#18-gpioi2c--继电器模块)

---

## 1. 常见嵌入式外设硬件分类

| 分类 | 通信接口 | 常见外设举例 |
|------|----------|-------------|
| 显示类 | I2C/SPI/MIPI | OLED(SSD1306)、TFT LCD(ST7789)、墨水屏(SSD1680) |
| 传感器类 | I2C/SPI/1-Wire/ADC | 温湿度(DHT22/BMP280/DS18B20)、加速度(MPU6050)、光照(BH1750) |
| 通信类 | UART/SPI/USB | GPS(NEO-6M)、蓝牙(HC-05)、LoRa(SX1278)、CAN(MCP2515)、WiFi(ESP8266) |
| 执行器类 | GPIO/PWM | LED、继电器、舵机(SG90)、步进电机(28BYJ-48)、蜂鸣器 |
| 存储类 | SPI/SDIO | SD卡、SPI Flash(W25Q128)、EEPROM(AT24C256) |
| 音视频类 | I2S/MIPI CSI/USB | 麦克风(INMP441)、DAC(PCM5102A)、摄像头(IMX415/OV5647) |
| 网络类 | SPI/SDIO/USB | 以太网(W5500)、WiFi模块、4G模块(EC20) |
| 时钟类 | I2C | RTC(DS3231)、GPS授时 |


### 两块开发板的关键接口对比

| 接口 | Orange Pi Zero 2 (H616) | Orange Pi 5 Max (RK3588) |
|------|------------------------|--------------------------|
| GPIO | 26-pin扩展头 | 40-pin扩展头 |
| I2C | I2C-1, I2C-2 | I2C-2, I2C-3, I2C-5 |
| SPI | SPI-1 (CS0) | SPI-0, SPI-4 |
| UART | UART-2, UART-5 | UART-1, UART-3, UART-4 |
| PWM | PWM通道有限 | 多路PWM |
| ADC | 无独立ADC引脚 | 1路SARADC |
| I2S | 支持 | 支持 |
| USB | 1xUSB2.0 + 1xUSB3.0 | 2xUSB3.0 + 1xUSB2.0 + 1xType-C |
| MIPI CSI | 不支持 | 2x 4-lane MIPI CSI |
| PCIe | 不支持 | PCIe 3.0 x4 |

> 注意：Orange Pi Zero 2 的GPIO引脚较少，部分高级接口需要复用；Orange Pi 5 Max 接口丰富，适合更复杂的外设驱动开发。

---

## 2. GPIO — LED灯

LED是最基础的GPIO外设，适合作为驱动开发的入门练习。

### 2.1 硬件

**器件：** 5mm LED + 330Ω电阻

**接线方式：**

```
Orange Pi GPIO Pin (例如 PC9/Pin7)
        │
        ├──── 330Ω电阻 ────┬──── LED(+) ──── LED(-) ──── GND
                            │
                     (限流保护)
```

| 开发板 | 推荐GPIO | 物理引脚 |
|--------|---------|---------|
| OPi Zero 2 | PC9 | Pin 7 (26-pin header) |
| OPi 5 Max | GPIO1_A4 | Pin 7 (40-pin header) |

**注意事项：**
- LED正极(长脚)接电阻，负极(短脚)接GND
- 330Ω电阻限流，防止烧毁LED（3.3V GPIO，LED压降约2V，电流约3mA）
- 不要直接将GPIO接LED，必须串联电阻

### 2.2 软件驱动

#### 方法一：sysfs方式（用户空间，快速验证）

```bash
# 查看GPIO编号映射
cat /sys/kernel/debug/gpio

# Orange Pi Zero 2: PC9 = 32*2+9 = 73
# Orange Pi 5 Max: GPIO1_A4 = 32*1+4 = 36 (具体参考原理图)

GPIO_NUM=73  # 根据实际开发板修改

# 导出GPIO
echo $GPIO_NUM > /sys/class/gpio/export

# 设置为输出
echo out > /sys/class/gpio/gpio${GPIO_NUM}/direction

# 点亮LED
echo 1 > /sys/class/gpio/gpio${GPIO_NUM}/value

# 熄灭LED
echo 0 > /sys/class/gpio/gpio${GPIO_NUM}/value

# 释放GPIO
echo $GPIO_NUM > /sys/class/gpio/unexport
```

#### 方法二：libgpiod方式（推荐的现代方式）

```bash
# 安装libgpiod
sudo apt install gpiod libgpiod-dev
```

```c
/* led_blink.c - 使用libgpiod控制LED闪烁 */
#include <gpiod.h>
#include <stdio.h>
#include <unistd.h>

#define CHIP_NAME "gpiochip0"  /* OPi Zero2用gpiochip0, OPi 5 Max可能是gpiochip1 */
#define LED_LINE  9            /* 根据实际GPIO编号修改 */

int main(void)
{
    struct gpiod_chip *chip;
    struct gpiod_line *led;
    int ret;

    chip = gpiod_chip_open_by_name(CHIP_NAME);
    if (!chip) {
        perror("gpiod_chip_open");
        return 1;
    }

    led = gpiod_chip_get_line(chip, LED_LINE);
    if (!led) {
        perror("gpiod_chip_get_line");
        gpiod_chip_close(chip);
        return 1;
    }

    /* 请求GPIO为输出模式，初始值为0(灭) */
    ret = gpiod_line_request_output(led, "led-blink", 0);
    if (ret < 0) {
        perror("gpiod_line_request_output");
        gpiod_chip_close(chip);
        return 1;
    }

    /* LED闪烁10次 */
    for (int i = 0; i < 10; i++) {
        gpiod_line_set_value(led, 1);
        printf("LED ON\n");
        sleep(1);

        gpiod_line_set_value(led, 0);
        printf("LED OFF\n");
        sleep(1);
    }

    gpiod_line_release(led);
    gpiod_chip_close(chip);
    return 0;
}
```

```bash
# 编译
gcc -o led_blink led_blink.c -lgpiod

# 运行（需要root权限）
sudo ./led_blink
```

#### 方法三：内核驱动模块（学习内核驱动开发）

```c
/* led_driver.c - 最简单的GPIO LED内核驱动 */
#include <linux/module.h>
#include <linux/gpio.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>

#define DEVICE_NAME "myled"
#define GPIO_LED    73  /* 根据实际修改 */

static dev_t dev_num;
static struct cdev led_cdev;
static struct class *led_class;

static int led_open(struct inode *inode, struct file *filp)
{
    return 0;
}

static ssize_t led_write(struct file *filp, const char __user *buf,
                         size_t count, loff_t *ppos)
{
    char val;
    if (copy_from_user(&val, buf, 1))
        return -EFAULT;

    gpio_set_value(GPIO_LED, val == '1' ? 1 : 0);
    return count;
}

static struct file_operations led_fops = {
    .owner = THIS_MODULE,
    .open  = led_open,
    .write = led_write,
};

static int __init led_init(void)
{
    int ret;

    /* 申请GPIO */
    ret = gpio_request(GPIO_LED, "myled");
    if (ret) {
        pr_err("Failed to request GPIO %d\n", GPIO_LED);
        return ret;
    }
    gpio_direction_output(GPIO_LED, 0);

    /* 注册字符设备 */
    alloc_chrdev_region(&dev_num, 0, 1, DEVICE_NAME);
    cdev_init(&led_cdev, &led_fops);
    cdev_add(&led_cdev, dev_num, 1);

    /* 创建设备节点 /dev/myled */
    led_class = class_create(THIS_MODULE, DEVICE_NAME);
    device_create(led_class, NULL, dev_num, NULL, DEVICE_NAME);

    pr_info("LED driver loaded, GPIO=%d\n", GPIO_LED);
    return 0;
}

static void __exit led_exit(void)
{
    gpio_set_value(GPIO_LED, 0);
    gpio_free(GPIO_LED);
    device_destroy(led_class, dev_num);
    class_destroy(led_class);
    cdev_del(&led_cdev);
    unregister_chrdev_region(dev_num, 1);
    pr_info("LED driver unloaded\n");
}

module_init(led_init);
module_exit(led_exit);
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Simple LED GPIO Driver");
```

Makefile:
```makefile
obj-m += led_driver.o

KDIR := /lib/modules/$(shell uname -r)/build

all:
	make -C $(KDIR) M=$(PWD) modules

clean:
	make -C $(KDIR) M=$(PWD) clean
```

```bash
# 编译内核模块
make

# 加载驱动
sudo insmod led_driver.ko

# 使用
echo 1 | sudo tee /dev/myled   # 点亮
echo 0 | sudo tee /dev/myled   # 熄灭

# 卸载驱动
sudo rmmod led_driver
```

---

## 3. I2C — OLED显示屏 SSD1306

SSD1306是最常见的小型OLED显示屏控制器，0.96寸128x64分辨率，I2C接口。

### 3.1 硬件

**器件：** 0.96寸 SSD1306 OLED模块 (I2C接口，4针)

**接线：**

```
OLED模块          开发板
┌──────┐
│ VCC  │────────── 3.3V (Pin 1)
│ GND  │────────── GND  (Pin 6)
│ SCL  │────────── I2C SCL
│ SDA  │────────── I2C SDA
└──────┘
```

| 开发板 | I2C总线 | SCL引脚 | SDA引脚 |
|--------|---------|---------|---------|
| OPi Zero 2 | I2C-1 | Pin 5 (PA12) | Pin 3 (PA11) |
| OPi 5 Max | I2C-5 | Pin 5 (GPIO1_B6) | Pin 3 (GPIO1_B7) |

**注意事项：**
- SSD1306默认I2C地址为 `0x3C`（有些模块是 `0x3D`，取决于背面电阻配置）
- I2C总线已有上拉电阻（模块自带），无需额外添加
- 供电3.3V或5V均可（模块自带稳压）

### 3.2 软件驱动

#### 步骤一：启用I2C

```bash
# Orange Pi Zero 2: 编辑 /boot/orangepiEnv.txt
# 添加 overlays=i2c1

# Orange Pi 5 Max: 使用 orangepi-config 或编辑设备树
sudo orangepi-config
# -> System -> Hardware -> 启用对应I2C

# 重启后验证
sudo apt install i2c-tools
sudo i2cdetect -y 1   # 应该在0x3c位置看到设备
```

#### 步骤二：用户空间驱动（C语言直接操作I2C）

```c
/* ssd1306_i2c.c - SSD1306 OLED I2C驱动 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>

#define SSD1306_ADDR    0x3C
#define SSD1306_WIDTH   128
#define SSD1306_HEIGHT  64

static int i2c_fd;
static uint8_t framebuffer[SSD1306_WIDTH * SSD1306_HEIGHT / 8];

/* 发送命令 */
static int ssd1306_cmd(uint8_t cmd)
{
    uint8_t buf[2] = {0x00, cmd};  /* Co=0, D/C#=0 (command) */
    return write(i2c_fd, buf, 2);
}

/* 初始化SSD1306 */
static int ssd1306_init(void)
{
    /* 标准初始化序列 */
    ssd1306_cmd(0xAE);  /* Display OFF */
    ssd1306_cmd(0xD5);  /* Set display clock */
    ssd1306_cmd(0x80);
    ssd1306_cmd(0xA8);  /* Set multiplex ratio */
    ssd1306_cmd(0x3F);  /* 1/64 duty */
    ssd1306_cmd(0xD3);  /* Set display offset */
    ssd1306_cmd(0x00);
    ssd1306_cmd(0x40);  /* Set start line = 0 */
    ssd1306_cmd(0x8D);  /* Charge pump */
    ssd1306_cmd(0x14);  /* Enable charge pump */
    ssd1306_cmd(0x20);  /* Memory addressing mode */
    ssd1306_cmd(0x00);  /* Horizontal addressing */
    ssd1306_cmd(0xA1);  /* Segment remap */
    ssd1306_cmd(0xC8);  /* COM output scan direction */
    ssd1306_cmd(0xDA);  /* COM pins configuration */
    ssd1306_cmd(0x12);
    ssd1306_cmd(0x81);  /* Set contrast */
    ssd1306_cmd(0xCF);
    ssd1306_cmd(0xD9);  /* Set pre-charge period */
    ssd1306_cmd(0xF1);
    ssd1306_cmd(0xDB);  /* Set VCOMH deselect level */
    ssd1306_cmd(0x40);
    ssd1306_cmd(0xA4);  /* Display from RAM */
    ssd1306_cmd(0xA6);  /* Normal display (not inverted) */
    ssd1306_cmd(0xAF);  /* Display ON */
    return 0;
}

/* 刷新显示 */
static void ssd1306_display(void)
{
    ssd1306_cmd(0x21);  /* Set column address */
    ssd1306_cmd(0);     /* Start = 0 */
    ssd1306_cmd(127);   /* End = 127 */
    ssd1306_cmd(0x22);  /* Set page address */
    ssd1306_cmd(0);     /* Start = 0 */
    ssd1306_cmd(7);     /* End = 7 */

    /* 发送framebuffer数据 */
    uint8_t buf[SSD1306_WIDTH * SSD1306_HEIGHT / 8 + 1];
    buf[0] = 0x40;  /* Co=0, D/C#=1 (data) */
    memcpy(buf + 1, framebuffer, sizeof(framebuffer));
    write(i2c_fd, buf, sizeof(buf));
}

/* 画点 */
static void ssd1306_pixel(int x, int y, int color)
{
    if (x < 0 || x >= SSD1306_WIDTH || y < 0 || y >= SSD1306_HEIGHT)
        return;
    if (color)
        framebuffer[x + (y / 8) * SSD1306_WIDTH] |= (1 << (y & 7));
    else
        framebuffer[x + (y / 8) * SSD1306_WIDTH] &= ~(1 << (y & 7));
}

int main(void)
{
    /* 打开I2C设备 */
    i2c_fd = open("/dev/i2c-1", O_RDWR);
    if (i2c_fd < 0) {
        perror("open i2c");
        return 1;
    }

    if (ioctl(i2c_fd, I2C_SLAVE, SSD1306_ADDR) < 0) {
        perror("ioctl I2C_SLAVE");
        close(i2c_fd);
        return 1;
    }

    ssd1306_init();

    /* 清屏 */
    memset(framebuffer, 0, sizeof(framebuffer));

    /* 画一个矩形框 */
    for (int x = 0; x < 128; x++) {
        ssd1306_pixel(x, 0, 1);
        ssd1306_pixel(x, 63, 1);
    }
    for (int y = 0; y < 64; y++) {
        ssd1306_pixel(0, y, 1);
        ssd1306_pixel(127, y, 1);
    }

    /* 画对角线 */
    for (int i = 0; i < 64; i++) {
        ssd1306_pixel(i * 2, i, 1);
        ssd1306_pixel(127 - i * 2, i, 1);
    }

    ssd1306_display();
    printf("OLED display updated!\n");

    close(i2c_fd);
    return 0;
}
```

```bash
gcc -o ssd1306_test ssd1306_i2c.c
sudo ./ssd1306_test
```

---

## 4. SPI — TFT液晶屏 ST7789

ST7789是常见的TFT LCD控制器，支持240x240或240x320分辨率，SPI接口，刷新速度快。

### 4.1 硬件

**器件：** 1.3寸 ST7789 TFT LCD模块 (240x240, SPI接口)

**接线：**

```
TFT模块           开发板
┌──────┐
│ VCC  │────────── 3.3V
│ GND  │────────── GND
│ SCL  │────────── SPI CLK
│ SDA  │────────── SPI MOSI
│ RES  │────────── GPIO (复位引脚)
│ DC   │────────── GPIO (数据/命令选择)
│ CS   │────────── SPI CS0
│ BLK  │────────── 3.3V 或 GPIO (背光控制)
└──────┘
```

| 开发板 | SPI总线 | CLK | MOSI | CS0 | DC(推荐) | RES(推荐) |
|--------|---------|-----|------|-----|---------|----------|
| OPi Zero 2 | SPI-1 | Pin 23 (PC0) | Pin 19 (PC2) | Pin 24 (PC3) | Pin 7 (PC9) | Pin 11 (PC6) |
| OPi 5 Max | SPI-4 | Pin 23 | Pin 19 | Pin 24 | Pin 7 | Pin 11 |

**注意事项：**
- DC引脚区分命令和数据，低电平=命令，高电平=数据
- RES引脚低电平复位，正常工作时保持高电平
- SPI时钟频率可设置到40-60MHz以获得较好的刷新率
- 背光BLK接3.3V常亮，或接PWM引脚实现亮度调节

### 4.2 软件驱动

#### 步骤一：启用SPI

```bash
# 编辑设备树overlay启用SPI
# Orange Pi Zero 2: /boot/orangepiEnv.txt 添加 overlays=spi-spidev1
# Orange Pi 5 Max: 使用 orangepi-config 启用SPI

# 重启后验证
ls /dev/spidev*
# 应看到 /dev/spidev1.0 或 /dev/spidev4.0
```

#### 步骤二：用户空间SPI驱动

```c
/* st7789_spi.c - ST7789 TFT LCD SPI驱动 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>
#include <linux/gpio.h>
#include <gpiod.h>

#define SPI_DEVICE  "/dev/spidev1.0"
#define SPI_SPEED   40000000  /* 40MHz */
#define WIDTH       240
#define HEIGHT      240

static int spi_fd;
static struct gpiod_line *dc_line;
static struct gpiod_line *rst_line;

/* SPI传输 */
static void spi_transfer(const uint8_t *data, size_t len)
{
    struct spi_ioc_transfer tr = {
        .tx_buf = (unsigned long)data,
        .len = len,
        .speed_hz = SPI_SPEED,
        .bits_per_word = 8,
    };
    ioctl(spi_fd, SPI_IOC_MESSAGE(1), &tr);
}

/* 发送命令 */
static void st7789_cmd(uint8_t cmd)
{
    gpiod_line_set_value(dc_line, 0);  /* DC=0: command */
    spi_transfer(&cmd, 1);
}

/* 发送数据 */
static void st7789_data(const uint8_t *data, size_t len)
{
    gpiod_line_set_value(dc_line, 1);  /* DC=1: data */
    spi_transfer(data, len);
}

static void st7789_data_byte(uint8_t val)
{
    st7789_data(&val, 1);
}

/* 硬件复位 */
static void st7789_reset(void)
{
    gpiod_line_set_value(rst_line, 1);
    usleep(50000);
    gpiod_line_set_value(rst_line, 0);
    usleep(50000);
    gpiod_line_set_value(rst_line, 1);
    usleep(150000);
}

/* 初始化ST7789 */
static void st7789_init(void)
{
    st7789_reset();

    st7789_cmd(0x01);  /* Software reset */
    usleep(150000);

    st7789_cmd(0x11);  /* Sleep out */
    usleep(120000);

    st7789_cmd(0x3A);  /* Pixel format: 16bit/pixel (RGB565) */
    st7789_data_byte(0x55);

    st7789_cmd(0x36);  /* Memory access control */
    st7789_data_byte(0x00);

    st7789_cmd(0x21);  /* Display inversion on (ST7789特性) */

    st7789_cmd(0x13);  /* Normal display mode */
    st7789_cmd(0x29);  /* Display ON */
}

/* 设置绘制窗口 */
static void st7789_set_window(uint16_t x0, uint16_t y0,
                               uint16_t x1, uint16_t y1)
{
    uint8_t data[4];

    st7789_cmd(0x2A);  /* Column address set */
    data[0] = x0 >> 8; data[1] = x0 & 0xFF;
    data[2] = x1 >> 8; data[3] = x1 & 0xFF;
    st7789_data(data, 4);

    st7789_cmd(0x2B);  /* Row address set */
    data[0] = y0 >> 8; data[1] = y0 & 0xFF;
    data[2] = y1 >> 8; data[3] = y1 & 0xFF;
    st7789_data(data, 4);

    st7789_cmd(0x2C);  /* Memory write */
}

/* 填充颜色 (RGB565) */
static void st7789_fill(uint16_t color)
{
    st7789_set_window(0, 0, WIDTH - 1, HEIGHT - 1);

    /* 批量发送像素数据 */
    uint8_t buf[WIDTH * 2];
    for (int x = 0; x < WIDTH; x++) {
        buf[x * 2]     = color >> 8;
        buf[x * 2 + 1] = color & 0xFF;
    }

    gpiod_line_set_value(dc_line, 1);
    for (int y = 0; y < HEIGHT; y++) {
        spi_transfer(buf, sizeof(buf));
    }
}

int main(void)
{
    /* 打开SPI设备 */
    spi_fd = open(SPI_DEVICE, O_RDWR);
    if (spi_fd < 0) {
        perror("open spi");
        return 1;
    }

    uint8_t mode = SPI_MODE_0;
    uint8_t bits = 8;
    uint32_t speed = SPI_SPEED;
    ioctl(spi_fd, SPI_IOC_WR_MODE, &mode);
    ioctl(spi_fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
    ioctl(spi_fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);

    /* 初始化GPIO (DC和RST引脚) */
    struct gpiod_chip *chip = gpiod_chip_open_by_name("gpiochip0");
    dc_line  = gpiod_chip_get_line(chip, 9);   /* PC9 */
    rst_line = gpiod_chip_get_line(chip, 6);    /* PC6 */
    gpiod_line_request_output(dc_line, "st7789-dc", 1);
    gpiod_line_request_output(rst_line, "st7789-rst", 1);

    st7789_init();

    /* 红色填充 (RGB565: 0xF800) */
    st7789_fill(0xF800);
    printf("Screen filled RED\n");
    sleep(2);

    /* 绿色填充 (RGB565: 0x07E0) */
    st7789_fill(0x07E0);
    printf("Screen filled GREEN\n");
    sleep(2);

    /* 蓝色填充 (RGB565: 0x001F) */
    st7789_fill(0x001F);
    printf("Screen filled BLUE\n");

    gpiod_line_release(dc_line);
    gpiod_line_release(rst_line);
    gpiod_chip_close(chip);
    close(spi_fd);
    return 0;
}
```

```bash
gcc -o st7789_test st7789_spi.c -lgpiod
sudo ./st7789_test
```

#### 方法二：使用内核FBTFT框架（设备树方式）

对于ST7789，Linux内核自带`fbtft`驱动，可以通过设备树overlay直接使用：

```dts
/* sun50i-h616-spi1-st7789v.dts - Orange Pi Zero 2设备树overlay示例 */
/dts-v1/;
/plugin/;

/ {
    compatible = "allwinner,sun50i-h616";

    fragment@0 {
        target = <&spi1>;
        __overlay__ {
            status = "okay";
            #address-cells = <1>;
            #size-cells = <0>;

            st7789v@0 {
                compatible = "sitronix,st7789v";
                reg = <0>;                      /* CS0 */
                spi-max-frequency = <40000000>;
                rotate = <0>;
                width = <240>;
                height = <240>;
                dc-gpios = <&pio 2 9 0>;        /* PC9 */
                reset-gpios = <&pio 2 6 0>;     /* PC6 */
                buswidth = <8>;
            };
        };
    };
};
```

编译并加载后，会自动创建 `/dev/fb1` 帧缓冲设备，可以直接用 `fbset`、`fbi` 等工具操作。

---

## 5. UART — GPS模块 NEO-6M

NEO-6M是u-blox出品的低成本GPS接收模块，UART接口，输出NMEA标准语句。

### 5.1 硬件

**器件：** NEO-6M GPS模块（带陶瓷天线）

**接线：**

```
GPS模块            开发板
┌──────┐
│ VCC  │────────── 5V (或3.3V，模块自带稳压)
│ GND  │────────── GND
│ TXD  │────────── UART RXD
│ RXD  │────────── UART TXD
│ PPS  │────────── GPIO (可选，秒脉冲输出)
└──────┘
```

| 开发板 | UART | TXD引脚 | RXD引脚 |
|--------|------|---------|---------|
| OPi Zero 2 | UART-5 | Pin 8 (PH2) | Pin 10 (PH3) |
| OPi 5 Max | UART-4 | Pin 8 | Pin 10 |

**注意事项：**
- GPS模块TXD接开发板RXD，RXD接开发板TXD（交叉连接）
- NEO-6M默认波特率9600bps
- 首次定位需要在室外空旷处，冷启动约30秒-几分钟
- 模块自带EEPROM保存配置，热启动更快

### 5.2 软件驱动

#### 步骤一：启用UART

```bash
# 启用对应UART overlay
# Orange Pi Zero 2: /boot/orangepiEnv.txt 添加 overlays=uart5
# Orange Pi 5 Max: orangepi-config 启用 uart4

# 重启后验证
ls /dev/ttyS*
# 应看到 /dev/ttyS5 (Zero2) 或 /dev/ttyS4 (5 Max)
```

#### 步骤二：快速验证

```bash
# 安装minicom
sudo apt install minicom

# 查看GPS原始数据
sudo minicom -D /dev/ttyS5 -b 9600
# 应看到类似：
# $GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,...
# $GPRMC,123519,A,4807.038,N,01131.000,E,022.4,084.4,...
```

#### 步骤三：C语言NMEA解析程序

```c
/* gps_reader.c - 读取NEO-6M GPS数据并解析GPRMC语句 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>

#define GPS_DEVICE "/dev/ttyS5"  /* 根据实际修改 */
#define BAUD_RATE  B9600

/* 配置串口 */
static int uart_open(const char *dev)
{
    int fd = open(dev, O_RDONLY | O_NOCTTY);
    if (fd < 0) {
        perror("open uart");
        return -1;
    }

    struct termios tty;
    tcgetattr(fd, &tty);

    cfsetispeed(&tty, BAUD_RATE);
    cfsetospeed(&tty, BAUD_RATE);

    tty.c_cflag &= ~PARENB;        /* 无校验 */
    tty.c_cflag &= ~CSTOPB;        /* 1位停止位 */
    tty.c_cflag &= ~CSIZE;
    tty.c_cflag |= CS8;            /* 8位数据位 */
    tty.c_cflag |= CREAD | CLOCAL; /* 使能接收 */

    tty.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG); /* 原始模式 */
    tty.c_iflag &= ~(IXON | IXOFF | IXANY);
    tty.c_oflag &= ~OPOST;

    tty.c_cc[VMIN]  = 1;
    tty.c_cc[VTIME] = 10;  /* 1秒超时 */

    tcsetattr(fd, TCSANOW, &tty);
    return fd;
}

/* 解析GPRMC语句 */
/* 格式: $GPRMC,hhmmss.ss,A,llll.ll,N,yyyyy.yy,E,speed,course,ddmmyy,... */
static void parse_gprmc(const char *line)
{
    char status;
    double lat, lon, speed;
    int time_h, time_m, time_s;
    char ns, ew;

    if (sscanf(line, "$GPRMC,%2d%2d%2d.%*d,%c,%lf,%c,%lf,%c,%lf",
               &time_h, &time_m, &time_s, &status,
               &lat, &ns, &lon, &ew, &speed) >= 8) {

        if (status == 'A') {  /* A=有效定位 */
            /* NMEA格式: ddmm.mmmm -> 十进制度 */
            int lat_deg = (int)(lat / 100);
            double lat_min = lat - lat_deg * 100;
            double lat_dec = lat_deg + lat_min / 60.0;
            if (ns == 'S') lat_dec = -lat_dec;

            int lon_deg = (int)(lon / 100);
            double lon_min = lon - lon_deg * 100;
            double lon_dec = lon_deg + lon_min / 60.0;
            if (ew == 'W') lon_dec = -lon_dec;

            printf("UTC时间: %02d:%02d:%02d\n", time_h, time_m, time_s);
            printf("纬度: %.6f°%c (%.6f)\n", lat, ns, lat_dec);
            printf("经度: %.6f°%c (%.6f)\n", lon, ew, lon_dec);
            printf("速度: %.1f节\n\n", speed);
        } else {
            printf("GPS未定位 (status=%c)\n", status);
        }
    }
}

int main(void)
{
    int fd = uart_open(GPS_DEVICE);
    if (fd < 0) return 1;

    char buf[512];
    char line[256];
    int line_pos = 0;

    printf("等待GPS数据...\n");

    while (1) {
        int n = read(fd, buf, sizeof(buf) - 1);
        if (n <= 0) continue;

        for (int i = 0; i < n; i++) {
            if (buf[i] == '\n') {
                line[line_pos] = '\0';
                if (strncmp(line, "$GPRMC", 6) == 0) {
                    parse_gprmc(line);
                }
                line_pos = 0;
            } else if (buf[i] != '\r' && line_pos < (int)sizeof(line) - 1) {
                line[line_pos++] = buf[i];
            }
        }
    }

    close(fd);
    return 0;
}
```

```bash
gcc -o gps_reader gps_reader.c
sudo ./gps_reader
```

---

## 6. PWM — 舵机 SG90

SG90是最常见的微型舵机，通过PWM信号控制旋转角度（0°-180°）。

### 6.1 硬件

**器件：** SG90微型舵机（9g）

**接线：**

```
SG90舵机           开发板
┌──────┐
│ 红线  │────────── 5V (外部供电更佳)
│ 棕线  │────────── GND
│ 橙线  │────────── PWM引脚
└──────┘
```

| 开发板 | PWM通道 | 引脚 |
|--------|---------|------|
| OPi Zero 2 | PWM1 | Pin 7 (PC9，需确认PWM复用) |
| OPi 5 Max | PWM14 | Pin 32 (GPIO3_B2) |

**PWM参数：**
- 频率：50Hz（周期20ms）
- 0°位置：脉宽0.5ms（占空比2.5%）
- 90°位置：脉宽1.5ms（占空比7.5%）
- 180°位置：脉宽2.5ms（占空比12.5%）

**注意事项：**
- 舵机电流较大（堵转可达700mA），建议使用外部5V电源供电，不要直接从开发板取电
- 外部供电时GND必须与开发板共地
- PWM信号线可以直接接3.3V GPIO，SG90兼容3.3V逻辑电平

### 6.2 软件驱动

#### 方法一：sysfs PWM控制

```bash
# 启用PWM overlay (以OPi 5 Max为例)
# orangepi-config -> Hardware -> 启用pwm14

# 导出PWM通道
echo 0 > /sys/class/pwm/pwmchip0/export

# 设置周期20ms = 20000000ns
echo 20000000 > /sys/class/pwm/pwmchip0/pwm0/period

# 设置脉宽1.5ms = 1500000ns (90°位置)
echo 1500000 > /sys/class/pwm/pwmchip0/pwm0/duty_cycle

# 使能PWM
echo 1 > /sys/class/pwm/pwmchip0/pwm0/enable

# 转到0° (0.5ms)
echo 500000 > /sys/class/pwm/pwmchip0/pwm0/duty_cycle

# 转到180° (2.5ms)
echo 2500000 > /sys/class/pwm/pwmchip0/pwm0/duty_cycle
```

#### 方法二：C语言PWM控制程序

```c
/* servo_pwm.c - SG90舵机PWM控制 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#define PWM_CHIP    "/sys/class/pwm/pwmchip0"
#define PWM_CHANNEL 0
#define PERIOD_NS   20000000  /* 20ms = 50Hz */

/* 角度转脉宽(ns): 0°=500000, 180°=2500000 */
#define ANGLE_TO_DUTY(angle) (500000 + (angle) * 2000000 / 180)

static void pwm_write(const char *attr, const char *value)
{
    char path[256];
    snprintf(path, sizeof(path), "%s/pwm%d/%s", PWM_CHIP, PWM_CHANNEL, attr);
    int fd = open(path, O_WRONLY);
    if (fd >= 0) {
        write(fd, value, strlen(value));
        close(fd);
    }
}

static void pwm_export(void)
{
    char path[256];
    snprintf(path, sizeof(path), "%s/export", PWM_CHIP);
    int fd = open(path, O_WRONLY);
    if (fd >= 0) {
        char ch[4];
        snprintf(ch, sizeof(ch), "%d", PWM_CHANNEL);
        write(fd, ch, strlen(ch));
        close(fd);
    }
    usleep(100000);  /* 等待sysfs节点创建 */
}

static void servo_set_angle(int angle)
{
    if (angle < 0) angle = 0;
    if (angle > 180) angle = 180;

    char duty[32];
    snprintf(duty, sizeof(duty), "%d", ANGLE_TO_DUTY(angle));
    pwm_write("duty_cycle", duty);
    printf("舵机角度: %d° (脉宽: %dns)\n", angle, ANGLE_TO_DUTY(angle));
}

int main(void)
{
    pwm_export();

    char period[32];
    snprintf(period, sizeof(period), "%d", PERIOD_NS);
    pwm_write("period", period);
    pwm_write("duty_cycle", "1500000");  /* 初始90° */
    pwm_write("enable", "1");

    /* 舵机扫描演示: 0° -> 180° -> 0° */
    for (int angle = 0; angle <= 180; angle += 10) {
        servo_set_angle(angle);
        usleep(300000);  /* 300ms等待舵机到位 */
    }
    for (int angle = 180; angle >= 0; angle -= 10) {
        servo_set_angle(angle);
        usleep(300000);
    }

    pwm_write("enable", "0");
    return 0;
}
```

```bash
gcc -o servo_test servo_pwm.c
sudo ./servo_test
```

---

## 7. ADC — 温度传感器 NTC热敏电阻

NTC热敏电阻是最简单的模拟温度传感器，需要ADC采集。

### 7.1 硬件

**器件：** 10KΩ NTC热敏电阻 (B值3950) + 10KΩ固定电阻

> 注意：Orange Pi Zero 2 没有独立ADC引脚，此示例主要针对 Orange Pi 5 Max (RK3588有SARADC)。
> 对于Zero 2，可以使用外部ADC芯片如ADS1115（I2C接口），见后文补充。

**分压电路：**

```
3.3V ──── 10KΩ固定电阻 ──┬── NTC热敏电阻 ──── GND
                          │
                          └── ADC输入引脚
                         (采集此处电压)
```

| 开发板 | ADC通道 | 引脚 |
|--------|---------|------|
| OPi 5 Max | SARADC_IN0 | Pin 26 |

**原理：**
- NTC电阻随温度升高而减小
- 25°C时NTC=10KΩ，分压点电压=3.3V/2=1.65V
- 温度升高→NTC减小→分压点电压升高
- 温度降低→NTC增大→分压点电压降低

### 7.2 软件驱动

#### RK3588 SARADC读取

```c
/* ntc_adc.c - 通过SARADC读取NTC热敏电阻温度 */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/* NTC参数 (10K B3950) */
#define NTC_R25     10000.0   /* 25°C时的电阻值 */
#define NTC_B       3950.0    /* B值 */
#define T25_KELVIN  298.15    /* 25°C = 298.15K */
#define R_FIXED     10000.0   /* 分压电阻 */
#define V_REF       1.8       /* RK3588 SARADC参考电压 */
#define ADC_MAX     1023      /* 10位ADC */

/* 读取ADC原始值 */
static int read_adc(int channel)
{
    char path[128];
    snprintf(path, sizeof(path),
             "/sys/bus/iio/devices/iio:device0/in_voltage%d_raw", channel);

    FILE *fp = fopen(path, "r");
    if (!fp) {
        perror("open adc");
        return -1;
    }

    int value;
    fscanf(fp, "%d", &value);
    fclose(fp);
    return value;
}

/* ADC值转温度 */
static double adc_to_temperature(int adc_raw)
{
    /* ADC值转电压 */
    double voltage = (double)adc_raw / ADC_MAX * V_REF;

    /* 电压转NTC电阻值: V = Vcc * Rntc / (Rfixed + Rntc) */
    /* 注意：实际电路中Vcc=3.3V但ADC参考电压可能不同，需要校准 */
    double r_ntc = R_FIXED * voltage / (3.3 - voltage);

    /* Steinhart-Hart简化公式 (B参数方程) */
    /* 1/T = 1/T25 + (1/B) * ln(R/R25) */
    double temp_k = 1.0 / (1.0 / T25_KELVIN + log(r_ntc / NTC_R25) / NTC_B);
    double temp_c = temp_k - 273.15;

    return temp_c;
}

int main(void)
{
    printf("NTC温度传感器读取 (ADC通道0)\n");
    printf("----------------------------\n");

    for (int i = 0; i < 10; i++) {
        int raw = read_adc(0);
        if (raw >= 0) {
            double temp = adc_to_temperature(raw);
            printf("ADC原始值: %4d | 温度: %.1f°C\n", raw, temp);
        }
        usleep(1000000);  /* 1秒间隔 */
    }

    return 0;
}
```

#### 补充：Orange Pi Zero 2 使用外部ADC ADS1115 (I2C)

如果开发板没有ADC，可以使用ADS1115模块（16位精度，I2C接口）：

```
ADS1115模块        开发板
┌──────┐
│ VDD  │────────── 3.3V
│ GND  │────────── GND
│ SCL  │────────── I2C SCL
│ SDA  │────────── I2C SDA
│ A0   │────────── NTC分压点
│ ADDR │────────── GND (地址0x48)
└──────┘
```

```c
/* ads1115_read.c - 通过I2C读取ADS1115 ADC */
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <stdint.h>

#define ADS1115_ADDR  0x48

static int i2c_fd;

static int16_t ads1115_read_channel(int channel)
{
    /* 配置寄存器: 单次转换, ±4.096V, 128SPS */
    uint16_t config = 0x8000  /* 开始转换 */
        | ((0x04 + channel) << 12)  /* MUX: AINx vs GND */
        | 0x0200  /* ±4.096V */
        | 0x0100  /* 单次模式 */
        | 0x0080; /* 128 SPS */

    uint8_t buf[3];
    buf[0] = 0x01;  /* 配置寄存器地址 */
    buf[1] = config >> 8;
    buf[2] = config & 0xFF;
    write(i2c_fd, buf, 3);

    usleep(10000);  /* 等待转换完成 */

    /* 读取转换结果 */
    buf[0] = 0x00;  /* 转换寄存器地址 */
    write(i2c_fd, buf, 1);
    read(i2c_fd, buf, 2);

    return (int16_t)((buf[0] << 8) | buf[1]);
}

int main(void)
{
    i2c_fd = open("/dev/i2c-1", O_RDWR);
    ioctl(i2c_fd, I2C_SLAVE, ADS1115_ADDR);

    for (int i = 0; i < 10; i++) {
        int16_t raw = ads1115_read_channel(0);
        /* ±4.096V量程, 16位有符号: 分辨率 = 4.096/32768 = 0.125mV */
        double voltage = raw * 0.000125;
        printf("ADS1115 CH0: raw=%d, voltage=%.4fV\n", raw, voltage);
        sleep(1);
    }

    close(i2c_fd);
    return 0;
}
```

---

## 8. 1-Wire — 温度传感器 DS18B20

DS18B20是Dallas/Maxim出品的数字温度传感器，使用1-Wire单总线协议，精度±0.5°C。

### 8.1 硬件

**器件：** DS18B20温度传感器 + 4.7KΩ上拉电阻

**接线：**

```
DS18B20 (TO-92封装，平面朝向自己，从左到右)
┌─────────┐
│ 1  2  3 │
│GND DQ VDD│
└─────────┘

DS18B20            开发板
┌──────┐
│ VDD  │────────── 3.3V
│ GND  │────────── GND
│ DQ   │──┬─────── GPIO引脚
│      │  │
│      │  └── 4.7KΩ ── 3.3V  (上拉电阻，必须！)
└──────┘
```

| 开发板 | 推荐GPIO | 引脚 |
|--------|---------|------|
| OPi Zero 2 | PA10 | Pin 26 |
| OPi 5 Max | GPIO4_C6 | Pin 7 |

**注意事项：**
- 4.7KΩ上拉电阻是必须的，1-Wire协议依赖上拉电阻
- 一条总线可以挂多个DS18B20（每个有唯一64位ROM地址）
- 测量范围 -55°C ~ +125°C，常用范围精度±0.5°C
- 转换时间：12位精度约750ms

### 8.2 软件驱动

#### 步骤一：启用1-Wire设备树overlay

```bash
# Orange Pi Zero 2: 编辑 /boot/orangepiEnv.txt
# 添加: overlays=w1-gpio

# 或手动编写设备树overlay:
```

```dts
/* w1-gpio-overlay.dts */
/dts-v1/;
/plugin/;

/ {
    fragment@0 {
        target-path = "/";
        __overlay__ {
            onewire {
                compatible = "w1-gpio";
                gpios = <&pio 0 10 0>;  /* PA10 for Zero2 */
                status = "okay";
            };
        };
    };
};
```

```bash
# 加载内核模块
sudo modprobe w1-gpio
sudo modprobe w1-therm

# 验证：查看1-Wire设备
ls /sys/bus/w1/devices/
# 应看到类似 28-xxxxxxxxxxxx 的目录（28开头是DS18B20的家族码）
```

#### 步骤二：读取温度（Shell方式）

```bash
# 读取温度（内核驱动自动处理1-Wire协议）
cat /sys/bus/w1/devices/28-*/w1_slave

# 输出示例：
# 73 01 4b 46 7f ff 0d 10 41 : crc=41 YES
# 73 01 4b 46 7f ff 0d 10 41 t=23187
# t=23187 表示 23.187°C
```

#### 步骤三：C语言读取程序

```c
/* ds18b20.c - 读取DS18B20温度传感器 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>

#define W1_DEVICES_PATH "/sys/bus/w1/devices"

/* 查找DS18B20设备 (家族码28) */
static int find_ds18b20(char *device_path, size_t len)
{
    DIR *dir = opendir(W1_DEVICES_PATH);
    if (!dir) {
        perror("opendir w1");
        return -1;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strncmp(entry->d_name, "28-", 3) == 0) {
            snprintf(device_path, len, "%s/%s/w1_slave",
                     W1_DEVICES_PATH, entry->d_name);
            closedir(dir);
            return 0;
        }
    }

    closedir(dir);
    return -1;
}

/* 读取温度 */
static double read_temperature(const char *device_path)
{
    FILE *fp = fopen(device_path, "r");
    if (!fp) {
        perror("open w1_slave");
        return -999.0;
    }

    char line[128];
    double temp = -999.0;

    /* 第一行包含CRC校验结果 */
    if (fgets(line, sizeof(line), fp)) {
        if (strstr(line, "YES") == NULL) {
            fprintf(stderr, "CRC校验失败\n");
            fclose(fp);
            return -999.0;
        }
    }

    /* 第二行包含温度值 */
    if (fgets(line, sizeof(line), fp)) {
        char *t_pos = strstr(line, "t=");
        if (t_pos) {
            temp = atof(t_pos + 2) / 1000.0;
        }
    }

    fclose(fp);
    return temp;
}

int main(void)
{
    char device_path[256];

    if (find_ds18b20(device_path, sizeof(device_path)) < 0) {
        fprintf(stderr, "未找到DS18B20设备，请检查接线和驱动\n");
        return 1;
    }

    printf("DS18B20设备: %s\n", device_path);
    printf("开始读取温度...\n\n");

    for (int i = 0; i < 20; i++) {
        double temp = read_temperature(device_path);
        if (temp > -999.0) {
            printf("[%2d] 温度: %.3f°C\n", i + 1, temp);
        }
        sleep(1);
    }

    return 0;
}
```

```bash
gcc -o ds18b20_test ds18b20.c
sudo ./ds18b20_test
```

---

## 9. I2C — 气压传感器 BMP280

BMP280是Bosch出品的气压/温度传感器，I2C接口，常用于气象站和无人机。

### 9.1 硬件

**器件：** BMP280模块（GY-BMP280）

**接线：**

```
BMP280模块         开发板
┌──────┐
│ VCC  │────────── 3.3V
│ GND  │────────── GND
│ SCL  │────────── I2C SCL
│ SDA  │────────── I2C SDA
│ CSB  │────────── 3.3V (I2C模式，拉高)
│ SDO  │────────── GND (地址0x76) 或 3.3V (地址0x77)
└──────┘
```

| 开发板 | I2C总线 | SCL | SDA |
|--------|---------|-----|-----|
| OPi Zero 2 | I2C-1 | Pin 5 | Pin 3 |
| OPi 5 Max | I2C-5 | Pin 5 | Pin 3 |

**注意事项：**
- CSB引脚拉高选择I2C模式，拉低选择SPI模式
- SDO引脚决定I2C地址：接GND=0x76，接VCC=0x77
- 气压测量范围：300-1100 hPa，精度±1 hPa
- 温度测量范围：-40°C ~ +85°C，精度±1°C

### 9.2 软件驱动

```bash
# 验证I2C设备
sudo i2cdetect -y 1
# 应在0x76或0x77位置看到设备
```

```c
/* bmp280_i2c.c - BMP280气压温度传感器驱动 */
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <stdint.h>

#define BMP280_ADDR  0x76

static int i2c_fd;

/* 校准参数 (从芯片读取) */
static uint16_t dig_T1;
static int16_t  dig_T2, dig_T3;
static uint16_t dig_P1;
static int16_t  dig_P2, dig_P3, dig_P4, dig_P5;
static int16_t  dig_P6, dig_P7, dig_P8, dig_P9;
static int32_t  t_fine;  /* 温度补偿中间值 */

static uint8_t bmp280_read_reg(uint8_t reg)
{
    write(i2c_fd, &reg, 1);
    uint8_t val;
    read(i2c_fd, &val, 1);
    return val;
}

static void bmp280_read_regs(uint8_t reg, uint8_t *buf, int len)
{
    write(i2c_fd, &reg, 1);
    read(i2c_fd, buf, len);
}

static void bmp280_write_reg(uint8_t reg, uint8_t val)
{
    uint8_t buf[2] = {reg, val};
    write(i2c_fd, buf, 2);
}

/* 读取校准参数 */
static void bmp280_read_calibration(void)
{
    uint8_t cal[26];
    bmp280_read_regs(0x88, cal, 26);

    dig_T1 = cal[0]  | (cal[1] << 8);
    dig_T2 = cal[2]  | (cal[3] << 8);
    dig_T3 = cal[4]  | (cal[5] << 8);
    dig_P1 = cal[6]  | (cal[7] << 8);
    dig_P2 = cal[8]  | (cal[9] << 8);
    dig_P3 = cal[10] | (cal[11] << 8);
    dig_P4 = cal[12] | (cal[13] << 8);
    dig_P5 = cal[14] | (cal[15] << 8);
    dig_P6 = cal[16] | (cal[17] << 8);
    dig_P7 = cal[18] | (cal[19] << 8);
    dig_P8 = cal[20] | (cal[21] << 8);
    dig_P9 = cal[22] | (cal[23] << 8);
}

/* 温度补偿计算 (来自BMP280 datasheet) */
static double bmp280_compensate_temperature(int32_t adc_T)
{
    double var1 = ((double)adc_T / 16384.0 - (double)dig_T1 / 1024.0) * (double)dig_T2;
    double var2 = (((double)adc_T / 131072.0 - (double)dig_T1 / 8192.0) *
                   ((double)adc_T / 131072.0 - (double)dig_T1 / 8192.0)) * (double)dig_T3;
    t_fine = (int32_t)(var1 + var2);
    return (var1 + var2) / 5120.0;
}

/* 气压补偿计算 */
static double bmp280_compensate_pressure(int32_t adc_P)
{
    double var1 = ((double)t_fine / 2.0) - 64000.0;
    double var2 = var1 * var1 * (double)dig_P6 / 32768.0;
    var2 = var2 + var1 * (double)dig_P5 * 2.0;
    var2 = (var2 / 4.0) + ((double)dig_P4 * 65536.0);
    var1 = ((double)dig_P3 * var1 * var1 / 524288.0 + (double)dig_P2 * var1) / 524288.0;
    var1 = (1.0 + var1 / 32768.0) * (double)dig_P1;

    if (var1 == 0) return 0;

    double p = 1048576.0 - (double)adc_P;
    p = (p - (var2 / 4096.0)) * 6250.0 / var1;
    var1 = (double)dig_P9 * p * p / 2147483648.0;
    var2 = p * (double)dig_P8 / 32768.0;
    p = p + (var1 + var2 + (double)dig_P7) / 16.0;

    return p;  /* 单位: Pa */
}

int main(void)
{
    i2c_fd = open("/dev/i2c-1", O_RDWR);
    if (i2c_fd < 0) { perror("open i2c"); return 1; }
    ioctl(i2c_fd, I2C_SLAVE, BMP280_ADDR);

    /* 验证芯片ID (BMP280 = 0x58) */
    uint8_t chip_id = bmp280_read_reg(0xD0);
    printf("BMP280 Chip ID: 0x%02X %s\n", chip_id,
           chip_id == 0x58 ? "(OK)" : "(ERROR!)");

    /* 读取校准参数 */
    bmp280_read_calibration();

    /* 配置: 正常模式, 温度x2过采样, 气压x16过采样 */
    bmp280_write_reg(0xF4, 0x57);  /* ctrl_meas: osrs_t=010, osrs_p=101, mode=11 */
    bmp280_write_reg(0xF5, 0x00);  /* config: standby=0.5ms, filter=off */

    printf("\n持续读取温度和气压...\n\n");

    for (int i = 0; i < 10; i++) {
        usleep(50000);  /* 等待转换 */

        /* 读取原始数据 (6字节: press[2:0], temp[2:0]) */
        uint8_t data[6];
        bmp280_read_regs(0xF7, data, 6);

        int32_t adc_P = ((int32_t)data[0] << 12) | ((int32_t)data[1] << 4) | (data[2] >> 4);
        int32_t adc_T = ((int32_t)data[3] << 12) | ((int32_t)data[4] << 4) | (data[5] >> 4);

        double temperature = bmp280_compensate_temperature(adc_T);
        double pressure = bmp280_compensate_pressure(adc_P);

        printf("[%2d] 温度: %.2f°C | 气压: %.2f hPa (%.0f Pa)\n",
               i + 1, temperature, pressure / 100.0, pressure);

        sleep(1);
    }

    close(i2c_fd);
    return 0;
}
```

```bash
gcc -o bmp280_test bmp280_i2c.c -lm
sudo ./bmp280_test
```

---

## 10. SPI — CAN总线控制器 MCP2515

MCP2515是Microchip出品的独立CAN控制器，通过SPI接口与主控通信，适合为没有CAN接口的开发板添加CAN总线功能。在工业控制和汽车电子中广泛使用。

### 10.1 硬件

**器件：** MCP2515 CAN模块（通常集成TJA1050 CAN收发器）

**接线：**

```
MCP2515模块        开发板
┌──────┐
│ VCC  │────────── 5V
│ GND  │────────── GND
│ CS   │────────── SPI CS0
│ SO   │────────── SPI MISO
│ SI   │────────── SPI MOSI
│ SCK  │────────── SPI CLK
│ INT  │────────── GPIO (中断引脚)
└──────┘

CAN总线侧:
│ CANH │────────── CAN总线 H线
│ CANL │────────── CAN总线 L线
```

| 开发板 | SPI | CS | INT(推荐) |
|--------|-----|-----|----------|
| OPi Zero 2 | SPI-1 | Pin 24 (PC3) | Pin 22 (PC14) |
| OPi 5 Max | SPI-4 | Pin 24 | Pin 22 |

**注意事项：**
- MCP2515模块通常需要5V供电（因为TJA1050收发器需要5V）
- SPI信号线是3.3V兼容的
- CAN总线两端需要120Ω终端电阻（很多模块自带跳线选择）
- INT引脚为低电平有效中断输出

### 10.2 软件驱动

#### 方法一：设备树overlay + SocketCAN（推荐）

Linux内核自带MCP2515驱动，通过设备树配置后可使用标准SocketCAN接口。

```dts
/* mcp2515-overlay.dts - MCP2515设备树overlay */
/dts-v1/;
/plugin/;

/ {
    fragment@0 {
        target = <&spi1>;
        __overlay__ {
            status = "okay";
            #address-cells = <1>;
            #size-cells = <0>;

            mcp2515: can@0 {
                compatible = "microchip,mcp2515";
                reg = <0>;                          /* CS0 */
                spi-max-frequency = <10000000>;     /* 10MHz */
                clocks = <&mcp2515_osc>;
                interrupt-parent = <&pio>;
                interrupts = <2 14 2>;              /* PC14, 下降沿触发 */
            };
        };
    };

    fragment@1 {
        target-path = "/";
        __overlay__ {
            mcp2515_osc: mcp2515-osc {
                compatible = "fixed-clock";
                #clock-cells = <0>;
                clock-frequency = <8000000>;  /* 模块晶振频率，常见8MHz或16MHz */
            };
        };
    };
};
```

```bash
# 编译设备树overlay
dtc -@ -I dts -O dtb -o mcp2515.dtbo mcp2515-overlay.dts

# 加载（具体方式取决于开发板）
sudo cp mcp2515.dtbo /boot/dtb/overlay/

# 重启后配置CAN接口
sudo ip link set can0 type can bitrate 500000  # 500Kbps
sudo ip link set can0 up

# 安装CAN工具
sudo apt install can-utils
```

#### 使用SocketCAN收发数据

```bash
# 发送CAN帧 (ID=0x123, 数据=DEADBEEF)
cansend can0 123#DEADBEEF

# 接收CAN帧
candump can0

# 发送远程帧
cansend can0 123#R

# 查看CAN统计
ip -details link show can0
```

#### C语言SocketCAN编程

```c
/* can_test.c - SocketCAN收发示例 */
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/can.h>
#include <linux/can/raw.h>

int main(void)
{
    int sock;
    struct sockaddr_can addr;
    struct ifreq ifr;
    struct can_frame frame;

    /* 创建CAN套接字 */
    sock = socket(PF_CAN, SOCK_RAW, CAN_RAW);
    if (sock < 0) {
        perror("socket");
        return 1;
    }

    /* 绑定到can0接口 */
    strcpy(ifr.ifr_name, "can0");
    ioctl(sock, SIOCGIFINDEX, &ifr);

    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;
    bind(sock, (struct sockaddr *)&addr, sizeof(addr));

    /* 发送CAN帧 */
    frame.can_id = 0x123;
    frame.can_dlc = 4;
    frame.data[0] = 0xDE;
    frame.data[1] = 0xAD;
    frame.data[2] = 0xBE;
    frame.data[3] = 0xEF;

    int nbytes = write(sock, &frame, sizeof(frame));
    printf("发送CAN帧: ID=0x%03X, DLC=%d, 数据=",
           frame.can_id, frame.can_dlc);
    for (int i = 0; i < frame.can_dlc; i++)
        printf("%02X ", frame.data[i]);
    printf("(%d bytes written)\n", nbytes);

    /* 接收CAN帧 */
    printf("\n等待接收CAN帧...\n");
    while (1) {
        nbytes = read(sock, &frame, sizeof(frame));
        if (nbytes > 0) {
            printf("收到: ID=0x%03X DLC=%d 数据=",
                   frame.can_id, frame.can_dlc);
            for (int i = 0; i < frame.can_dlc; i++)
                printf("%02X ", frame.data[i]);
            printf("\n");
        }
    }

    close(sock);
    return 0;
}
```

```bash
gcc -o can_test can_test.c
sudo ./can_test
```

---

## 11. USB — USB摄像头 (UVC)

USB摄像头遵循UVC (USB Video Class) 标准，Linux内核原生支持，即插即用。

### 11.1 硬件

**器件：** 任意UVC兼容USB摄像头（如罗技C270）

**接线：** 直接插入USB口即可

| 开发板 | 推荐USB口 |
|--------|----------|
| OPi Zero 2 | USB 2.0 Type-A |
| OPi 5 Max | USB 3.0 Type-A（带宽更大，支持高分辨率） |

**注意事项：**
- 确保内核已编译 `CONFIG_USB_VIDEO_CLASS=y` 或 `=m`
- 高分辨率摄像头建议使用USB 3.0口
- 供电不足可能导致摄像头不稳定，可使用带供电的USB Hub

### 11.2 软件驱动

#### 步骤一：验证摄像头识别

```bash
# 插入摄像头后检查
lsusb
# 应看到类似: Bus 001 Device 003: ID 046d:0825 Logitech, Inc. Webcam C270

dmesg | tail -20
# 应看到: uvcvideo: Found UVC 1.00 device ...

# 检查视频设备节点
ls /dev/video*
# 应看到 /dev/video0

# 查看摄像头支持的格式和分辨率
sudo apt install v4l-utils
v4l2-ctl --list-formats-ext -d /dev/video0
```

#### 步骤二：V4L2采集程序

```c
/* camera_capture.c - V4L2摄像头采集单帧图像 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/videodev2.h>

#define VIDEO_DEV   "/dev/video0"
#define WIDTH       640
#define HEIGHT      480
#define BUF_COUNT   4

struct buffer {
    void   *start;
    size_t  length;
};

static struct buffer buffers[BUF_COUNT];

int main(void)
{
    int fd = open(VIDEO_DEV, O_RDWR);
    if (fd < 0) { perror("open video"); return 1; }

    /* 查询设备能力 */
    struct v4l2_capability cap;
    ioctl(fd, VIDIOC_QUERYCAP, &cap);
    printf("驱动: %s\n设备: %s\n", cap.driver, cap.card);

    /* 设置格式: MJPEG 640x480 */
    struct v4l2_format fmt = {0};
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.width = WIDTH;
    fmt.fmt.pix.height = HEIGHT;
    fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_MJPEG;
    fmt.fmt.pix.field = V4L2_FIELD_NONE;
    ioctl(fd, VIDIOC_S_FMT, &fmt);

    printf("实际格式: %dx%d\n", fmt.fmt.pix.width, fmt.fmt.pix.height);

    /* 申请缓冲区 */
    struct v4l2_requestbuffers req = {0};
    req.count = BUF_COUNT;
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;
    ioctl(fd, VIDIOC_REQBUFS, &req);

    /* 映射缓冲区 */
    for (int i = 0; i < BUF_COUNT; i++) {
        struct v4l2_buffer buf = {0};
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = i;
        ioctl(fd, VIDIOC_QUERYBUF, &buf);

        buffers[i].length = buf.length;
        buffers[i].start = mmap(NULL, buf.length,
                                PROT_READ | PROT_WRITE, MAP_SHARED,
                                fd, buf.m.offset);

        /* 入队 */
        ioctl(fd, VIDIOC_QBUF, &buf);
    }

    /* 开始采集 */
    enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    ioctl(fd, VIDIOC_STREAMON, &type);

    /* 采集一帧 */
    struct v4l2_buffer buf = {0};
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;
    ioctl(fd, VIDIOC_DQBUF, &buf);

    /* 保存为JPEG文件 */
    FILE *fp = fopen("capture.jpg", "wb");
    fwrite(buffers[buf.index].start, buf.bytesused, 1, fp);
    fclose(fp);
    printf("已保存 capture.jpg (%d bytes)\n", buf.bytesused);

    /* 停止采集 */
    ioctl(fd, VIDIOC_STREAMOFF, &type);

    /* 清理 */
    for (int i = 0; i < BUF_COUNT; i++)
        munmap(buffers[i].start, buffers[i].length);
    close(fd);
    return 0;
}
```

```bash
gcc -o camera_capture camera_capture.c
sudo ./camera_capture
# 生成 capture.jpg
```

#### 使用OpenCV简化开发（Python快速验证）

```python
#!/usr/bin/env python3
# camera_opencv.py - OpenCV摄像头采集
import cv2

cap = cv2.VideoCapture(0)
cap.set(cv2.CAP_PROP_FRAME_WIDTH, 640)
cap.set(cv2.CAP_PROP_FRAME_HEIGHT, 480)

ret, frame = cap.read()
if ret:
    cv2.imwrite("capture_cv.jpg", frame)
    print(f"已保存 capture_cv.jpg, 分辨率: {frame.shape[1]}x{frame.shape[0]}")
else:
    print("采集失败")

cap.release()
```

```bash
pip3 install opencv-python-headless
python3 camera_opencv.py
```

---

## 12. I2C — 实时时钟 DS3231

DS3231是高精度I2C实时时钟(RTC)芯片，内置温补晶振(TCXO)，精度±2ppm，带电池可断电保持时间。

### 12.1 硬件

**器件：** DS3231 RTC模块（带CR2032电池座）

**接线：**

```
DS3231模块         开发板
┌──────┐
│ VCC  │────────── 3.3V
│ GND  │────────── GND
│ SCL  │────────── I2C SCL
│ SDA  │────────── I2C SDA
│ SQW  │────────── GPIO (可选，方波/闹钟中断输出)
│ 32K  │────────── (可选，32.768KHz输出)
└──────┘
```

| 开发板 | I2C总线 | SCL | SDA |
|--------|---------|-----|-----|
| OPi Zero 2 | I2C-1 | Pin 5 | Pin 3 |
| OPi 5 Max | I2C-5 | Pin 5 | Pin 3 |

DS3231 I2C地址固定为 `0x68`。

**注意事项：**
- 模块自带CR2032电池，断电后可保持时间数年
- 模块上通常有AT24C32 EEPROM（地址0x57），可忽略
- 精度极高，适合需要精确计时的场景

### 12.2 软件驱动

#### 方法一：使用内核RTC驱动（推荐）

```bash
# 加载DS3231内核驱动
sudo modprobe rtc-ds1307

# 手动绑定I2C设备
echo ds3231 0x68 | sudo tee /sys/bus/i2c/devices/i2c-1/new_device

# 验证
ls /dev/rtc*
# 应看到 /dev/rtc1 (rtc0通常是SoC内置RTC)

# 读取RTC时间
sudo hwclock -r -f /dev/rtc1

# 将系统时间写入RTC
sudo hwclock -w -f /dev/rtc1

# 从RTC同步系统时间
sudo hwclock -s -f /dev/rtc1
```

#### 方法二：用户空间I2C直接读写

```c
/* ds3231_rtc.c - DS3231 RTC读写 */
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <stdint.h>

#define DS3231_ADDR 0x68

static int i2c_fd;

/* BCD转十进制 */
static uint8_t bcd2dec(uint8_t bcd) { return (bcd >> 4) * 10 + (bcd & 0x0F); }
/* 十进制转BCD */
static uint8_t dec2bcd(uint8_t dec) { return ((dec / 10) << 4) | (dec % 10); }

static uint8_t ds3231_read_reg(uint8_t reg)
{
    write(i2c_fd, &reg, 1);
    uint8_t val;
    read(i2c_fd, &val, 1);
    return val;
}

static void ds3231_write_reg(uint8_t reg, uint8_t val)
{
    uint8_t buf[2] = {reg, val};
    write(i2c_fd, buf, 2);
}

/* 读取时间 */
static void ds3231_read_time(int *year, int *month, int *day,
                              int *hour, int *min, int *sec)
{
    *sec   = bcd2dec(ds3231_read_reg(0x00) & 0x7F);
    *min   = bcd2dec(ds3231_read_reg(0x01));
    *hour  = bcd2dec(ds3231_read_reg(0x02) & 0x3F);  /* 24小时制 */
    *day   = bcd2dec(ds3231_read_reg(0x04));
    *month = bcd2dec(ds3231_read_reg(0x05) & 0x1F);
    *year  = bcd2dec(ds3231_read_reg(0x06)) + 2000;
}

/* 设置时间 */
static void ds3231_set_time(int year, int month, int day,
                             int hour, int min, int sec)
{
    ds3231_write_reg(0x00, dec2bcd(sec));
    ds3231_write_reg(0x01, dec2bcd(min));
    ds3231_write_reg(0x02, dec2bcd(hour));
    ds3231_write_reg(0x04, dec2bcd(day));
    ds3231_write_reg(0x05, dec2bcd(month));
    ds3231_write_reg(0x06, dec2bcd(year - 2000));
}

/* 读取芯片温度 (DS3231内置温度传感器) */
static float ds3231_read_temperature(void)
{
    int8_t msb = (int8_t)ds3231_read_reg(0x11);
    uint8_t lsb = ds3231_read_reg(0x12);
    return msb + (lsb >> 6) * 0.25f;
}

int main(int argc, char *argv[])
{
    i2c_fd = open("/dev/i2c-1", O_RDWR);
    if (i2c_fd < 0) { perror("open i2c"); return 1; }
    ioctl(i2c_fd, I2C_SLAVE, DS3231_ADDR);

    if (argc == 7) {
        /* 设置时间: ./ds3231_rtc 2026 4 21 14 30 0 */
        int y = atoi(argv[1]), mo = atoi(argv[2]), d = atoi(argv[3]);
        int h = atoi(argv[4]), mi = atoi(argv[5]), s = atoi(argv[6]);
        ds3231_set_time(y, mo, d, h, mi, s);
        printf("RTC时间已设置: %04d-%02d-%02d %02d:%02d:%02d\n",
               y, mo, d, h, mi, s);
    }

    /* 读取并显示时间 */
    int year, month, day, hour, min, sec;
    ds3231_read_time(&year, &month, &day, &hour, &min, &sec);
    printf("RTC时间: %04d-%02d-%02d %02d:%02d:%02d\n",
           year, month, day, hour, min, sec);

    float temp = ds3231_read_temperature();
    printf("芯片温度: %.2f°C\n", temp);

    close(i2c_fd);
    return 0;
}
```

```bash
gcc -o ds3231_test ds3231_rtc.c
sudo ./ds3231_test                          # 读取时间
sudo ./ds3231_test 2026 4 21 14 30 0       # 设置时间
```

---

## 13. GPIO中断 — 红外接收 VS1838B

VS1838B是常见的红外接收头，配合NEC编码遥控器使用，可实现遥控功能。

### 13.1 硬件

**器件：** VS1838B红外接收头 + NEC编码红外遥控器

**接线：**

```
VS1838B (正面朝向自己，从左到右)
┌─────────┐
│ 1  2  3 │
│OUT GND VCC│
└─────────┘

VS1838B            开发板
┌──────┐
│ VCC  │────────── 3.3V
│ GND  │────────── GND
│ OUT  │────────── GPIO引脚 (支持中断)
└──────┘
```

| 开发板 | 推荐GPIO | 引脚 |
|--------|---------|------|
| OPi Zero 2 | PC14 | Pin 22 |
| OPi 5 Max | GPIO4_C6 | Pin 22 |

**注意事项：**
- VS1838B输出为低电平有效（空闲高电平，收到信号时拉低）
- 接收距离约10-15米
- 载波频率38KHz
- 建议在VCC和GND之间并联100nF去耦电容

### 13.2 软件驱动

#### 方法一：使用LIRC框架（推荐）

Linux内核的`gpio-ir-recv`驱动配合LIRC可以方便地解码红外信号。

```bash
# 安装LIRC
sudo apt install lirc

# 设备树overlay启用gpio-ir-recv
```

```dts
/* ir-recv-overlay.dts */
/dts-v1/;
/plugin/;

/ {
    fragment@0 {
        target-path = "/";
        __overlay__ {
            ir_recv: ir-receiver {
                compatible = "gpio-ir-receiver";
                gpios = <&pio 2 14 1>;  /* PC14, 低电平有效 */
                linux,rc-map-name = "rc-nec-terratec-cinergy-xs";
                status = "okay";
            };
        };
    };
};
```

```bash
# 加载后验证
ls /dev/lirc*
# 应看到 /dev/lirc0

# 测试接收原始红外数据
sudo mode2 -d /dev/lirc0
# 按遥控器按键，应看到 pulse/space 数据

# 录制遥控器按键
sudo irrecord -d /dev/lirc0 ~/my_remote.conf
```

#### 方法二：GPIO中断 + 软件解码NEC协议

```c
/* ir_nec_decode.c - 使用GPIO中断解码NEC红外协议 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gpiod.h>
#include <time.h>

#define CHIP_NAME   "gpiochip0"
#define IR_LINE     14  /* PC14 */

/* NEC协议时序 (微秒) */
#define NEC_HEADER_PULSE    9000
#define NEC_HEADER_SPACE    4500
#define NEC_BIT_PULSE       560
#define NEC_BIT_0_SPACE     560
#define NEC_BIT_1_SPACE     1690
#define TOLERANCE           200

static long get_us(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000000L + ts.tv_nsec / 1000;
}

int main(void)
{
    struct gpiod_chip *chip = gpiod_chip_open_by_name(CHIP_NAME);
    struct gpiod_line *line = gpiod_chip_get_line(chip, IR_LINE);

    /* 请求GPIO为输入，监听双边沿事件 */
    gpiod_line_request_both_edges_events(line, "ir-recv");

    printf("红外接收就绪，请按遥控器...\n\n");

    long timestamps[128];
    int count = 0;

    while (1) {
        struct gpiod_line_event event;
        struct timespec timeout = {0, 100000000};  /* 100ms超时 */

        int ret = gpiod_line_event_wait(line, &timeout);
        if (ret == 1) {
            gpiod_line_event_read(line, &event);
            timestamps[count++] = get_us();

            if (count >= 68) {  /* NEC: 2(header) + 32*2(data) + 2(stop) */
                /* 解码NEC数据 */
                uint32_t code = 0;
                int valid = 1;

                /* 跳过引导码，从第3个边沿开始解码 */
                for (int i = 0; i < 32 && valid; i++) {
                    int idx = 4 + i * 2;  /* 数据位起始索引 */
                    if (idx + 1 >= count) { valid = 0; break; }

                    long space = timestamps[idx + 1] - timestamps[idx];

                    if (space > NEC_BIT_1_SPACE - TOLERANCE &&
                        space < NEC_BIT_1_SPACE + TOLERANCE) {
                        code |= (1 << i);  /* bit 1 */
                    } else if (space > NEC_BIT_0_SPACE - TOLERANCE &&
                               space < NEC_BIT_0_SPACE + TOLERANCE) {
                        /* bit 0, 不需要操作 */
                    } else {
                        valid = 0;
                    }
                }

                if (valid) {
                    uint8_t addr     = code & 0xFF;
                    uint8_t addr_inv = (code >> 8) & 0xFF;
                    uint8_t cmd      = (code >> 16) & 0xFF;
                    uint8_t cmd_inv  = (code >> 24) & 0xFF;

                    printf("NEC码: 0x%08X | 地址: 0x%02X | 命令: 0x%02X",
                           code, addr, cmd);
                    if ((addr ^ addr_inv) == 0xFF && (cmd ^ cmd_inv) == 0xFF)
                        printf(" (校验通过)\n");
                    else
                        printf(" (扩展NEC)\n");
                }

                count = 0;
            }
        } else if (ret == 0) {
            /* 超时，重置 */
            if (count > 0) count = 0;
        }
    }

    gpiod_line_release(line);
    gpiod_chip_close(chip);
    return 0;
}
```

```bash
gcc -o ir_test ir_nec_decode.c -lgpiod
sudo ./ir_test
```

---

## 14. I2S — 音频DAC模块 PCM5102A

PCM5102A是TI出品的高品质I2S音频DAC，支持32位384kHz采样率，音质优秀。

### 14.1 硬件

**器件：** PCM5102A DAC模块（GY-PCM5102）

**接线：**

```
PCM5102A模块       开发板
┌──────┐
│ VCC  │────────── 3.3V
│ GND  │────────── GND
│ BCK  │────────── I2S BCLK (位时钟)
│ DIN  │────────── I2S DOUT (数据输出)
│ LCK  │────────── I2S LRCK (左右声道时钟)
│ SCK  │────────── GND (使用内部时钟) 或 I2S MCLK
│ FMT  │────────── GND (I2S标准格式)
│ XMT  │────────── 3.3V (使能输出)
└──────┘

音频输出:
│ L    │────────── 左声道 → 耳机/音箱
│ R    │────────── 右声道 → 耳机/音箱
│ GND  │────────── 音频地
```

| 开发板 | I2S接口 | BCLK | LRCK | DOUT |
|--------|---------|------|------|------|
| OPi Zero 2 | I2S0 | Pin 12 | Pin 35 | Pin 40 |
| OPi 5 Max | I2S1 | 参考原理图 | 参考原理图 | 参考原理图 |

**注意事项：**
- SCK引脚接GND让PCM5102A使用内部PLL生成系统时钟
- FMT引脚接GND选择I2S标准格式（接VCC为左对齐格式）
- XMT引脚必须拉高才能使能音频输出
- 模块输出为线路电平，可直接驱动耳机或接有源音箱

### 14.2 软件驱动

#### 步骤一：设备树配置

```dts
/* i2s-pcm5102a-overlay.dts */
/dts-v1/;
/plugin/;

/ {
    fragment@0 {
        target-path = "/";
        __overlay__ {
            pcm5102a_codec: pcm5102a {
                compatible = "ti,pcm5102a";
                #sound-dai-cells = <0>;
                status = "okay";
            };

            sound {
                compatible = "simple-audio-card";
                simple-audio-card,name = "PCM5102A";
                simple-audio-card,format = "i2s";

                simple-audio-card,cpu {
                    sound-dai = <&i2s0>;  /* 根据实际I2S控制器修改 */
                };

                simple-audio-card,codec {
                    sound-dai = <&pcm5102a_codec>;
                };
            };
        };
    };

    fragment@1 {
        target = <&i2s0>;
        __overlay__ {
            status = "okay";
        };
    };
};
```

#### 步骤二：验证和播放

```bash
# 重启后检查声卡
aplay -l
# 应看到: card X: PCM5102A [PCM5102A], device 0: ...

# 安装ALSA工具
sudo apt install alsa-utils

# 播放测试音
speaker-test -D plughw:X,0 -c 2 -t wav
# X替换为实际声卡编号

# 播放WAV文件
aplay -D plughw:X,0 test.wav

# 调节音量
alsamixer
```

#### 步骤三：ALSA编程播放

```c
/* pcm5102a_play.c - ALSA PCM播放示例 */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <alsa/asoundlib.h>

#define SAMPLE_RATE 44100
#define CHANNELS    2
#define DURATION    3  /* 秒 */

int main(void)
{
    snd_pcm_t *pcm;
    int err;

    /* 打开PCM设备 (根据实际声卡修改) */
    err = snd_pcm_open(&pcm, "plughw:1,0", SND_PCM_STREAM_PLAYBACK, 0);
    if (err < 0) {
        fprintf(stderr, "打开PCM失败: %s\n", snd_strerror(err));
        return 1;
    }

    /* 设置参数 */
    snd_pcm_set_params(pcm,
                       SND_PCM_FORMAT_S16_LE,    /* 16位有符号小端 */
                       SND_PCM_ACCESS_RW_INTERLEAVED,
                       CHANNELS,
                       SAMPLE_RATE,
                       1,                         /* 允许重采样 */
                       500000);                   /* 延迟500ms */

    /* 生成正弦波 (440Hz A4音) */
    int total_frames = SAMPLE_RATE * DURATION;
    int16_t *buffer = malloc(total_frames * CHANNELS * sizeof(int16_t));

    for (int i = 0; i < total_frames; i++) {
        double t = (double)i / SAMPLE_RATE;
        int16_t sample = (int16_t)(sin(2.0 * M_PI * 440.0 * t) * 16000);
        buffer[i * 2]     = sample;  /* 左声道 */
        buffer[i * 2 + 1] = sample;  /* 右声道 */
    }

    /* 播放 */
    printf("播放440Hz正弦波 %d秒...\n", DURATION);
    snd_pcm_sframes_t frames = snd_pcm_writei(pcm, buffer, total_frames);
    if (frames < 0)
        frames = snd_pcm_recover(pcm, frames, 0);

    printf("已播放 %ld 帧\n", frames);

    snd_pcm_drain(pcm);
    snd_pcm_close(pcm);
    free(buffer);
    return 0;
}
```

```bash
sudo apt install libasound2-dev
gcc -o pcm5102a_play pcm5102a_play.c -lasound -lm
./pcm5102a_play
```

---

## 15. 以太网/SPI — W5500网络模块

W5500是WIZnet出品的硬件TCP/IP协议栈以太网控制器，SPI接口，内置8个独立Socket，适合为没有以太网的设备添加网络功能。

### 15.1 硬件

**器件：** W5500以太网模块（带RJ45接口和变压器）

**接线：**

```
W5500模块          开发板
┌──────┐
│ 3.3V │────────── 3.3V
│ GND  │────────── GND
│ MISO │────────── SPI MISO
│ MOSI │────────── SPI MOSI
│ SCLK │────────── SPI CLK
│ SCS  │────────── SPI CS
│ INT  │────────── GPIO (中断，可选)
│ RST  │────────── GPIO (复位，可选)
└──────┘
```

**注意事项：**
- W5500工作电压3.3V，SPI时钟最高80MHz
- 内置硬件TCP/IP协议栈，不占用主控CPU资源
- 支持TCP、UDP、IPRAW、MACRAW模式
- 8个独立Socket，可同时处理8个连接

### 15.2 软件驱动

#### 方法一：内核驱动（推荐）

Linux内核自带W5500驱动（`w5100-spi`模块），通过设备树配置后作为标准网络接口使用。

```dts
/* w5500-overlay.dts */
/dts-v1/;
/plugin/;

/ {
    fragment@0 {
        target = <&spi1>;
        __overlay__ {
            status = "okay";
            #address-cells = <1>;
            #size-cells = <0>;

            w5500: ethernet@0 {
                compatible = "wiznet,w5500";
                reg = <0>;                      /* CS0 */
                spi-max-frequency = <30000000>; /* 30MHz */
                interrupt-parent = <&pio>;
                interrupts = <2 14 2>;          /* PC14, 下降沿 */
                local-mac-address = [00 11 22 33 44 55];
            };
        };
    };
};
```

```bash
# 加载驱动
sudo modprobe w5100-spi

# 重启后验证
ip link show
# 应看到新的 eth1 接口

# 配置IP
sudo ip addr add 192.168.1.100/24 dev eth1
sudo ip link set eth1 up

# 测试
ping 192.168.1.1
```

#### 方法二：用户空间SPI直接操作（学习用途）

```c
/* w5500_basic.c - W5500寄存器读写示例 */
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>
#include <stdint.h>

#define SPI_DEVICE "/dev/spidev1.0"
#define SPI_SPEED  10000000

static int spi_fd;

/* W5500 SPI帧格式: 16位地址 + 8位控制 + 数据 */
static void w5500_write_reg(uint16_t addr, uint8_t block, uint8_t val)
{
    uint8_t tx[4] = {
        addr >> 8,
        addr & 0xFF,
        (block << 3) | 0x04,  /* 写操作, VDM模式 */
        val
    };
    struct spi_ioc_transfer tr = {
        .tx_buf = (unsigned long)tx,
        .len = 4,
        .speed_hz = SPI_SPEED,
    };
    ioctl(spi_fd, SPI_IOC_MESSAGE(1), &tr);
}

static uint8_t w5500_read_reg(uint16_t addr, uint8_t block)
{
    uint8_t tx[4] = {
        addr >> 8,
        addr & 0xFF,
        (block << 3) | 0x00,  /* 读操作, VDM模式 */
        0x00
    };
    uint8_t rx[4] = {0};
    struct spi_ioc_transfer tr = {
        .tx_buf = (unsigned long)tx,
        .rx_buf = (unsigned long)rx,
        .len = 4,
        .speed_hz = SPI_SPEED,
    };
    ioctl(spi_fd, SPI_IOC_MESSAGE(1), &tr);
    return rx[3];
}

int main(void)
{
    spi_fd = open(SPI_DEVICE, O_RDWR);
    if (spi_fd < 0) { perror("open spi"); return 1; }

    uint8_t mode = SPI_MODE_0;
    ioctl(spi_fd, SPI_IOC_WR_MODE, &mode);

    /* 读取版本寄存器 (0x0039, Common Block 0) */
    uint8_t version = w5500_read_reg(0x0039, 0);
    printf("W5500 Version: 0x%02X (应为0x04)\n", version);

    /* 设置MAC地址 */
    uint8_t mac[6] = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55};
    for (int i = 0; i < 6; i++)
        w5500_write_reg(0x0009 + i, 0, mac[i]);

    /* 设置IP地址: 192.168.1.100 */
    uint8_t ip[4] = {192, 168, 1, 100};
    for (int i = 0; i < 4; i++)
        w5500_write_reg(0x000F + i, 0, ip[i]);

    printf("MAC和IP已配置\n");

    close(spi_fd);
    return 0;
}
```

---

## 16. SDIO — WiFi模块

Orange Pi Zero 2和5 Max都自带板载WiFi模块，通过SDIO接口连接。

### 16.1 硬件

| 开发板 | WiFi芯片 | 接口 | 频段 |
|--------|---------|------|------|
| OPi Zero 2 | Allwinner XR829 或 RTL8189FTV | SDIO | 2.4GHz |
| OPi 5 Max | AP6275P (BCM4375) | SDIO | 2.4GHz + 5GHz (WiFi 6) |

板载WiFi无需额外接线，天线已集成或通过IPEX座外接。

### 16.2 软件驱动

```bash
# 通常官方镜像已包含WiFi驱动，验证：
ip link show wlan0
# 或
iwconfig

# 如果没有wlan0，检查驱动加载
dmesg | grep -i wifi
dmesg | grep -i wlan
lsmod | grep -E "xr829|8189fs|bcmdhd"

# 扫描WiFi网络
sudo nmcli device wifi list

# 连接WiFi
sudo nmcli device wifi connect "SSID名称" password "密码"

# 或使用wpa_supplicant手动连接
sudo wpa_passphrase "SSID名称" "密码" | sudo tee /etc/wpa_supplicant/wpa_supplicant.conf
sudo wpa_supplicant -B -i wlan0 -c /etc/wpa_supplicant/wpa_supplicant.conf
sudo dhclient wlan0
```

#### 编程控制WiFi（使用nl80211 Netlink接口）

```c
/* wifi_scan.c - 使用iwlib扫描WiFi (简化示例) */
#include <stdio.h>
#include <string.h>
#include <iwlib.h>

int main(void)
{
    int sock = iw_sockets_open();
    if (sock < 0) {
        perror("socket");
        return 1;
    }

    wireless_scan_head head;
    wireless_scan *result;

    printf("扫描WiFi网络...\n\n");

    if (iw_scan(sock, "wlan0", IFACE_VERSION, &head) < 0) {
        perror("iw_scan");
        iw_sockets_close(sock);
        return 1;
    }

    result = head.result;
    int count = 0;
    while (result) {
        char essid[IW_ESSID_MAX_SIZE + 1] = {0};
        if (result->b.has_essid && result->b.essid_on) {
            strncpy(essid, result->b.essid, IW_ESSID_MAX_SIZE);
        }

        printf("[%d] SSID: %-32s | 信号: %d dBm | 频率: %.3f GHz\n",
               ++count, essid,
               result->stats.qual.level - 256,
               result->b.freq / 1e9);

        result = result->next;
    }

    printf("\n共发现 %d 个网络\n", count);
    iw_sockets_close(sock);
    return 0;
}
```

```bash
sudo apt install libiw-dev
gcc -o wifi_scan wifi_scan.c -liw
sudo ./wifi_scan
```

---

## 17. MIPI CSI — 摄像头模块 IMX415

IMX415是Sony出品的高性能CMOS图像传感器，4K分辨率，MIPI CSI-2接口。

> 注意：此部分仅适用于 Orange Pi 5 Max (RK3588)，Orange Pi Zero 2 不支持MIPI CSI。

### 17.1 硬件

**器件：** IMX415摄像头模块（MIPI CSI-2接口，15pin/22pin FPC排线）

**接线：**

```
IMX415模块 ──── FPC排线 ──── Orange Pi 5 Max CSI接口
                              (CAM0 或 CAM1)
```

| 接口 | 规格 |
|------|------|
| CAM0 | 4-lane MIPI CSI-2, 15pin FPC |
| CAM1 | 4-lane MIPI CSI-2, 22pin FPC |

**注意事项：**
- FPC排线有正反面，金手指朝向需与接口匹配
- 插拔排线时先抬起卡扣，插入后压下卡扣固定
- IMX415支持最高4K@30fps或1080p@60fps
- 需要确认摄像头模块的FPC接口类型与开发板匹配

### 17.2 软件驱动

#### 步骤一：启用摄像头设备树

```bash
# 使用orangepi-config启用摄像头
sudo orangepi-config
# -> System -> Hardware -> 启用 rk3588-camera-imx415

# 或手动编辑 /boot/orangepiEnv.txt
# 添加: overlays=rk3588-camera-imx415

# 重启后验证
ls /dev/video*
# 应看到多个video设备节点

# 查看媒体设备拓扑
sudo apt install v4l-utils
media-ctl -p -d /dev/media0
```

#### 步骤二：配置ISP管线

RK3588的摄像头需要通过RKISP (Image Signal Processor) 管线处理：

```bash
# 查看所有视频设备
v4l2-ctl --list-devices

# 配置媒体管线 (具体节点名称根据实际情况调整)
media-ctl -d /dev/media0 --set-v4l2 '"m00_b_imx415 4-001a":0[fmt:SGBRG10_1X10/3840x2160]'

# 使用v4l2采集
v4l2-ctl -d /dev/video0 --set-fmt-video=width=3840,height=2160,pixelformat=NV12
v4l2-ctl -d /dev/video0 --stream-mmap --stream-count=1 --stream-to=frame.raw
```

#### 步骤三：使用GStreamer采集和编码

```bash
# 安装GStreamer
sudo apt install gstreamer1.0-tools gstreamer1.0-plugins-bad \
     gstreamer1.0-rockchip

# 4K预览 (使用RK3588硬件编码)
gst-launch-1.0 v4l2src device=/dev/video0 ! \
    video/x-raw,format=NV12,width=3840,height=2160,framerate=30/1 ! \
    waylandsink

# 录制H.265视频 (硬件编码)
gst-launch-1.0 v4l2src device=/dev/video0 num-buffers=300 ! \
    video/x-raw,format=NV12,width=3840,height=2160,framerate=30/1 ! \
    mpph265enc ! h265parse ! mp4mux ! filesink location=output_4k.mp4

# 1080p RTSP推流
gst-launch-1.0 v4l2src device=/dev/video0 ! \
    video/x-raw,format=NV12,width=1920,height=1080,framerate=30/1 ! \
    mpph264enc ! rtph264pay ! \
    udpsink host=192.168.1.200 port=5000
```

#### 步骤四：V4L2编程采集

```c
/* mipi_capture.c - MIPI CSI摄像头V4L2采集 (简化版) */
/* 基本流程与USB摄像头相同(参考第11节)，主要区别: */
/* 1. 设备节点可能不同 (/dev/video0 或其他) */
/* 2. 像素格式通常为NV12而非MJPEG */
/* 3. 可能需要先配置media-ctl管线 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/videodev2.h>

#define VIDEO_DEV "/dev/video0"
#define WIDTH     1920
#define HEIGHT    1080

int main(void)
{
    int fd = open(VIDEO_DEV, O_RDWR);
    if (fd < 0) { perror("open"); return 1; }

    /* 设置NV12格式 */
    struct v4l2_format fmt = {0};
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.width = WIDTH;
    fmt.fmt.pix.height = HEIGHT;
    fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_NV12;
    ioctl(fd, VIDIOC_S_FMT, &fmt);

    printf("MIPI CSI采集: %dx%d NV12\n", fmt.fmt.pix.width, fmt.fmt.pix.height);
    printf("(完整V4L2采集流程参考第11节USB摄像头)\n");

    close(fd);
    return 0;
}
```

---

## 18. GPIO/I2C — 继电器模块

继电器是最常见的执行器，用于控制高压/大电流设备（如灯、电机、电磁阀）。

### 18.1 硬件

**器件：** 5V单路/多路继电器模块（带光耦隔离）

**接线（单路继电器）：**

```
继电器模块         开发板
┌──────┐
│ VCC  │────────── 5V (继电器线圈需要5V)
│ GND  │────────── GND
│ IN   │────────── GPIO引脚 (低电平触发)
└──────┘

高压侧 (注意安全！):
│ COM  │────────── 公共端 (接电源)
│ NO   │────────── 常开端 (继电器吸合时导通)
│ NC   │────────── 常闭端 (继电器释放时导通)
```

| 开发板 | 推荐GPIO | 引脚 |
|--------|---------|------|
| OPi Zero 2 | PC7 | Pin 13 |
| OPi 5 Max | GPIO1_A4 | Pin 7 |

**注意事项：**
- 大多数继电器模块为低电平触发（IN=LOW时继电器吸合）
- 模块自带光耦隔离和续流二极管，可直接接3.3V GPIO
- 操作高压（220V交流）时务必注意安全，确保绝缘
- 继电器有机械寿命，频繁开关建议使用固态继电器(SSR)

### 18.2 软件驱动

继电器本质上就是GPIO控制，驱动方式与LED完全相同。

```c
/* relay_control.c - 继电器控制 */
#include <gpiod.h>
#include <stdio.h>
#include <unistd.h>

#define CHIP_NAME   "gpiochip0"
#define RELAY_LINE  7  /* 根据实际GPIO修改 */

int main(int argc, char *argv[])
{
    struct gpiod_chip *chip = gpiod_chip_open_by_name(CHIP_NAME);
    if (!chip) { perror("open chip"); return 1; }

    struct gpiod_line *relay = gpiod_chip_get_line(chip, RELAY_LINE);
    if (!relay) { perror("get line"); gpiod_chip_close(chip); return 1; }

    /* 初始状态: 高电平(继电器断开，低电平触发模块) */
    gpiod_line_request_output(relay, "relay-ctrl", 1);

    if (argc > 1) {
        if (argv[1][0] == '1') {
            gpiod_line_set_value(relay, 0);  /* 低电平 = 继电器吸合 */
            printf("继电器: ON (吸合)\n");
        } else {
            gpiod_line_set_value(relay, 1);  /* 高电平 = 继电器断开 */
            printf("继电器: OFF (断开)\n");
        }
    } else {
        /* 演示: 开关5次 */
        for (int i = 0; i < 5; i++) {
            gpiod_line_set_value(relay, 0);
            printf("继电器 ON\n");
            sleep(2);

            gpiod_line_set_value(relay, 1);
            printf("继电器 OFF\n");
            sleep(2);
        }
    }

    gpiod_line_release(relay);
    gpiod_chip_close(chip);
    return 0;
}
```

```bash
gcc -o relay_ctrl relay_control.c -lgpiod
sudo ./relay_ctrl 1    # 打开继电器
sudo ./relay_ctrl 0    # 关闭继电器
sudo ./relay_ctrl      # 循环开关演示
```

#### 多路继电器 (I2C扩展，使用PCF8574)

如果需要控制多路继电器（8路/16路），可以使用PCF8574 I2C IO扩展芯片：

```c
/* relay_i2c.c - 通过PCF8574 I2C控制8路继电器 */
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <stdint.h>

#define PCF8574_ADDR 0x20  /* A0=A1=A2=GND */

int main(void)
{
    int fd = open("/dev/i2c-1", O_RDWR);
    ioctl(fd, I2C_SLAVE, PCF8574_ADDR);

    /* PCF8574: 写入一个字节控制8个IO口 */
    /* 低电平触发: 0=继电器ON, 1=继电器OFF */

    uint8_t state = 0xFF;  /* 全部关闭 */
    write(fd, &state, 1);
    printf("全部继电器关闭\n");
    sleep(1);

    /* 逐个打开 */
    for (int i = 0; i < 8; i++) {
        state &= ~(1 << i);  /* 第i位置0 = 打开第i路 */
        write(fd, &state, 1);
        printf("继电器 %d ON (state=0x%02X)\n", i + 1, state);
        usleep(500000);
    }

    sleep(2);

    /* 全部关闭 */
    state = 0xFF;
    write(fd, &state, 1);
    printf("全部继电器关闭\n");

    close(fd);
    return 0;
}
```

---

## 附录A：开发环境搭建

### 通用依赖安装

```bash
# 基础开发工具
sudo apt update
sudo apt install -y build-essential gcc g++ make cmake

# GPIO/I2C/SPI工具和库
sudo apt install -y gpiod libgpiod-dev i2c-tools libi2c-dev \
     spidev python3-spidev

# 多媒体相关
sudo apt install -y v4l-utils libv4l-dev libasound2-dev \
     gstreamer1.0-tools

# 网络工具
sudo apt install -y can-utils libiw-dev net-tools wireless-tools

# Python开发
sudo apt install -y python3-dev python3-pip python3-smbus python3-gpiod
```

### 内核头文件（编译内核模块需要）

```bash
# Orange Pi官方镜像通常已包含
sudo apt install linux-headers-$(uname -r)

# 如果没有，需要从Orange Pi官方源码编译
```

### 设备树overlay编译

```bash
# 安装设备树编译器
sudo apt install device-tree-compiler

# 编译overlay
dtc -@ -I dts -O dtb -o my_overlay.dtbo my_overlay.dts

# 放置到overlay目录
sudo cp my_overlay.dtbo /boot/dtb/overlay/

# 在 /boot/orangepiEnv.txt 中添加
# overlays=my_overlay
```

## 附录B：调试技巧

### GPIO调试

```bash
# 查看所有GPIO状态
cat /sys/kernel/debug/gpio

# 使用gpioinfo查看GPIO芯片信息
gpioinfo gpiochip0

# 监控GPIO电平变化
gpiomon gpiochip0 14  # 监控第14号GPIO
```

### I2C调试

```bash
# 扫描I2C总线上的设备
sudo i2cdetect -y 1

# 读取设备寄存器
sudo i2cget -y 1 0x68 0x00  # 读取地址0x68设备的0x00寄存器

# 写入设备寄存器
sudo i2cset -y 1 0x68 0x00 0x00  # 写入

# dump所有寄存器
sudo i2cdump -y 1 0x68
```

### SPI调试

```bash
# 查看SPI设备
ls /dev/spidev*

# SPI回环测试 (MOSI接MISO)
# 使用spi-tools
sudo apt install spi-tools
spi-config -d /dev/spidev1.0 -q
```

### 内核日志

```bash
# 实时查看内核日志
dmesg -w

# 过滤特定驱动日志
dmesg | grep -i "i2c\|spi\|gpio\|usb"

# 查看设备树
dtc -I fs /sys/firmware/devicetree/base | less
```

### 逻辑分析仪

对于时序敏感的协议调试（SPI、I2C、UART、1-Wire），强烈建议使用逻辑分析仪：
- 推荐：Saleae Logic 或国产DSLogic
- 配合开源软件 PulseView (sigrok) 使用
- 可以直观看到信号波形和协议解码

```bash
# 安装PulseView
sudo apt install pulseview sigrok
```

---

## 附录C：常见问题排查

| 问题 | 可能原因 | 解决方法 |
|------|---------|---------|
| GPIO无法导出 | 引脚被其他功能占用 | 检查设备树，禁用冲突的overlay |
| I2C设备检测不到 | 接线错误/未启用I2C | 检查接线，确认I2C overlay已启用 |
| SPI设备无响应 | CS引脚配置错误 | 确认CS引脚，检查SPI模式(CPOL/CPHA) |
| 权限不足 | 需要root权限 | 使用sudo，或将用户加入gpio/i2c/spi组 |
| 设备树overlay不生效 | 编译错误或路径错误 | 检查dmesg日志，确认dtbo文件正确 |
| 内核模块编译失败 | 缺少内核头文件 | 安装linux-headers包 |
| 外设供电不足 | USB口电流不够 | 使用外部供电或带供电USB Hub |

---

> 本文档涵盖了嵌入式Linux开发中最常见的外设类型和驱动方法。每种外设都提供了硬件接线和软件驱动的完整示例，可以直接在Orange Pi Zero 2或Orange Pi 5 Max上实践。建议从GPIO LED开始，逐步深入到I2C、SPI等更复杂的接口。
