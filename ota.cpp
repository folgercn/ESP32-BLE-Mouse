#include "ota.h"
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Update.h>
#include <stdlib.h> // For malloc and free

// --- Constants / 常量 ---
// EN: NeoPixel LED parameters.
// 中文: NeoPixel 灯珠参数。
#define PIXEL_PIN    48
#define PIXEL_COUNT  1

// EN: URL for the OTA update JSON file.
// 中文: 用于 OTA 更新的 JSON 文件 URL。
const char* OTA_JSON_URL = "https://datav-d-gzcom.oss-cn-hangzhou.aliyuncs.com/esp32/s3/otaup.json";

// --- LED Helper Functions / LED 辅助函数 ---

void OtaUpdater::setLedColor(uint32_t color) {
    _strip.setPixelColor(0, color);
    _strip.show();
}

void OtaUpdater::ledOff() {
    _strip.clear();
    _strip.show();
}

// --- Public Methods / 公有方法 ---

void OtaUpdater::begin(long long currentVersion) {
    _currentVersion = currentVersion;
    // EN: Initialize the NeoPixel strip.
    // 中文: 初始化 NeoPixel 灯条。
    _strip = Adafruit_NeoPixel(PIXEL_COUNT, PIXEL_PIN, NEO_GRB + NEO_KHZ800);
    _strip.begin();
    ledOff();
}

void OtaUpdater::tick(bool isWifiConnected, bool isBleConnected) {
    // --- 1. Handle timed OTA polling / 处理 OTA 定时轮询 ---
    if (_lastCheckMillis == 0) {
        // EN: Initialize timer on first run.
        // 中文: 首次运行时初始化计时器。
        _lastCheckMillis = millis();
    }
    if (millis() - _lastCheckMillis > _checkIntervalMillis) {
        checkAndUpdate();
        _lastCheckMillis = millis();
    }

    // --- 2. Handle status LED logic / 处理状态灯逻辑 ---
    // EN: If an OTA update is in progress, it has full control over the LED.
    // 中文: 如果 OTA 更新正在进行，它将完全控制 LED，tick 不再干预。
    if (_isOtaInProgress) {
        return;
    }

    // EN: If both Wi-Fi and BLE are connected, the device is fully operational.
    // 中文: 如果 Wi-Fi 和蓝牙都已连接，设备处于完全工作状态。
    if (isWifiConnected && isBleConnected) {
        ledOff(); // EN: Turn off the LED. / 中文: 关闭 LED。
    } else {
        // EN: If either connection is down, blink blue to indicate "not ready".
        // 中文: 如果任一连接断开，则蓝色闪烁以表示“未就绪”。
        if (millis() - _lastStatusBlink > 500) { // EN: Non-blocking blink every 500ms. / 中文: 每 500 毫秒进行非阻塞闪烁。
            _lastStatusBlink = millis();
            _statusLedState = !_statusLedState;
            if (_statusLedState) {
                setLedColor(_strip.Color(0, 0, 255)); // Blue
            } else {
                ledOff();
            }
        }
    }
}

void OtaUpdater::setApModeLed(bool active) {
    // EN: AP mode also takes LED control, setting _isOtaInProgress to true temporarily.
    // 中文: AP 模式也接管 LED 控制权，暂时将 _isOtaInProgress 设置为 true。
    _isOtaInProgress = active; 
    if (active) {
        setLedColor(_strip.Color(255, 255, 0)); // EN: Solid Yellow for AP mode. / 中文: AP 模式常亮黄灯。
    } else {
        ledOff();
    }
}

