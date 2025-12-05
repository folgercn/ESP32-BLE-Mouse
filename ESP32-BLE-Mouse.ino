#include <WebServer.h>
#include <ArduinoJson.h>
#include <WiFi.h>

#include "NetHelper.h"
#include "BleDriver.h"
#include "AutoSwipe.h"

NetHelper net;
BleDriver ble;
WebServer server(80);
AutoSwipeManager autoSwipe;

// 引脚定义
const int PIN_BOOT = 0; // BOOT 按键 (IO0, 低电平为按下)
const int PIN_LED  = 2; // 板载蓝色 LED

// BOOT 长按检测状态
bool bootPressed = false;
unsigned long bootPressAt = 0;
bool resettingNow = false;

// 辅助函数：解析参数
// Helper: parse JSON options into ActionOptions
ActionOptions parseOptions(JsonDocument& doc) {
    ActionOptions opts;
    if (doc.containsKey("screen_w")) opts.screenW = doc["screen_w"];
    if (doc.containsKey("screen_h")) opts.screenH = doc["screen_h"];
    if (doc.containsKey("delay_hover"))    opts.delayHover = doc["delay_hover"];
    if (doc.containsKey("delay_press"))    opts.delayPress = doc["delay_press"];
    if (doc.containsKey("delay_interval")) opts.delayInterval = doc["delay_interval"];
    if (doc.containsKey("delay_release"))  opts.delayRelease = doc["delay_release"];
    if (doc.containsKey("delay_multi_click_interval")) opts.delayMultiClickInterval = doc["delay_multi_click_interval"];
    if (doc.containsKey("multi_interval")) opts.delayMultiClickInterval = doc["multi_interval"];
    if (doc.containsKey("double_check"))   opts.delayDoubleCheck = doc["double_check"];
    if (doc.containsKey("curve_strength")) opts.curveStrength = doc["curve_strength"];
    return opts;
}

void handleAction() {
    if (server.method() != HTTP_POST || server.args() == 0 || server.arg("plain") == "") {
        server.send(400, "application/json", "{\"error\":\"Body missing\"}");
        return;
    }

    String body = server.arg("plain");
    JsonDocument doc; 
    DeserializationError error = deserializeJson(doc, body);

    ble.pulseRx(80); // 有 HTTP 数据包时闪烁 RX

    if (error) {
        server.send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
        return;
    }

    if (!ble.isConnected()) {
        server.send(503, "application/json", "{\"error\":\"Bluetooth not connected\"}");
        return;
    }

    String type = doc["type"].as<String>();
    ActionOptions opts = parseOptions(doc);
    
    if (type == "click") {
        int x = doc["x"];
        int y = doc["y"];
        int count = doc.containsKey("count") ? max(1, doc["count"].as<int>()) : 1;
        ble.click(x, y, count, opts);
    } else if (type == "swipe") {
        int x1 = doc["x1"];
        int y1 = doc["y1"];
        int x2 = doc["x2"];
        int y2 = doc["y2"];
        int duration = doc["duration"]; 
        ble.swipe(x1, y1, x2, y2, duration, opts);
    } else {
        server.send(400, "application/json", "{\"error\":\"Unknown type\"}");
        return;
    }

    server.send(200, "application/json", "{\"status\":\"ok\"}");
}

void setup() {
    Serial.begin(115200);
    randomSeed(analogRead(0));

    pinMode(PIN_BOOT, INPUT_PULLUP);
    pinMode(PIN_LED, OUTPUT);
    digitalWrite(PIN_LED, LOW);

    // 1. 自动配网 (阻塞式，直到连上 WiFi 才会继续)
    // EN: Auto WiFi provisioning (blocking until WiFi is connected)
    // 第一次运行请用手机连接 "Wacom-Setup-xxxx" 热点进行配置
    // EN: On first boot, connect to the "Wacom-Setup-xxxx" AP with a phone/PC to configure WiFi
    net.autoConfig();
    
    // 2. 网络通了之后，根据 IP 生成蓝牙名
    // EN: After WiFi is up, generate BLE name based on IP
    String bleName = net.getDynamicBleName();
    ble.begin(bleName);

    // 重置 WiFi 的接口
    server.on("/reset_wifi", HTTP_GET, []() {
        ble.pulseRx(80);
        server.send(200, "text/plain", "WiFi settings cleared! Restarting...");
        delay(1000);
        WiFi.disconnect(true, true); // 清除保存的凭证
        ESP.restart(); // 重启后就会重新出现 Wacom-Setup 热点
    });
    
    // 自动上划接口注册
    autoSwipe.begin(&server, &ble);

    server.on("/action", HTTP_POST, handleAction);
    server.begin();
    Serial.println("[System] Ready. Control: http://" + net.getLocalIP() + "/action");
}

void loop() {
    server.handleClient();
    autoSwipe.tick();
    ble.tick();

    // 检测 BOOT 按键长按以恢复出厂设置
    int btn = digitalRead(PIN_BOOT);
    if (btn == LOW) {
        if (!bootPressed) {
            bootPressed = true;
            bootPressAt = millis();
        } else if (!resettingNow && millis() - bootPressAt >= 2000) {
            resettingNow = true;

            // LED 快闪 25 次
            for (int i = 0; i < 25; i++) {
                digitalWrite(PIN_LED, HIGH);
                delay(120);
                digitalWrite(PIN_LED, LOW);
                delay(120);
            }

            // 清除 BLE 配对与 WiFi 配置
            ble.resetPairing();
            WiFi.disconnect(true, true); // 断开并清空凭证
            net.resetSettings();         // 清除 WiFiManager/静态 IP

            Serial.println("Rebooting..");
            delay(200);
            ESP.restart();
        }
    } else {
        bootPressed = false;
    }
}
