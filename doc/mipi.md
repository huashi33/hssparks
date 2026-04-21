# Orange Pi 5 Max 接入 RM69330 MIPI DSI 屏幕 — 完整适配指南

> 开发板：Orange Pi 5 Max（RK3588，非 RK3588S）
> 屏幕：Raydium RM69330 AMOLED（MIPI DSI 接口）

---

## 1. 硬件概述

### 1.1 Orange Pi 5 Max 开发板

- SoC：Rockchip RK3588（8 核，4×A76 + 4×A55）
- 内存：4GB / 8GB / 16GB LPDDR5
- 显示输出：2× HDMI 2.1（最高 8K@60）+ 1× 4-lane MIPI DSI（最高 4K@60）
- MIPI DSI 连接器：板载 30pin FPC 座（0.5mm 间距），对应 RK3588 的 DSI0 控制器
- 摄像头接口：2× 4-lane MIPI CSI + 1× 4-lane MIPI D-DPHY Rx
- 40pin GPIO 扩展头（UART、I2C、SPI、PWM 等）
- 官方系统：Orange Pi OS（Android/Arch/OpenHarmony）、Ubuntu 20.04/22.04、Debian 11/12
- 内核版本：官方 SDK 使用 Rockchip vendor kernel 5.10.x（推荐）

### 1.2 RM69330 屏幕简介

- 驱动IC：Raydium RM69330
- 接口：MIPI DSI（1/2 lane）
- 典型分辨率：454×454（圆形 AMOLED）或 360×360
- 色深：RGB888 / RGB565
- 供电：VDDI 1.8V，VCI 2.8~3.3V
- 命令模式（Command Mode）为主，部分支持 Video Mode

### 1.3 RK3588 MIPI DSI 控制器

RK3588 集成两个 MIPI DSI 控制器（DSI0 / DSI1），每个支持：
- 最高 4 lane，每 lane 2.5Gbps
- Video Mode 和 Command Mode
- DSC（Display Stream Compression）
- Samsung DCPHY（DPHY 2.0 + CPHY 1.1）

Orange Pi 5 Max 板载 DSI 连接器接的是 DSI0。

---

## 2. 硬件连接

### 2.1 Orange Pi 5 Max 板载 MIPI DSI 连接器（30pin FPC）

Orange Pi 5 Max 的 MIPI DSI 连接器为 30pin、0.5mm 间距 FPC 座。典型引脚定义如下（参考 Orange Pi RK3588 系列原理图）：

| Pin | 信号 | 说明 |
|-----|------|------|
| 1   | VCC_LCD (3.3V) | LCD 电源 3.3V |
| 2   | VCC_LCD (3.3V) | LCD 电源 3.3V |
| 3   | VCCIO_LCD (1.8V) | LCD IO 电源 1.8V |
| 4   | VCCIO_LCD (1.8V) | LCD IO 电源 1.8V |
| 5   | LCD_RESET | 复位信号（GPIO 控制） |
| 6   | NC / TE | Tearing Effect 或空 |
| 7   | GND | 地 |
| 8   | MIPI_DSI0_TX_D0N | Data Lane 0 负 |
| 9   | MIPI_DSI0_TX_D0P | Data Lane 0 正 |
| 10  | GND | 地 |
| 11  | MIPI_DSI0_TX_D1N | Data Lane 1 负 |
| 12  | MIPI_DSI0_TX_D1P | Data Lane 1 正 |
| 13  | GND | 地 |
| 14  | MIPI_DSI0_TX_CLKN | Clock Lane 负 |
| 15  | MIPI_DSI0_TX_CLKP | Clock Lane 正 |
| 16  | GND | 地 |
| 17  | MIPI_DSI0_TX_D2N | Data Lane 2 负 |
| 18  | MIPI_DSI0_TX_D2P | Data Lane 2 正 |
| 19  | GND | 地 |
| 20  | MIPI_DSI0_TX_D3N | Data Lane 3 负 |
| 21  | MIPI_DSI0_TX_D3P | Data Lane 3 正 |
| 22  | GND | 地 |
| 23  | TP_INT | 触摸中断（I2C 触摸屏用） |
| 24  | TP_RST | 触摸复位 |
| 25  | TP_SDA | 触摸 I2C 数据 |
| 26  | TP_SCL | 触摸 I2C 时钟 |
| 27  | GND | 地 |
| 28  | GND | 地 |
| 29  | LCD_BL_PWM | 背光 PWM 控制 |
| 30  | LCD_BL_EN | 背光使能 |

