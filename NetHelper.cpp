#include "NetHelper.h"

// 标志位：告诉我们是否需要保存参数
bool shouldSaveConfig = false;

// 回调函数：当用户点击网页上的 Save 时触发
void saveConfigCallback () {
  Serial.println("[WiFi] User clicked Save. Configuration needs update.");
  shouldSaveConfig = true;
}

void NetHelper::autoConfig() {
    WiFiManager wm;
    
    // 1. 从 NVS (闪存) 加载之前的固定 IP 设置
    char static_ip[16] = "";
    char static_gw[16] = "";
    char static_sn[16] = "";
    
    // 读取配置，如果存在返回 true
    bool hasStatic = loadConfig(static_ip, static_gw, static_sn);

    // 2. 如果之前存过固定 IP，告诉 WiFiManager 使用它
    if (hasStatic) {
        IPAddress _ip, _gw, _sn;
        if (_ip.fromString(static_ip) && _gw.fromString(static_gw) && _sn.fromString(static_sn)) {
            Serial.println("[WiFi] Using Static IP: " + String(static_ip));
            wm.setSTAStaticIPConfig(_ip, _gw, _sn);
        }
    }

    // 3. 定义网页上的自定义输入框
    // 参数：id, label, default value, length
    WiFiManagerParameter custom_ip("ip", "Static IP (Optional)", static_ip, 15);
    WiFiManagerParameter custom_gw("gateway", "Gateway", static_gw, 15);
    WiFiManagerParameter custom_sn("subnet", "Subnet", static_sn, 15);

    // 添加到配网页面
    wm.addParameter(&custom_ip);
    wm.addParameter(&custom_gw);
    wm.addParameter(&custom_sn);

    // 设置回调
    wm.setSaveParamsCallback(saveConfigCallback);

    // 设置热点名和超时
    String apName = "Wacom-Setup-" + String((uint32_t)ESP.getEfuseMac(), HEX);
    wm.setConfigPortalTimeout(180); // 3分钟超时

    Serial.println("[WiFi] Auto Connecting...");
    
    // 4. 开始连接 (如果失败会建立热点)
    if(!wm.autoConnect(apName.c_str())) {
        Serial.println("[WiFi] Failed to connect. Restarting...");
        delay(3000);
        ESP.restart();
    }

    // 5. 连接成功后，检查是否需要保存新填写的 IP
    if (shouldSaveConfig) {
        String newIP = custom_ip.getValue();
        String newGW = custom_gw.getValue();
        String newSN = custom_sn.getValue();
        
        // 只有当 IP 不为空时才保存
        if (newIP.length() > 0) {
            Serial.println("[WiFi] Saving new Static IP: " + newIP);
            saveConfig(newIP, newGW, newSN);
        } else {
            // 如果用户清空了输入框，表示想切回 DHCP
            // 这里可以做一个逻辑清除，视需求而定，目前暂不清除
        }
    }

    Serial.println("\n[WiFi] Connected!");
    Serial.println("[WiFi] IP: " + WiFi.localIP().toString());
}

String NetHelper::getLocalIP() {
    return WiFi.localIP().toString();
}

// 辅助函数：保存配置到 NVS
void NetHelper::saveConfig(String ip, String gw, String sn) {
    pref.begin("net_config", false); // 命名空间 net_config, 读写模式
    pref.putString("ip", ip);
    pref.putString("gw", gw);
    pref.putString("sn", sn);
    pref.end();
}

// 辅助函数：加载配置
bool NetHelper::loadConfig(char* ip, char* gw, char* sn) {
    pref.begin("net_config", true); // 只读模式
    String s_ip = pref.getString("ip", "");
    String s_gw = pref.getString("gw", "");
    String s_sn = pref.getString("sn", "");
    pref.end();

    if (s_ip.length() > 0) {
        strcpy(ip, s_ip.c_str());
        strcpy(gw, s_gw.c_str());
        strcpy(sn, s_sn.c_str());
        return true; // 找到了配置
    }
    
    // 没找到配置，设置默认子网掩码方便用户填
    strcpy(sn, "255.255.255.0"); 
    return false;
}

// 重置所有设置 (WiFi + Static IP)
void NetHelper::resetSettings() {
    WiFiManager wm;
    wm.resetSettings(); // 清除 WiFi 密码
    
    pref.begin("net_config", false);
    pref.clear(); // 清除存储的 IP 设置
    pref.end();
    
    Serial.println("[WiFi] All settings erased (WiFi + Static IP)!");
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