# CHANGELOG / 更新日志

## [Unreleased]
- AP 模式下黄灯常亮，便于确认热点状态 / Yellow LED stays on in AP mode for hotspot visibility.
- Wi-Fi 已连接但蓝牙未连接时，蓝灯闪烁提示正在等待 BLE 链接 / Blue LED blinks while Wi-Fi is up but BLE is not yet connected.
- OTA 行为保持不变：获取阶段蓝色，写入阶段绿色，失败时红色 / OTA indicators unchanged: blue while fetching, green while updating, red on failure.
- 新增 DEBUG_LOG_ENABLED 编译开关，所有 Serial 输出可一键启用/关闭以减少发布版开销 / Added DEBUG_LOG_ENABLED compile flag so all Serial logs can be toggled off for production builds.

## [0.1] - 2025-12-05
- TX LED pulses on BLE traffic, RX LED pulses on HTTP/WiFi traffic; both auto-off when idle (TX/RX 低电平闪烁，空闲自动熄灭)。
- Added bilingual docs for the indicator behavior in README (README 增加通讯指示灯说明)。
- Inline bilingual comments for the new LED pulse logic (LED 脉冲逻辑补充中英文注释)。
