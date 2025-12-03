#ifndef NETHELPER_H
#define NETHELPER_H

#include <WiFi.h>
#include <WiFiManager.h>
#include <Preferences.h> // 引入存储库
// EN: NVS key-value storage helper

class NetHelper {
public:
    void autoConfig(); 
    
    String getDynamicBleName(); 
    String getLocalIP();
    
    // 清除 WiFi 同时也清除固定 IP 设置
    // EN: Clear WiFi credentials and stored static IP config
    void resetSettings(); 

private:
    Preferences pref; // 用于读写闪存
    // EN: Preferences instance for reading/writing flash (NVS)
    
    // 辅助函数：保存/加载自定义 IP
    // EN: Helper functions: save/load custom static IP
    void saveConfig(String ip, String gw, String sn);
    bool loadConfig(char* ip, char* gw, char* sn);
};

#endif