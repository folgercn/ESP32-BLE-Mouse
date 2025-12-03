#ifndef NETHELPER_H
#define NETHELPER_H

#include <WiFi.h>
#include <WiFiManager.h>
#include <Preferences.h> // 引入存储库

class NetHelper {
public:
    void autoConfig(); 
    
    String getDynamicBleName(); 
    String getLocalIP();
    
    // 清除 WiFi 同时也清除固定 IP 设置
    void resetSettings(); 

private:
    Preferences pref; // 用于读写闪存
    
    // 辅助函数：保存/加载自定义 IP
    void saveConfig(String ip, String gw, String sn);
    bool loadConfig(char* ip, char* gw, char* sn);
};

#endif