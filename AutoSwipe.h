#ifndef AUTOSWIPE_H
#define AUTOSWIPE_H

// AutoSwipe: manage auto swipe configuration, persistence, HTTP UI, and scheduling.
// Handles: load/save config, HTML/JSON endpoints, and periodic swipe execution.
#include <WebServer.h>
#include <Preferences.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <math.h>

#include "BleDriver.h"

// 自动上划配置 / Auto-swipe configuration (defaults act as fallbacks)
struct AutoSwipeConfig {
    bool enabled = true;          // 开机自动上划 / auto start when WiFi+BLE ready
    int x1 = 540;                 // 起点 X / start X
    int y1 = 1248;                // 起点 Y / start Y
    int x2 = 540;                 // 终点 X / end X (定义矩形)
    int y2 = 1000;                // 终点 Y / end Y (定义矩形)
    int duration = 250;           // 基准时长(ms) / base swipe duration
    int lengthPercent = 80;       // 相对矩形高的长度比例 / path length vs box height (%)
    int lengthJitterPercent = 15; // 长度波动 / length jitter (%)
    int durationJitterPercent = 20; // 时长波动 / duration jitter (%)
    int delayJitterPercent = 15;    // 延迟与曲率波动 / delay & curve jitter (%)
    int screenW = 1080;           // 屏幕宽 / screen width
    int screenH = 2250;           // 屏幕高 / screen height
    int delayHover = 30;          // 悬停延迟 / hover delay
    int delayPress = 30;          // 按下延迟 / press delay
    int delayInterval = 10;       // 步进间隔 / interval between points
    int curveStrength = 20;       // 贝塞尔弯曲度 / curve strength
    int doubleCheck = 20;         // 二次抬起延迟 / double-release delay
    int intervalMinSec = 5;       // 上划间隔最小秒 / min interval (s)
    int intervalMaxSec = 45;      // 上划间隔最大秒 / max interval (s)
};

class AutoSwipeManager {
public:
    void begin(WebServer* srv, BleDriver* bleDriver);
    void tick();

private:
    Preferences pref;
    WebServer* server = nullptr;
    BleDriver* ble = nullptr;
    AutoSwipeConfig cfg;
    unsigned long nextSwipeAt = 0;
    bool swipeInFlight = false;

    // 工具
    int clampInt(int val, int minVal, int maxVal);
    int randomAround(int base, int spread);
    void normalizeConfig();
    void applyJsonToConfig(JsonDocument& doc, AutoSwipeConfig& c);
    void loadConfig();
    void saveConfig(const AutoSwipeConfig& c);

    // HTTP
    String renderPage(const AutoSwipeConfig& c, const String& message = "");
    void handleGet();
    void handlePost();
    void handleStatus();

    // 业务
    unsigned long randomIntervalMs();
    void scheduleNext();
    void performSwipe();
};

#endif
