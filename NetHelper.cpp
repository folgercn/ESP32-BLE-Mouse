#include "Config.h"
#include "NetHelper.h"
#include "ota.h" // EN: Include OtaUpdater header here for its definition. / 中文: 在这里引入 OtaUpdater 头文件以获取其定义。

// EN: Flag to indicate whether WiFiManager parameters should be saved.
// 中文: 标志位，用于指示是否需要保存 WiFiManager 的参数。
bool shouldSaveConfig = false;

// EN: Callback invoked when the user clicks "Save" on the WiFiManager portal.
// 中文: 当用户在 WiFiManager 门户上点击“保存”时触发的回调函数。
void saveConfigCallback () {
  DEBUG_PRINTLN("[WiFi] User clicked Save. Configuration needs update.");
  shouldSaveConfig = true;
}

// EN: Link the OtaUpdater instance to this helper.
// 中文: 将 OtaUpdater 实例关联到此助手。
void NetHelper::setOtaUpdater(OtaUpdater* updater) {
    _otaUpdater = updater;
}

void NetHelper::autoConfig() {
    WiFiManager wm;
    
    // EN: Set up the callback that triggers when the device enters AP mode.
    // 中文: 设置当设备进入 AP 模式时触发的回调函数。
    wm.setAPCallback([this](WiFiManager* myWiFiManager) {
        if (_otaUpdater) {
            // EN: Turn on solid yellow LED to indicate AP mode.
            // 中文: 点亮常亮黄灯以指示 AP 模式。
            DEBUG_PRINTLN("[WiFi] Entered AP mode, turning on yellow LED.");
            _otaUpdater->setApModeLed(true);
        }
        // EN: We don't need to process WiFiManager events here in a loop,
        // EN: as the yellow light is solid and not blinking.
        // 中文: 由于黄灯是常亮的而不是闪烁的，我们不需要在这里循环处理 WiFiManager 事件。
    });

    // EN: Step 1: Load previously stored static IP config from NVS.
    // 中文: 第1步: 从 NVS (闪存) 加载之前存储的静态 IP 配置。
    char static_ip[16] = "";
    char static_gw[16] = "";
    char static_sn[16] = "";
    bool hasStatic = loadConfig(static_ip, static_gw, static_sn);

    // EN: Step 2: If static IP is stored, pass it to WiFiManager.
    // 中文: 第2步: 如果存储了静态 IP，则将其传递给 WiFiManager。
    if (hasStatic) {
        IPAddress _ip, _gw, _sn;
        if (_ip.fromString(static_ip) && _gw.fromString(static_gw) && _sn.fromString(static_sn)) {
            DEBUG_PRINTLN("[WiFi] Using Static IP: " + String(static_ip));
            wm.setSTAStaticIPConfig(_ip, _gw, _sn);
        }
    }

    // EN: Step 3: Define custom form fields in the WiFiManager portal.
    // 中文: 第3步: 在 WiFiManager 门户中定义自定义表单字段。
    WiFiManagerParameter custom_ip("ip", "Static IP (Optional)", static_ip, 15);
    WiFiManagerParameter custom_gw("gateway", "Gateway", static_gw, 15);
    WiFiManagerParameter custom_sn("subnet", "Subnet", static_sn, 15);
    wm.addParameter(&custom_ip);
    wm.addParameter(&custom_gw);
    wm.addParameter(&custom_sn);

    // EN: Register callback for when parameters are saved.
    // 中文: 注册保存参数时的回调。
    wm.setSaveParamsCallback(saveConfigCallback);

    // EN: Configure AP name and portal timeout.
    // 中文: 配置 AP 名称和门户超时时间。
    String apName = "Wacom-Setup-" + String((uint32_t)ESP.getEfuseMac(), HEX);
    wm.setConfigPortalTimeout(180); // 3-minute timeout

    DEBUG_PRINTLN("[WiFi] Auto Connecting...");
    
    // EN: Step 4: Try to auto connect (or start AP on failure, which triggers the callback).
    // 中文: 第4步: 尝试自动连接 (如果失败则启动 AP，这将触发回调)。
    if(!wm.autoConnect(apName.c_str())) {
        DEBUG_PRINTLN("[WiFi] Failed to connect and hit timeout. Restarting...");
        delay(3000);
        ESP.restart();
    }

    // EN: Step 5: After connection, check if a new static IP should be saved.
    // 中文: 第5步: 连接成功后，检查是否需要保存新填写的静态 IP。
    if (shouldSaveConfig) {
        String newIP = custom_ip.getValue();
        String newGW = custom_gw.getValue();
        String newSN = custom_sn.getValue();
        
        if (newIP.length() > 0) {
            DEBUG_PRINTLN("[WiFi] Saving new Static IP: " + newIP);
            saveConfig(newIP, newGW, newSN);
        }
    }

    DEBUG_PRINTLN("\n[WiFi] Connected!");
    DEBUG_PRINTLN("[WiFi] IP: " + WiFi.localIP().toString());
}

String NetHelper::getLocalIP() {
    return WiFi.localIP().toString();
}

// EN: Helper: save static IP config into NVS.
// 中文: 辅助函数：将静态 IP 配置保存到 NVS。
void NetHelper::saveConfig(String ip, String gw, String sn) {
    pref.begin("net_config", false);
    pref.putString("ip", ip);
    pref.putString("gw", gw);
    pref.putString("sn", sn);
    pref.end();
}

// EN: Helper: load static IP config from NVS.
// 中文: 辅助函数：从 NVS 加载静态 IP 配置。
bool NetHelper::loadConfig(char* ip, char* gw, char* sn) {
    pref.begin("net_config", true);
    String s_ip = pref.getString("ip", "");
    String s_gw = pref.getString("gw", "");
    String s_sn = pref.getString("sn", "");
    pref.end();

    if (s_ip.length() > 0) {
        strcpy(ip, s_ip.c_str());
        strcpy(gw, s_gw.c_str());
        strcpy(sn, s_sn.c_str());
        return true;
    }
    
    strcpy(sn, "255.255.255.0"); 
    return false;
}

// EN: Reset all settings (WiFi credentials + static IP).
// 中文: 重置所有设置 (WiFi 凭据 + 静态 IP)。
void NetHelper::resetSettings() {
    WiFiManager wm;
    wm.resetSettings();
    
    pref.begin("net_config", false);
    pref.clear();
    pref.end();
    
    DEBUG_PRINTLN("[WiFi] All settings erased (WiFi + Static IP)!");
}

String NetHelper::getDynamicBleName() {
    String mac = WiFi.macAddress(); 
    mac.replace(":", ""); 
    String macSuffix = mac.substring(mac.length() - 4);
    
    IPAddress ip = WiFi.localIP();
    int lastOctet = ip[3];
    String ipSuffix = String(lastOctet);
    if (ipSuffix.length() == 1) ipSuffix = "00" + ipSuffix;
    else if (ipSuffix.length() == 2) ipSuffix = "0" + ipSuffix;

    String name = "Wacom-" + macSuffix + "-" + ipSuffix;
    return name;
}