> **重要**：以上引脚定义为 Orange Pi RK3588 系列的典型排列，实际请以 Orange Pi 5 Max 原理图为准。
> 原理图可从 [Orange Pi 官网下载页](http://www.orangepi.org/html/hardWare/computerAndMicrocontrollers/details/Orange-Pi-5-Max.html) 获取。

### 2.2 RM69330 屏幕与 30pin 连接器的适配

RM69330 是小尺寸 AMOLED 屏（通常只需 1~2 lane），而 Orange Pi 5 Max 的 DSI 连接器是 4-lane 的。你需要做一块 FPC 转接板：

```
Orange Pi 5 Max                    转接板                    RM69330 屏幕 FPC
30pin FPC 座 ──── FPC 排线 ──── 转接 PCB ──── FPC 排线 ──── 屏幕 FPC 座
```

转接板需要完成：
1. **信号映射**：将 30pin 中的 D0P/D0N + CLKP/CLKN 接到屏幕对应引脚
2. **电源适配**：30pin 连接器已提供 3.3V 和 1.8V，直接引到屏幕 VCI 和 VDDI
3. **RESET 信号**：Pin 5 (LCD_RESET) 接到屏幕 RESET
4. **未用 lane 悬空**：D1/D2/D3 不接（RM69330 只用 1 lane）
5. **触摸引脚**：RM69330 如果没有触摸功能，Pin 23~26 悬空

### 2.3 接线对照表

| Orange Pi 5 Max 30pin | 信号 | RM69330 屏幕引脚 |
|----------------------|------|-----------------|
| Pin 1,2 (VCC_LCD 3.3V) | 电源 | VCI (3.3V) |
| Pin 3,4 (VCCIO 1.8V) | IO电源 | VDDI (1.8V) |
| Pin 5 (LCD_RESET) | 复位 | RESET |
| Pin 7,10,13,16 (GND) | 地 | GND |
| Pin 8 (DSI0_D0N) | Data0- | DSI_D0N |
| Pin 9 (DSI0_D0P) | Data0+ | DSI_D0P |
| Pin 14 (DSI0_CLKN) | CLK- | DSI_CLKN |
| Pin 15 (DSI0_CLKP) | CLK+ | DSI_CLKP |
| Pin 6 (TE) | 帧同步 | TE（可选） |
| Pin 29 (BL_PWM) | 背光 | 不接（AMOLED 无需） |

### 2.4 硬件设计要点

1. **转接板走线**：MIPI 差分对等长，阻抗 100Ω（差分），走线尽量短（<5cm）
2. **电源滤波**：VCI 和 VDDI 在转接板上各加 10μF + 100nF 去耦电容
3. **FPC 排线**：选用 0.5mm 间距 30pin FPC 排线，长度不超过 10cm
4. **ESD 防护**：MIPI 数据线建议加 ESD 保护器件（如 TPD4E05U06）

### 2.5 连接示意图

```
Orange Pi 5 Max                              RM69330 屏幕
┌──────────────┐                            ┌──────────┐
│ 30pin FPC座  │                            │          │
│  Pin9  D0P  ├────────────────────────────┤ D0P      │
│  Pin8  D0N  ├────────────────────────────┤ D0N      │
│  Pin15 CLKP ├────────────────────────────┤ CLKP     │
│  Pin14 CLKN ├────────────────────────────┤ CLKN     │
│              │                            │          │
│  Pin5  RST  ├────────────────────────────┤ RESET    │
│  Pin6  TE   ├────────────────────────────┤ TE       │
│              │                            │          │
│  Pin1,2 3.3V├────[10μF+100nF]───────────┤ VCI      │
│  Pin3,4 1.8V├────[10μF+100nF]───────────┤ VDDI     │
│  Pin7   GND ├────────────────────────────┤ GND      │
│              │                            │          │
│  Pin11~21   │  (D1/D2/D3 悬空不接)       │          │
└──────────────┘                            └──────────┘
```

---

## 3. 获取与准备内核源码

### 3.1 获取 Orange Pi 5 Max SDK

```bash
# 从 Orange Pi 官网下载页获取 Linux SDK
# 通常包含：u-boot、kernel（5.10.x vendor）、buildroot/debian rootfs

# 解压后目录结构类似：
# orangepi-build/
# ├── kernel/               # Linux 内核源码
# ├── u-boot/               # U-Boot 源码
# ├── external/              # 外部工具
# └── build.sh              # 编译脚本

# 或者使用 Orange Pi 提供的 GitHub 仓库
git clone https://github.com/orangepi-xunlong/linux-orangepi.git -b orange-pi-5.10-rk35xx
```

### 3.2 确认设备树文件

Orange Pi 5 Max 的设备树文件位于：

```
arch/arm64/boot/dts/rockchip/rk3588-orangepi-5-max.dts
```

这是我们需要修改的主要文件。

---

## 4. 设备树（Device Tree）配置

### 4.1 创建 DSI Panel Overlay 或直接修改 DTS

推荐方式：在 `rk3588-orangepi-5-max.dts` 中添加 DSI panel 节点。

```dts
/* 启用 MIPI DCPHY0（Samsung DPHY） */
&mipi_dcphy0 {
    status = "okay";
};

/* 启用 DSI0 控制器 */
&dsi0 {
    status = "okay";
    #address-cells = <1>;
    #size-cells = <0>;

    /* DSI 时钟配置 */
    rockchip,lane-rate = <480>;  /* Mbps per lane，根据屏幕需求调整 */

    panel@0 {
        compatible = "raydium,rm69330";
        reg = <0>;

        /* 复位引脚 — 对应 Orange Pi 5 Max DSI 连接器 Pin5
         * 具体 GPIO 编号需查原理图，以下为示例 */
        reset-gpios = <&gpio2 RK_PC1 GPIO_ACTIVE_LOW>;

        /* TE 引脚（Command Mode 帧同步，可选） */
        /* te-gpios = <&gpio2 RK_PC2 GPIO_ACTIVE_HIGH>; */

        /* 电源 — Orange Pi 5 Max DSI 连接器已提供 3.3V 和 1.8V
         * 如果是板载 regulator 直接供电，可使用 fixed regulator */
        vci-supply = <&vcc3v3_lcd>;
        vddi-supply = <&vcc1v8_lcd>;

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
```

### 4.2 显示通路绑定（VOP2 → DSI0）

RK3588 的 VOP2 有 4 个 Video Port（VP0~VP3），小分辨率屏用 VP2 或 VP3 即可：

```dts
&vop {
    status = "okay";
};

/* VP2 适合小分辨率（≤2048x1536） */
&vp2 {
    status = "okay";
};

&route_dsi0 {
    status = "okay";
    connect = <&vp2_out_dsi0>;
};
```

> **注意**：如果 VP2 已被 HDMI 占用，可改用 VP3。Orange Pi 5 Max 默认 VP0 给 HDMI0，VP1 给 HDMI1。

### 4.3 电源 Regulator 定义

Orange Pi 5 Max 的 DSI 连接器电源通常由板载 PMIC（RK806）提供。如果原理图中 DSI 电源是常供的，可定义为 fixed regulator：

```dts
vcc3v3_lcd: vcc3v3-lcd-regulator {
    compatible = "regulator-fixed";
    regulator-name = "vcc3v3_lcd";
    regulator-min-microvolt = <3300000>;
    regulator-max-microvolt = <3300000>;
    /* 如果有 GPIO 控制使能，添加以下行 */
    /* gpio = <&gpio4 RK_PA3 GPIO_ACTIVE_HIGH>; */
    /* enable-active-high; */
    regulator-boot-on;
    regulator-always-on;
};

vcc1v8_lcd: vcc1v8-lcd-regulator {
    compatible = "regulator-fixed";
    regulator-name = "vcc1v8_lcd";
    regulator-min-microvolt = <1800000>;
    regulator-max-microvolt = <1800000>;
    regulator-boot-on;
    regulator-always-on;
};
```

### 4.4 Pinctrl 配置

```dts
&pinctrl {
    lcd {
        lcd_rst: lcd-rst {
            rockchip,pins = <2 RK_PC1 RK_FUNC_GPIO &pcfg_pull_none>;
        };
    };
};
```

---

## 5. 内核驱动开发

### 5.1 驱动文件位置

```
drivers/gpu/drm/panel/panel-raydium-rm69330.c
```

### 5.2 驱动框架代码

```c
// SPDX-License-Identifier: GPL-2.0
/*
 * Panel driver for Raydium RM69330 AMOLED
 * Target: Orange Pi 5 Max (RK3588)
 */

#include <linux/delay.h>
#include <linux/gpio/consumer.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/regulator/consumer.h>

#include <video/mipi_display.h>

#include <drm/drm_mipi_dsi.h>
#include <drm/drm_modes.h>
#include <drm/drm_panel.h>

struct rm69330_panel {
    struct drm_panel panel;
    struct mipi_dsi_device *dsi;
    struct gpio_desc *reset_gpio;
    struct regulator *vci;
    struct regulator *vddi;
    bool prepared;
};

static inline struct rm69330_panel *to_rm69330(struct drm_panel *panel)
{
    return container_of(panel, struct rm69330_panel, panel);
}

/* RM69330 初始化序列 — 根据屏厂提供的 init code 填写 */
static int rm69330_init_sequence(struct rm69330_panel *ctx)
{
    struct mipi_dsi_device *dsi = ctx->dsi;
    int ret;

    /* Manufacturer Command Access Protect */
    ret = mipi_dsi_dcs_write_seq(dsi, 0xFE, 0x01);
    if (ret < 0) return ret;

    /* 以下为示例，实际序列需根据屏厂 spec 填写 */
    mipi_dsi_dcs_write_seq(dsi, 0x06, 0x62);
    mipi_dsi_dcs_write_seq(dsi, 0x0E, 0x80);
    mipi_dsi_dcs_write_seq(dsi, 0x0F, 0x80);
    mipi_dsi_dcs_write_seq(dsi, 0x10, 0x71);
    mipi_dsi_dcs_write_seq(dsi, 0x13, 0x81);
    mipi_dsi_dcs_write_seq(dsi, 0x14, 0x81);
    mipi_dsi_dcs_write_seq(dsi, 0x15, 0x82);
    mipi_dsi_dcs_write_seq(dsi, 0x16, 0x82);
    mipi_dsi_dcs_write_seq(dsi, 0x18, 0x88);
    mipi_dsi_dcs_write_seq(dsi, 0x19, 0x55);
    mipi_dsi_dcs_write_seq(dsi, 0x1A, 0x10);
    mipi_dsi_dcs_write_seq(dsi, 0x1C, 0x99);
    mipi_dsi_dcs_write_seq(dsi, 0x1D, 0x03);
    mipi_dsi_dcs_write_seq(dsi, 0x1E, 0x03);
    mipi_dsi_dcs_write_seq(dsi, 0x1F, 0x03);
    mipi_dsi_dcs_write_seq(dsi, 0x20, 0x03);

    /* 切回 User Command Set */
    mipi_dsi_dcs_write_seq(dsi, 0xFE, 0x00);

    /* 设置像素格式 RGB888 */
    mipi_dsi_dcs_set_pixel_format(dsi, MIPI_DCS_PIXEL_FMT_24BIT);

    /* 设置列/行地址（根据分辨率） */
    mipi_dsi_dcs_set_column_address(dsi, 0, 454 - 1);
    mipi_dsi_dcs_set_page_address(dsi, 0, 454 - 1);

    /* TE 输出使能 */
    mipi_dsi_dcs_set_tear_on(dsi, MIPI_DSI_DCS_TEAR_MODE_VBLANK);

    /* Sleep Out */
    ret = mipi_dsi_dcs_exit_sleep_mode(dsi);
    if (ret < 0) return ret;
    msleep(120);

    /* Display On */
    ret = mipi_dsi_dcs_set_display_on(dsi);
    if (ret < 0) return ret;
    msleep(20);

    return 0;
}

static int rm69330_prepare(struct drm_panel *panel)
{
    struct rm69330_panel *ctx = to_rm69330(panel);
    int ret;

    if (ctx->prepared)
        return 0;

    /* 上电时序：VDDI → VCI → 延时 → 释放 RESET */
    ret = regulator_enable(ctx->vddi);
    if (ret < 0) return ret;

    usleep_range(2000, 5000);

    ret = regulator_enable(ctx->vci);
    if (ret < 0) goto err_vci;

    usleep_range(10000, 15000);

    /* 复位时序 */
    gpiod_set_value_cansleep(ctx->reset_gpio, 1);
    usleep_range(5000, 10000);
    gpiod_set_value_cansleep(ctx->reset_gpio, 0);
    usleep_range(10000, 15000);
    gpiod_set_value_cansleep(ctx->reset_gpio, 1);
    usleep_range(10000, 15000);

    /* 发送初始化序列 */
    ret = rm69330_init_sequence(ctx);
    if (ret < 0) goto err_init;

    ctx->prepared = true;
    return 0;

err_init:
    gpiod_set_value_cansleep(ctx->reset_gpio, 0);
    regulator_disable(ctx->vci);
err_vci:
    regulator_disable(ctx->vddi);
    return ret;
}

static int rm69330_unprepare(struct drm_panel *panel)
{
    struct rm69330_panel *ctx = to_rm69330(panel);

    if (!ctx->prepared)
        return 0;

    mipi_dsi_dcs_set_display_off(ctx->dsi);
    msleep(20);
    mipi_dsi_dcs_enter_sleep_mode(ctx->dsi);
    msleep(120);

    gpiod_set_value_cansleep(ctx->reset_gpio, 0);
    regulator_disable(ctx->vci);
    usleep_range(5000, 10000);
    regulator_disable(ctx->vddi);

    ctx->prepared = false;
    return 0;
}

/* 屏幕时序参数 — 根据屏厂 datasheet 调整 */
static const struct drm_display_mode rm69330_mode = {
    .clock       = 16000,       /* pixel clock kHz */
    .hdisplay    = 454,
    .hsync_start = 454 + 20,    /* hdisplay + HFP */
    .hsync_end   = 454 + 20 + 4,  /* + HSW */
    .htotal      = 454 + 20 + 4 + 20, /* + HBP */
    .vdisplay    = 454,
    .vsync_start = 454 + 20,    /* vdisplay + VFP */
    .vsync_end   = 454 + 20 + 2,  /* + VSW */
    .vtotal      = 454 + 20 + 2 + 20, /* + VBP */
    .width_mm    = 30,
    .height_mm   = 30,
    .type        = DRM_MODE_TYPE_DRIVER | DRM_MODE_TYPE_PREFERRED,
};

static int rm69330_get_modes(struct drm_panel *panel,
                             struct drm_connector *connector)
{
    struct drm_display_mode *mode;

    mode = drm_mode_duplicate(connector->dev, &rm69330_mode);
    if (!mode)
        return -ENOMEM;

    drm_mode_set_name(mode);
    drm_mode_probed_add(connector, mode);
    connector->display_info.width_mm = mode->width_mm;
    connector->display_info.height_mm = mode->height_mm;

    return 1;
}

static const struct drm_panel_funcs rm69330_panel_funcs = {
    .prepare  = rm69330_prepare,
    .unprepare = rm69330_unprepare,
    .get_modes = rm69330_get_modes,
};

static int rm69330_probe(struct mipi_dsi_device *dsi)
{
    struct device *dev = &dsi->dev;
    struct rm69330_panel *ctx;
    int ret;

    ctx = devm_kzalloc(dev, sizeof(*ctx), GFP_KERNEL);
    if (!ctx)
        return -ENOMEM;

    ctx->dsi = dsi;
    mipi_dsi_set_drvdata(dsi, ctx);

    /* RM69330 只需 1 lane */
    dsi->lanes = 1;
    dsi->format = MIPI_DSI_FMT_RGB888;
    dsi->mode_flags = MIPI_DSI_MODE_LPM |
                      MIPI_DSI_MODE_NO_EOT_PACKET |
                      MIPI_DSI_CLOCK_NON_CONTINUOUS;

    ctx->reset_gpio = devm_gpiod_get(dev, "reset", GPIOD_OUT_LOW);
    if (IS_ERR(ctx->reset_gpio))
        return dev_err_probe(dev, PTR_ERR(ctx->reset_gpio),
                             "Failed to get reset GPIO\n");

    ctx->vci = devm_regulator_get(dev, "vci");
    if (IS_ERR(ctx->vci))
        return dev_err_probe(dev, PTR_ERR(ctx->vci),
                             "Failed to get VCI regulator\n");

    ctx->vddi = devm_regulator_get(dev, "vddi");
    if (IS_ERR(ctx->vddi))
        return dev_err_probe(dev, PTR_ERR(ctx->vddi),
                             "Failed to get VDDI regulator\n");

    drm_panel_init(&ctx->panel, dev, &rm69330_panel_funcs,
                   DRM_MODE_CONNECTOR_DSI);

    drm_panel_add(&ctx->panel);

    ret = mipi_dsi_attach(dsi);
    if (ret < 0) {
        drm_panel_remove(&ctx->panel);
        return ret;
    }

    dev_info(dev, "RM69330 panel attached on Orange Pi 5 Max DSI0\n");
    return 0;
}

static void rm69330_remove(struct mipi_dsi_device *dsi)
{
    struct rm69330_panel *ctx = mipi_dsi_get_drvdata(dsi);

    mipi_dsi_detach(dsi);
    drm_panel_remove(&ctx->panel);
}

static const struct of_device_id rm69330_of_match[] = {
    { .compatible = "raydium,rm69330" },
    { /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, rm69330_of_match);

static struct mipi_dsi_driver rm69330_driver = {
    .probe  = rm69330_probe,
    .remove = rm69330_remove,
    .driver = {
        .name = "panel-raydium-rm69330",
        .of_match_table = rm69330_of_match,
    },
};
module_mipi_dsi_driver(rm69330_driver);

MODULE_DESCRIPTION("DRM panel driver for Raydium RM69330 AMOLED on Orange Pi 5 Max");
MODULE_LICENSE("GPL");
```

### 5.3 Kconfig 与 Makefile

在 `drivers/gpu/drm/panel/Kconfig` 中添加：

```kconfig
config DRM_PANEL_RAYDIUM_RM69330
    tristate "Raydium RM69330 AMOLED panel"
    depends on OF
    depends on DRM_MIPI_DSI
    help
      Say Y here if you want to enable support for Raydium RM69330
      AMOLED display panels with MIPI DSI interface.
```

在 `drivers/gpu/drm/panel/Makefile` 中添加：

```makefile
obj-$(CONFIG_DRM_PANEL_RAYDIUM_RM69330) += panel-raydium-rm69330.o
```

---

## 6. 内核配置（defconfig）

Orange Pi 5 Max 使用 `rockchip_linux_defconfig`，确保以下配置项开启：

```
CONFIG_DRM=y
CONFIG_DRM_ROCKCHIP=y
CONFIG_ROCKCHIP_VOP2=y
CONFIG_ROCKCHIP_DW_MIPI_DSI=y
CONFIG_PHY_ROCKCHIP_SAMSUNG_DCPHY=y
CONFIG_DRM_PANEL_RAYDIUM_RM69330=y
# 或编译为模块方便调试
# CONFIG_DRM_PANEL_RAYDIUM_RM69330=m
```

---

## 7. 编译与部署

### 7.1 使用 Orange Pi SDK 编译

```bash
# 方法1：使用 orangepi-build 脚本（推荐）
cd orangepi-build
sudo ./build.sh

# 选择：
# 1. Kernel package → 只编译内核
# 2. 选择 orangepi5max 板型
# 3. 选择 vendor kernel (5.10.x)

# 编译产物在 output/debs/ 目录下
# linux-image-xxx.deb — 内核 + DTB
# linux-headers-xxx.deb — 内核头文件
```

### 7.2 手动编译内核

```bash
# 进入内核源码目录
cd kernel

export ARCH=arm64
export CROSS_COMPILE=aarch64-linux-gnu-

# 使用 Orange Pi 5 Max 的 defconfig
make rockchip_linux_defconfig

# 启用 RM69330 驱动
# 方法1：make menuconfig → Device Drivers → Graphics → DRM → Panel → Raydium RM69330
# 方法2：直接追加
echo "CONFIG_DRM_PANEL_RAYDIUM_RM69330=y" >> .config
make olddefconfig

# 编译
make -j$(nproc) Image dtbs modules
```

### 7.3 部署到 Orange Pi 5 Max

```bash
# 方法1：安装 deb 包（如果用 orangepi-build）
scp output/debs/linux-image-*.deb orangepi@<IP>:/tmp/
ssh orangepi@<IP> "sudo dpkg -i /tmp/linux-image-*.deb && sudo reboot"

# 方法2：手动替换（开发调试用）
# 替换内核
scp arch/arm64/boot/Image orangepi@<IP>:/tmp/
ssh orangepi@<IP> "sudo cp /tmp/Image /boot/Image && sudo reboot"

# 替换 DTB
scp arch/arm64/boot/dts/rockchip/rk3588-orangepi-5-max.dtb orangepi@<IP>:/tmp/
ssh orangepi@<IP> "sudo cp /tmp/rk3588-orangepi-5-max.dtb /boot/dtb/rockchip/"

# 方法3：如果驱动编译为模块
scp drivers/gpu/drm/panel/panel-raydium-rm69330.ko orangepi@<IP>:/tmp/
ssh orangepi@<IP> "sudo cp /tmp/panel-raydium-rm69330.ko /lib/modules/\$(uname -r)/extra/"
ssh orangepi@<IP> "sudo depmod -a && sudo modprobe panel-raydium-rm69330"
```

### 7.4 使用 Device Tree Overlay（免重编内核）

如果不想修改主 DTS，可以使用 overlay 方式。创建 `rm69330-dsi0.dts`：

```dts
/dts-v1/;
/plugin/;

/ {
    compatible = "rockchip,rk3588-orangepi-5-max", "rockchip,rk3588";

    fragment@0 {
        target = <&mipi_dcphy0>;
        __overlay__ {
            status = "okay";
        };
    };

    fragment@1 {
        target = <&dsi0>;
        __overlay__ {
            status = "okay";
            #address-cells = <1>;
            #size-cells = <0>;
            rockchip,lane-rate = <480>;

            panel@0 {
                compatible = "raydium,rm69330";
                reg = <0>;
                reset-gpios = <&gpio2 RK_PC1 GPIO_ACTIVE_LOW>;
                vci-supply = <&vcc3v3_lcd>;
                vddi-supply = <&vcc1v8_lcd>;

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
    };

    fragment@2 {
        target = <&route_dsi0>;
        __overlay__ {
            status = "okay";
            connect = <&vp2_out_dsi0>;
        };
    };
};
```

编译并部署 overlay：

```bash
# 编译 overlay
dtc -@ -I dts -O dtb -o rm69330-dsi0.dtbo rm69330-dsi0.dts

# 部署到 Orange Pi 5 Max
scp rm69330-dsi0.dtbo orangepi@<IP>:/tmp/
ssh orangepi@<IP> "sudo cp /tmp/rm69330-dsi0.dtbo /boot/dtb/rockchip/overlay/"

# 在 /boot/orangepiEnv.txt 中添加 overlay
ssh orangepi@<IP> "echo 'overlays=rm69330-dsi0' | sudo tee -a /boot/orangepiEnv.txt"
ssh orangepi@<IP> "sudo reboot"
```

---

## 8. 调试方法

### 8.1 检查 DSI 控制器状态

```bash
# 查看 DRM connector
cat /sys/class/drm/card0-DSI-1/status
cat /sys/class/drm/card0-DSI-1/modes

# 查看所有 connector 状态
modetest -M rockchip -c

# 查看 VOP 绑定情况
cat /sys/kernel/debug/dri/0/summary
```

### 8.2 内核日志

```bash
# 查看 panel 驱动加载
dmesg | grep -i "rm69330\|mipi\|dsi\|panel\|drm"

# 常见错误及含义：
# "failed to attach"     — DSI 通信失败，检查硬件连接和 lane 数
# "timeout"              — 初始化命令超时，检查复位时序和电源
# "failed to get reset"  — GPIO 配置错误，检查 DTS 中 GPIO 编号
# "no panel found"       — compatible 不匹配或驱动未编译
```

### 8.3 GPIO 调试

```bash
# 查看所有 GPIO 状态
cat /sys/kernel/debug/gpio

# 确认 RESET 引脚被正确申请
cat /sys/kernel/debug/gpio | grep reset

# 手动控制 GPIO 测试（需要先确认 GPIO 编号）
# RK3588 GPIO 编号计算：bank * 32 + group * 8 + pin
# 例如 GPIO2_C1 = 2*32 + 2*8 + 1 = 81
echo 81 > /sys/class/gpio/export
echo out > /sys/class/gpio/gpio81/direction
echo 0 > /sys/class/gpio/gpio81/value   # 拉低 RESET
sleep 0.1
echo 1 > /sys/class/gpio/gpio81/value   # 释放 RESET
```

### 8.4 MIPI DSI 调试

```bash
# 读取屏幕 ID（验证 DSI 通信是否正常）
# 在驱动 probe 中添加：
# u8 id[3];
# mipi_dsi_dcs_read(dsi, 0x04, id, 3);
# dev_info(dev, "Panel ID: %02x %02x %02x\n", id[0], id[1], id[2]);

# 查看 DSI PHY 状态
cat /sys/kernel/debug/dri/0/summary
```

### 8.5 显示测试

```bash
# 使用 modetest 输出测试图案
modetest -M rockchip -s <connector_id>@<crtc_id>:454x454 \
    -P <plane_id>@<crtc_id>:454x454

# 使用 framebuffer 测试（简单粗暴）
cat /dev/urandom > /dev/fb0

# 使用 fbset 查看 framebuffer 信息
fbset -i -fb /dev/fb0
```

### 8.6 Orange Pi 专用工具

```bash
# 使用 orangepi-config 查看硬件状态
sudo orangepi-config
# → System → Hardware → 查看 overlay 状态

# 查看当前使用的 DTB
cat /proc/device-tree/model
# 应显示 "Orange Pi 5 Max"
```

---

## 9. 常见问题与解决

| 问题 | 可能原因 | 解决方法 |
|------|---------|---------|
| 屏幕无显示 | 电源未到位 | 用万用表量 30pin 连接器 Pin1(3.3V) 和 Pin3(1.8V) |
| 屏幕白屏 | 初始化序列错误 | 核对屏厂提供的 init code，逐条检查 |
| 屏幕闪烁 | DSI 时钟不匹配 | 调整 `rockchip,lane-rate` 值 |
| 花屏/颜色异常 | 像素格式不匹配 | 确认 RGB888 vs RGB565 配置一致 |
| 屏幕倒置/镜像 | 扫描方向错误 | 在 init 序列中设置 `0x36`（MADCTL）寄存器 |
| 只亮一半 | lane 数配置错误 | 确认 `dsi->lanes = 1` 与实际接线一致 |
| dmesg 报 timeout | 复位时序不对 | 调整 reset 脉冲宽度和延时 |
| probe 失败 | compatible 不匹配 | 检查 DTS 中 compatible 字符串与驱动一致 |
| DSI connector 显示 disconnected | DCPHY 未启用 | 确认 `&mipi_dcphy0 { status = "okay"; }` |
| 编译报错找不到头文件 | 内核版本不对 | 确认使用 vendor kernel 5.10.x，非 mainline |
| overlay 不生效 | orangepiEnv.txt 配置错误 | 检查 overlays= 行，确认 dtbo 文件存在 |

---

## 10. 上电时序参考

```
        VCI (3.3V) — Pin1,2
        ──────────────────────────────────────
                  │
        VDDI(1.8V)│ — Pin3,4
        ──────────┼──────────────────────────
                  │  ≥10ms
        RESET     │ — Pin5      ┌───┐
        ──────────┴─────────────┘   └────────
                                ≥5ms  ≥10ms
                                      │
        Init Cmd ─────────────────────┼──────
                                      │
        Sleep Out ────────────────────┼──────
                                    ≥120ms
        Display On ───────────────────┼──────
                                    ≥20ms
        正常显示 ─────────────────────┴──────
```

---

## 11. 参考资料

- [Orange Pi 5 Max 官方页面](http://www.orangepi.org/html/hardWare/computerAndMicrocontrollers/details/Orange-Pi-5-Max.html) — 原理图、用户手册、系统镜像下载
- [Orange Pi Linux 内核源码](https://github.com/orangepi-xunlong/linux-orangepi) — branch: orange-pi-5.10-rk35xx
- Rockchip RK3588 TRM（Technical Reference Manual）— Display 章节
- Rockchip DRM 开发指南：SDK 内 `docs/linux/display/`
- RM69330 Datasheet（向屏厂索取）
- Linux DRM Panel 驱动框架：`Documentation/gpu/drm-kms-helpers.rst`
- MIPI DSI 规范：MIPI Alliance DSI-2 Specification
- [cnx-software Orange Pi 5 Max 评测](https://www.cnx-software.com/2024/08/01/rockchip-rk3588-powered-orange-pi-5-max-sbc-features-up-to-16gb-lpddr5-2-5gbe-onboard-wifi-6e-and-bluetooth-5-3/)
