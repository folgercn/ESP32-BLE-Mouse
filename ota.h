#ifndef OTA_H
#define OTA_H

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

class BleDriver;

/**
 * @class OtaUpdater
 * @brief Manages OTA firmware updates and system status LED notifications.
 * @brief 管理 OTA 固件更新和系统状态 LED 通知。
 */
class OtaUpdater {
public:
    /**
     * @brief Initializes the OtaUpdater.
     * @brief 初始化 OtaUpdater。
     * @param currentVersion The current firmware version.
     * @param currentVersion 当前固件版本号。
     */
    void begin(long long currentVersion);

    /**
     * @brief Checks for a new firmware version and performs the update if available.
     * @brief 检查新固件版本，并在可用时执行更新。
     */
    void checkAndUpdate();

    /**
     * @brief Periodic task handler, called in the main loop. Manages timed update checks and status LED.
     * @brief 周期性任务处理器，在主循环中调用。管理定时更新检查和状态 LED。
     * @param isWifiConnected The current status of the Wi-Fi connection.
     * @param isWifiConnected 当前 Wi-Fi 连接状态。
     * @param isBleConnected The current status of the BLE connection.
     * @param isBleConnected 当前 BLE 连接状态。
     */
    void tick(bool isWifiConnected, bool isBleConnected);

    /**
     * @brief Controls the LED for AP mode indication (solid yellow).
     * @brief 控制 AP 模式指示灯（常亮黄灯）。
     * @param active True to turn on the yellow light, false to turn it off.
     * @param active 为 true 时点亮黄灯，为 false 时熄灭。
     */
    void setApModeLed(bool active);

private:
    // --- Version & Timing / 版本与计时 ---
    long long _currentVersion; // EN: Current firmware version. / 中文: 当前固件版本。
    unsigned long _lastCheckMillis = 0; // EN: Timestamp of the last update check. / 中文: 上次更新检查的时间戳。
    const long _checkIntervalMillis = 12 * 60 * 60 * 1000; // EN: 12 hours between checks. / 中文: 12 小时检查间隔。

    // --- Hardware & Config / 硬件与配置 ---
    const int _ledPin = 48; // EN: GPIO pin for the WS2818 LED. / 中文: WS2818 LED 连接的 GPIO 引脚。
    const int _maxRetries = 3; // EN: Max retries for firmware download. / 中文: 固件下载的最大重试次数。
    Adafruit_NeoPixel _strip; // EN: NeoPixel library instance. / 中文: NeoPixel 库实例。
    
    // --- State Management / 状态管理 ---
    bool _isOtaInProgress = false; // EN: Flag to indicate if an OTA update is active, to prioritize LED control. / 中文: 标记 OTA 更新是否正在进行，以优先控制 LED。
    unsigned long _lastStatusBlink = 0; // EN: Timestamp for non-blocking status LED blinking. / 中文: 用于状态灯非阻塞闪烁的时间戳。
    bool _statusLedState = false; // EN: Current state of the status LED (on/off). / 中文: 当前状态灯的状态（亮/灭）。

    /**
     * @brief Sets the color of the WS2818 LED.
     * @brief 设置 WS2818 LED 的颜色。
     * @param color The color in 24-bit RGB format.
     * @param color 24位 RGB 格式的颜色值。
     */
    void setLedColor(uint32_t color);

    /**
     * @brief Turns off the WS2818 LED.
     * @brief 关闭 WS2818 LED。
     */
    void ledOff();

    /**
     * @brief Performs the actual firmware download and update process.
     * @brief 执行实际的固件下载和更新流程。
     * @param url The URL of the firmware binary.
     * @param url 固件二进制文件的 URL。
     * @param md5 The MD5 hash for firmware verification.
     * @param md5 用于固件校验的 MD5 哈希值。
     */
    void performUpdate(const String& url, const String& md5);

public:
    void setBleDriver(BleDriver* ble) { _ble = ble; }

private:
    BleDriver* _ble = nullptr;
};

#endif // OTA_H