void OtaUpdater::checkAndUpdate() {
    _isOtaInProgress = true; // EN: Take control of the LED for the OTA process. / 中文: 为 OTA 流程接管 LED 控制权。
    Serial.println("Checking for OTA update...");
    setLedColor(_strip.Color(0, 0, 255)); 

    WiFiClientSecure client;
    client.setInsecure();
    HTTPClient http;

    if (!http.begin(client, OTA_JSON_URL)) {
        Serial.println("Failed to connect to OTA server.");
        ledOff();
        _isOtaInProgress = false; // EN: Release LED control. / 中文: 释放 LED 控制权。
        return;
    }

    int httpCode = http.GET();
    if (httpCode != HTTP_CODE_OK) {
        Serial.printf("Failed to get OTA config, HTTP code: %d\n", httpCode);
        http.end();
        ledOff();
        _isOtaInProgress = false; // EN: Release LED control. / 中文: 释放 LED 控制权。
        return;
    }

    String payload = http.getString();
    http.end();

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, payload);

    if (error) {
        Serial.print("JSON deserialization failed: ");
        Serial.println(error.c_str());
        ledOff();
        _isOtaInProgress = false; // EN: Release LED control. / 中文: 释放 LED 控制权。
        return;
    }

    long long serverVersion = atoll(doc["version"].as<const char*>());
    String firmwareUrl = doc["url"].as<String>();
    String firmwareMd5 = doc["md5"].as<String>();

    Serial.printf("Current version: %lld, Server version: %lld\n", _currentVersion, serverVersion);

    if (serverVersion > _currentVersion) {
        Serial.println("New firmware version available. Starting update...");
        performUpdate(firmwareUrl, firmwareMd5);
    } else {
        Serial.println("Firmware is up to date.");
        ledOff();
        _isOtaInProgress = false; // EN: Release LED control. / 中文: 释放 LED 控制权。
    }
}

void OtaUpdater::performUpdate(const String& url, const String& md5) {
    HTTPClient http;
    bool success = false;
    const size_t bufferSize = 4096;
    uint8_t* buffer = (uint8_t*)malloc(bufferSize);

    if (!buffer) {
        Serial.println("Failed to allocate buffer for OTA download!");
        setLedColor(_strip.Color(255, 0, 0));
        _isOtaInProgress = false; // EN: Release LED control (fatal error). / 中文: 释放 LED 控制权（致命错误）。
        return;
    }

    for (int i = 0; i < _maxRetries; i++) {
        Serial.printf("Downloading firmware... Attempt %d/%d\n", i + 1, _maxRetries);

        for(int j = 0; j < 3; j++) {
            setLedColor(_strip.Color(255, 0, 0));
            delay(100);
            setLedColor(_strip.Color(0, 0, 255));
            delay(100);
        }
        ledOff();
        delay(200);

        if (!http.begin(url)) {
            Serial.println("Failed to begin HTTP for firmware download.");
            continue;
        }

        int httpCode = http.GET();
        if (httpCode != HTTP_CODE_OK) {
            Serial.printf("Firmware download failed, HTTP code: %d\n", httpCode);
            http.end();
            continue;
        }

        int contentLength = http.getSize();
        if (contentLength <= 0) {
            Serial.println("Content length is zero, skipping update.");
            http.end();
            continue;
        }

        Serial.println("Update process started. Flashing firmware...");
        setLedColor(_strip.Color(0, 255, 0));

        if (!Update.begin(contentLength)) {
            Serial.print("Not enough space to begin OTA: ");
            Serial.println(Update.errorString());
            http.end();
            Update.abort();
            goto end_update_free_buffer; 
        }

        Update.setMD5(md5.c_str());

        WiFiClient* stream = http.getStreamPtr();
        size_t written = 0;

        while (stream->connected() && (written < contentLength || contentLength == -1)) { 
            size_t bytesToRead = stream->available();
            if (bytesToRead) {
                if (bytesToRead > bufferSize) bytesToRead = bufferSize;
                int bytesRead = stream->readBytes(buffer, bytesToRead);
                if (bytesRead > 0) {
                    Update.write(buffer, bytesRead);
                    written += bytesRead;
                }
            } else {
                delay(1);
            }
        }
        
        if (!Update.end()) {
            Serial.print("Update failed with error: ");
            Serial.println(Update.errorString());
            http.end();
            Update.abort(); 
            continue;
        }

        if (Update.isFinished()) {
            Serial.println("Update successful! Rebooting...");
            success = true;
            http.end(); 
            free(buffer); 
            ESP.restart();
        } else {
             Serial.println("Update failed MD5 check or other finalization error.");
             http.end();
             Update.abort(); 
             continue;
        }
    }

end_update_free_buffer:
    free(buffer); 

    if (!success) {
        Serial.println("Failed to update firmware after all retries.");
        setLedColor(_strip.Color(255, 0, 0));
    }
    _isOtaInProgress = false; // EN: Release LED control. / 中文: 释放 LED 控制权。
}