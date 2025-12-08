# CHANGELOG / 更新日志

## [Unreleased]
- AP 模式下黄灯常亮，便于确认热点状态 / Yellow LED stays on in AP mode for hotspot visibility.
- Wi-Fi 已连接但蓝牙未连接时，蓝灯闪烁提示正在等待 BLE 链接 / Blue LED blinks while Wi-Fi is up but BLE is not yet connected.
- OTA 阶段区分：下载青色、刷写绿色、失败红色，10% 进度日志+JSON/URL 调试输出，15s 无数据自动中断重试 / OTA LEDs now cyan (download), green (flash), red (fail) with 10% progress logs, JSON/URL debug, and 15s stall timeout.
- OTA 前自动暂停 BLE（NimBLE deinit），减少 TLS 内存占用，失败时恢复，成功则重启 / BLE auto-pauses before OTA to free RAM for TLS; resumes on failure, device restarts on success.
- 启动时打印 PSRAM 状态（是否检测到、容量、PSRAM/内部剩余），便于确认开启情况 / Boot now logs PSRAM presence/size and free PSRAM/internal heap for verification.
- OTA 下载缓冲缩小至 1KB，并保留 MD5 校验与重试逻辑 / OTA download buffer reduced to 1KB; MD5 and retry logic unchanged.

## [0.1] - 2025-12-05
- TX LED pulses on BLE traffic, RX LED pulses on HTTP/WiFi traffic; both auto-off when idle (TX/RX 低电平闪烁，空闲自动熄灭)。
- Added bilingual docs for the indicator behavior in README (README 增加通讯指示灯说明)。
- Inline bilingual comments for the new LED pulse logic (LED 脉冲逻辑补充中英文注释)。
