# ESP32 BLE Mouse Gateway

[English README / 英文版说明请点击这里](#english-overview)

## 项目简介
ESP32 安卓自动化控制网关以 **Wacom Digitizer** 触控笔身份伪装，通过 BLE HID 与 Android 原生兼容；同时提供 HTTP+JSON 控制接口，所有动作参数由服务器下发，实现完全「无固件改动」的批量控制能力。WiFi 通过 WiFiManager 热点完成一次性配置，掉电即可自动重连。

## 项目亮点
- **设备伪装**：BLE HID 描述符模拟 Wacom 数位笔，系统原生识别；广播名称 `Wacom-{MAC后4位}-{IP后3位}`，便于群控平台快速定位。
- **拟人化动作**：二阶贝塞尔曲线轨迹 + 随机方向偏移，叠加坐标抖动；点击采用 Double-Release，避免长按卡死或断触。
- **稳定连接**：直接查询 NimBLE 协议栈连接数判断在线状态；自定义广播包解决部分机型搜不到设备的问题。
- **JSON 驱动**：屏幕分辨率、速度、延迟、曲率等均由上位机 JSON 配置，固件无需重新编译。
- **工程化网络**：WiFiManager 自动配网 + 自定义静态 IP 表单；支持 `/reset_wifi` 接口一键恢复热点模式。
- **可靠重连**：默认保留历史配对，设备重启后已配对手机会自动重连；需要清除配对时再手动触发。
- **一键恢复**：长按板载 BOOT 键 2 秒，LED 快闪 5 次后清除 BLE 配对和 WiFi 配置并重启；`/auto_swipe` 页面也有“重置蓝牙配对”按钮。
- **随机点赞**：自动上划间隔内可按概率随机触发双击点赞，概率/间隔/缓冲均支持波动。

## 系统结构
- `ESP32-BLE-Mouse.ino`：HTTP 服务、JSON 动作解析、全局生命周期。
- `BleDriver.*`：基于 NimBLE 的 Wacom HID 实现，负责拟人化移动与点击算法。
- `NetHelper.*`：WiFiManager 配网、静态 IP 存储、动态生成蓝牙广播名。

## 快速开始
1. **硬件**：ESP32-DevKitC / ESP32-WROOM 模组，5V 供电。
2. **开发环境**：Arduino IDE 或 PlatformIO，需安装：
   - `NimBLE-Arduino`
   - `ArduinoJson`
   - `WiFiManager`
3. **烧录**：将整个目录导入 IDE，选择对应的 ESP32 板卡与串口后上传。
4. **首次配置**：
   - 设备会创建热点 `Wacom-Setup-XXXX`，用手机/PC 连接。
   - 大多数手机会自动弹出配网 Portal 页面，在页面里填写 WiFi SSID/密码及可选静态 IP（IP/Gateway/Subnet 均可写）。
   - 如果没有自动弹出 Portal，再手动访问 `192.168.4.1` 进行同样的配置。
5. **工作模式**：
   - 连接到 WiFi 后自动根据 MAC/IP 生成蓝牙名称并开始广播。
   - HTTP 服务器监听 `http://<设备IP>/action`。
6. **恢复热点**：访问 `http://<设备IP>/reset_wifi`，设备会清除凭证并重启。

## HTTP/JSON 控制接口
- **Endpoint**：`POST /action`
- **Headers**：`Content-Type: application/json`
- **公共字段**：
  - `type`: `"click"` 或 `"swipe"`
  - `screen_w` / `screen_h`: 设备屏幕像素（默认 1080x2248）
  - `delay_hover` / `delay_press` / `delay_interval` / `delay_release` / `double_check`: 毫秒延迟
  - `curve_strength`: 0-100，决定贝塞尔弯曲程度
- **click 专属**：`x`, `y`，可选 `count`（默认 1，>1 变为连点）、`multi_interval`（连点间隔 ms，默认 30）
- **swipe 专属**：`x1`, `y1`, `x2`, `y2`, `duration`

点击示例：

```
POST http://192.168.1.23/action
{
  "type": "click",
  "x": 900,
  "y": 1100,
  "count": 2,
  "multi_interval": 40,
  "screen_w": 1080,
  "screen_h": 2248,
  "delay_hover": 50,
  "delay_press": 80,
  "double_check": 20
}
```

滑动示例：

```
POST http://192.168.1.23/action
{
  "type": "swipe",
  "x1": 200,
  "y1": 1800,
  "x2": 200,
  "y2": 400,
  "duration": 600,
  "screen_w": 1240,
  "screen_h": 2680,
  "delay_hover": 30,
  "delay_press": 40,
  "delay_interval": 12,
  "curve_strength": 25,
  "double_check": 40
}
```

## 自动上划 / Auto Swipe
- 页面 / Page：WiFi + 蓝牙连接后访问 `http://<设备IP>/auto_swipe`，中英双语表单；保存立即生效并写入闪存。
- 默认 / Defaults：`enabled=true`，`interval_min_sec=5`，`interval_max_sec=45`，`duration=250`，`length_percent=80`，`length_jitter_percent=15`，`duration_jitter_percent=20`，`delay_jitter_percent=15`，`double_tap_enabled=true`，`double_tap_prob_percent=30`，`double_tap_prob_jitter_percent=15`，`double_tap_interval_ms=120`，`double_tap_interval_jitter_percent=15`，`double_tap_edge_min_ms=250`，`double_tap_edge_max_ms=800`。
- 行为 / Behavior：开启后且 WiFi+BLE 均在线时，在 `x1,y1` 到 `x2,y2` 的矩形内随机起止点向上滑动；间隔在最小/最大秒数之间随机，时长按 `duration_jitter_percent` 浮动，长度按 `length_percent` 与 `length_jitter_percent` 缩放并抖动。
- 点赞 / Double Tap：`double_tap_enabled` 控制是否在两次上划间隔内随机双击（默认开启）。开启时，根据概率（含 `double_tap_prob_jitter_percent` 波动）决定是否点赞；双击间隔取自 `double_tap_interval_ms` 并按 `double_tap_interval_jitter_percent` 波动。点赞时间随机靠近“上次滑动结束”或“下次滑动开始”两段安全缓冲内，避免与滑动太贴边；坐标落在滑动矩形中心附近并抖动。
- API：`POST /auto_swipe` 支持 JSON 配置，键仅英文：`enabled`、`x1`/`y1`/`x2`/`y2`、`duration`、`screen_w`/`screen_h`、`delay_hover`/`delay_press`/`delay_interval`、`curve_strength`、`double_check`、`interval_min_sec`/`interval_max_sec`、`length_percent`、`length_jitter_percent`、`duration_jitter_percent`、`delay_jitter_percent`、`double_tap_enabled`、`double_tap_prob_percent`、`double_tap_prob_jitter_percent`、`double_tap_interval_ms`、`double_tap_interval_jitter_percent`、`double_tap_edge_min_ms`、`double_tap_edge_max_ms`。状态接口 `GET /auto_swipe/status` 返回当前配置与剩余计时。
- 功能现状 / Status：自动上划、随机路径/时长/间隔、间隔内随机点赞、JSON/表单配置及状态接口均可用，配置与状态字段仅用英文键。

## JSON 参数说明
| 字段 | 作用 |
| --- | --- |
| `screen_w / screen_h` | 上位机屏幕像素，内部映射为 0-32767 |
| `delay_hover` | 移动完成后按下前的悬停时间 |
| `delay_press` | 按下到开始滑动的停顿 |
| `delay_interval` | 滑动步进间隔，数值越小说明越平滑 |
| `delay_release` | 抬起后的冷却期 |
| `double_check` | Double-Release 延迟，避免系统误判 |
| `curve_strength` | 贝塞尔轨迹弯曲度，百分比 |

## 商用品质特性
1. **身份伪装**：Wacom HID 描述 + 高外观 ID，兼容 Android/大多数主机。
2. **抗风控**：贝塞尔+随机偏移+抖动，参考人手轨迹；点击多次抬起防止卡键。
3. **广播增强**：自构造广播包，确保群控软件获取到自定义名称。
4. **连接健康检测**：直接查询 `NimBLEDevice::getServer()->getConnectedCount()`，不依赖变量状态。
5. **服务器驱动**：固件只负责执行 JSON，策略全部由云端控制。
6. **可维护性**：所有延迟/曲率都能远程调参，无需 OTA。

---

## English Overview
ESP32 BLE Mouse Gateway impersonates a **Wacom Digitizer** so Android accepts it natively. A lightweight HTTP+JSON bridge feeds every motion parameter from the server, while WiFiManager handles provisioning via a temporary AP. Once WiFi is online, the device rebrands itself as `Wacom-{MAC4}-{IP3}` and starts NimBLE HID advertising.

### Highlights
- **Device Masquerade**: HID descriptor mirrors Wacom stylus; custom BLE name embeds MAC/IP for fleet management.
- **Human-like Motions**: Quadratic Bézier curves with random offsets and jitter; Double-Release prevents stuck touches.
- **Link Reliability**: Connection state reads directly from the NimBLE stack; handcrafted advertising fixes discoverability gaps.
- **JSON-Driven Logic**: Resolution, delays, curve strength and motion speed all controlled by server JSON—no firmware rebuild.
- **Operational Networking**: WiFiManager captive portal with optional static IP form; `GET /reset_wifi` clears credentials remotely.
- **Persisted Pairing**: Bonds are kept across reboots so phones auto-reconnect; clear bonds only via BOOT long-press or the Auto Swipe page button.
- **One-Key Recovery**: Hold BOOT (GPIO0) ~2s → LED (GPIO2) flashes 5x, then clears BLE bonds + WiFi creds and reboots. `/auto_swipe` also exposes a “Reset BLE Pairing” button.

### Architecture
- `ESP32-BLE-Mouse.ino`: Hosts HTTP server, parses JSON, manages lifecycle.
- `BleDriver.*`: Implements Wacom-style HID reports and motion algorithms.
- `NetHelper.*`: Wraps WiFiManager auto-provisioning, static IP storage, and BLE name generation.

### Quick Start
1. Hardware: ESP32-DevKitC / ESP32-WROOM, USB or 5 V supply.
2. Toolchain: Arduino IDE or PlatformIO with `NimBLE-Arduino`, `ArduinoJson`, `WiFiManager`.
3. Flash: open the folder, select the proper board/port, upload.
4. First boot: device spawns AP `Wacom-Setup-XXXX`; connect, most phones will pop up the captive portal automatically—fill in WiFi and optional static IP there, or manually visit `192.168.4.1` if no portal appears.
5. Run mode: after WiFi joins, BLE advertises with the dynamic name and HTTP server listens on `http://<device-ip>/action`.
6. Reset WiFi: call `http://<device-ip>/reset_wifi` to erase credentials and reboot into setup AP.

### API Recap
```
POST /action
Headers: Content-Type: application/json
type: "click" | "swipe"
click -> x, y, optional count (default 1; >1 = multi-click), optional multi_interval (gap between clicks, ms, default 30)
swipe -> x1, y1, x2, y2, duration
common -> screen_w, screen_h, delay_hover, delay_press, delay_interval,
          delay_release, double_check, curve_strength
```

#### Click Example
```
POST http://192.168.1.23/action
{
  "type": "click",
  "x": 900,
  "y": 1100,
  "screen_w": 1080,
  "screen_h": 2248,
  "delay_hover": 50,
  "delay_press": 80,
  "double_check": 20
}
```

#### Swipe Example
```
POST http://192.168.1.23/action
{
  "type": "swipe",
  "x1": 200,
  "y1": 1800,
  "x2": 200,
  "y2": 400,
  "duration": 600,
  "screen_w": 1240,
  "screen_h": 2680,
  "delay_hover": 30,
  "delay_press": 40,
  "delay_interval": 12,
  "curve_strength": 25,
  "double_check": 40
}
```

### Auto Swipe
- **Page**: After WiFi + BLE connection, visit `http://<device-ip>/auto_swipe` for a bilingual (CN/EN) configuration form; changes take effect immediately and are saved to flash.
- **Defaults**: `enabled=true`, `interval_min_sec=5`, `interval_max_sec=45`, `duration=250`, `length_percent=80`, `length_jitter_percent=15`, `duration_jitter_percent=20`, `delay_jitter_percent=15`, `double_tap_enabled=true`, `double_tap_prob_percent=30`, `double_tap_prob_jitter_percent=15`, `double_tap_interval_ms=120`, `double_tap_interval_jitter_percent=15`.
- **Behavior**: When enabled and both WiFi+BLE are online, performs random upward swipes within the rectangle defined by `x1,y1` to `x2,y2`; interval randomized between min/max seconds, duration fluctuates by `duration_jitter_percent`, length scaled by `length_percent` and jittered by `length_jitter_percent`.
- **Double Tap**: `double_tap_enabled` controls whether to randomly double-tap during the interval between two swipes (default: enabled). When enabled, triggers double-tap likes at random moments within the "interval before next swipe" based on probability; probability fluctuates by `double_tap_prob_percent` and `double_tap_prob_jitter_percent`, double-tap interval taken from `double_tap_interval_ms` and fluctuated by `double_tap_interval_jitter_percent`, calls `click count=2`.
- **API**: `POST /auto_swipe` accepts JSON config with English keys only: `enabled`, `x1`/`y1`/`x2`/`y2`, `duration`, `screen_w`/`screen_h`, `delay_hover`/`delay_press`/`delay_interval`, `curve_strength`, `double_check`, `interval_min_sec`/`interval_max_sec`, `length_percent`, `length_jitter_percent`, `duration_jitter_percent`, `delay_jitter_percent`, `double_tap_enabled`, `double_tap_prob_percent`, `double_tap_prob_jitter_percent`, `double_tap_interval_ms`, `double_tap_interval_jitter_percent`. Status endpoint `GET /auto_swipe/status` returns current config and remaining timer.
- **Status**: Auto swipe, random path/duration/interval, random likes during intervals, JSON/form config and status endpoints are all available; config and status fields use English keys only.

### JSON Parameter Reference
| Field | Purpose |
| --- | --- |
| `screen_w / screen_h` | Target screen pixels, internally mapped to 0-32767 |
| `delay_hover` | Hover time after movement completes, before press |
| `delay_press` | Pause from press to start of swipe |
| `delay_interval` | Step interval during swipe; smaller values mean smoother motion |
| `delay_release` | Cooldown period after release |
| `double_check` | Double-Release delay to avoid system misjudgment |
| `curve_strength` | Bézier curve bending degree, percentage |

### Commercial-Ready Traits
1. Wacom HID identity with Android-native compatibility.
2. Anti-detection motion algorithm (Bézier + randomness + jitter).
3. Forced advertising payload for better discoverability.
4. Connection monitoring via `NimBLEDevice::getServer()->getConnectedCount()`.
5. Fully server-driven behavior—firmware only executes JSON commands.
6. Remote tuning of every timing/curvature parameter, no OTA required.

With these capabilities, the gateway already satisfies commercial prototype readiness for Android automation and large-scale control rooms.
