#ifndef NETHELPER_H
#define NETHELPER_H

#include <WiFi.h>
#include <WiFiManager.h>
#include <Preferences.h>
#include <WiFiUdp.h>

class OtaUpdater; // EN: Forward declaration for OtaUpdater. / 中文: OtaUpdater 的前向声明。

/**
 * @class NetHelper
 * @brief Manages Wi-Fi connection, auto-provisioning, and dynamic BLE name generation.
 * @brief 管理 Wi-Fi 连接、自动配网及动态蓝牙名称生成。
 */
class NetHelper {
public:
    /**
     * @brief Links the OtaUpdater instance to this helper for status LED control.
     * @brief 将 OtaUpdater 实例关联到此助手，用于状态 LED 控制。
     * @param updater Pointer to the global OtaUpdater instance.
     * @param updater 指向全局 OtaUpdater 实例的指针。
     */
    void setOtaUpdater(OtaUpdater* updater);

    /**
     * @brief Starts the auto-configuration process using WiFiManager.
     * @brief 使用 WiFiManager 启动自动配网流程。
     */
    void autoConfig(); 
    
    /**
     * @brief Generates a dynamic BLE name based on MAC address and IP address.
     * @brief 根据 MAC 地址和 IP 地址生成动态蓝牙名称。
     * @return A string for the BLE device name。
     * @return 用于蓝牙设备名称的字符串。
     */
    String getDynamicBleName(); 
    
    /**
     * @brief Gets the local IP address of the device.
     * @brief 获取设备的本地 IP 地址。
     * @return A string containing the local IP address.
     * @return 包含本地 IP 地址的字符串。
     */
    String getLocalIP();
    
    /**
     * @brief Resets all Wi-Fi and custom static IP settings.
     * @brief 重置所有 Wi-Fi 和自定义的静态 IP 设置。
     */
    void resetSettings(); 

    /**
     * @brief Starts a UDP listener that answers discovery broadcasts.
     * @brief 启动 UDP 监听以响应发现广播。
     * @param port Listening port for discovery packets.
     * @param port 用于发现报文的监听端口。
     * @param magic Expected probe payload; only matching packets get a response.
     * @param magic 期望的探测报文内容，只有匹配时才会回应。
     * @param version Firmware version string placed in responses.
     * @param version 用于回应的固件版本号字符串。
     */
    void beginDiscoveryResponder(uint16_t port, const String& magic, const String& version);

    /**
     * @brief Handles incoming discovery packets; call this in loop().
     * @brief 处理收到的发现报文；在 loop() 中调用。
     */
    void tickDiscovery();

private:
    // EN: Instance for reading/writing flash (NVS).
    // 中文: 用于读写闪存 (NVS) 的实例。
    Preferences pref;
    
    // EN: Pointer to the OtaUpdater to control the LED during AP mode.
    // 中文: 指向 OtaUpdater 的指针，用于在 AP 模式下控制 LED。
    OtaUpdater* _otaUpdater = nullptr;
    
    /**
     * @brief Saves custom static IP configuration to NVS.
     * @brief 将自定义静态 IP 配置保存到 NVS。
     */
    void saveConfig(String ip, String gw, String sn);

    /**
     * @brief Loads custom static IP configuration from NVS.
     * @brief 从 NVS 加载自定义静态 IP 配置。
     * @return True if a configuration was successfully loaded, false otherwise.
     * @return 如果成功加载配置则返回 true，否则返回 false。
     */
    bool loadConfig(char* ip, char* gw, char* sn);

    // --- UDP discovery / UDP 发现 ---
    WiFiUDP _udp;
    bool _udpActive = false;
    uint16_t _discoveryPort = 0;
    String _discoveryMagic;
    String _discoveryVersion;
};

#endif
