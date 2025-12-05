#include "ota.h"
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Update.h>
#include <stdlib.h> // For malloc and free

// NeoPixel 灯珠参数
#define PIXEL_PIN    48  // GPIO a la que está conectada la tira de LEDs
#define PIXEL_COUNT  1   // Número de LEDs en la tira

const char* OTA_JSON_URL = "https://datav-d-gzcom.oss-cn-hangzhou.aliyuncs.com/esp32/s3/otaup.json";

// --- LED 控制辅助函数 ---
void OtaUpdater::setLedColor(uint32_t color) {
    _strip.setPixelColor(0, color);
    _strip.show();
}

void OtaUpdater::ledOff() {
    _strip.clear();
    _strip.show();
}
// ------------------------

void OtaUpdater::begin(long currentVersion) {
    _currentVersion = currentVersion;
    // 初始化 NeoPixel 灯珠
    _strip = Adafruit_NeoPixel(PIXEL_COUNT, PIXEL_PIN, NEO_GRB + NEO_KHZ800);
    _strip.begin();
    ledOff();
}

void OtaUpdater::tick() {
    if (_lastCheckMillis == 0) {
        _lastCheckMillis = millis();
        return;
    }

    if (millis() - _lastCheckMillis > _checkIntervalMillis) {
        checkAndUpdate();
        _lastCheckMillis = millis();
    }
}

void OtaUpdater::checkAndUpdate() {
    Serial.println("Checking for OTA update...");
    // 状态: 正在获取JSON -> 蓝色
    setLedColor(_strip.Color(0, 0, 255)); 

    WiFiClientSecure client;
    client.setInsecure(); // 跳过证书验证 (生产环境请务必替换为证书验证)
    HTTPClient http;

    if (!http.begin(client, OTA_JSON_URL)) {
        Serial.println("Failed to connect to OTA server.");
        ledOff();
        return;
    }

    int httpCode = http.GET();
    if (httpCode != HTTP_CODE_OK) {
        Serial.printf("Failed to get OTA config, HTTP code: %d\n", httpCode);
        http.end();
        ledOff();
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
        return;
    }

    long serverVersion = doc["version"].as<long>();
    String firmwareUrl = doc["url"].as<String>();
    String firmwareMd5 = doc["md5"].as<String>();

    Serial.printf("Current version: %ld, Server version: %ld\n", _currentVersion, serverVersion);

    if (serverVersion > _currentVersion) {
        Serial.println("New firmware version available. Starting update...");
        performUpdate(firmwareUrl, firmwareMd5);
    } else {
        Serial.println("Firmware is up to date.");
        ledOff();
    }
}

void OtaUpdater::performUpdate(const String& url, const String& md5) {
    HTTPClient http;
    bool success = false;
    const size_t bufferSize = 4096; // 缓冲区大小
    uint8_t* buffer = (uint8_t*)malloc(bufferSize);

    if (!buffer) {
        Serial.println("Failed to allocate buffer for OTA download!");
        setLedColor(_strip.Color(255, 0, 0)); // 红色表示致命错误
        return;
    }

    for (int i = 0; i < _maxRetries; i++) {
        Serial.printf("Downloading firmware... Attempt %d/%d\n", i + 1, _maxRetries);

        // 状态: 尝试下载中 -> 红蓝交替闪烁 (短暂)
        for(int j = 0; j < 3; j++) {
            setLedColor(_strip.Color(255, 0, 0)); // 红
            delay(100);
            setLedColor(_strip.Color(0, 0, 255)); // 蓝
            delay(100);
        }
        ledOff(); // 闪烁完短暂关闭
        delay(200);

        if (!http.begin(url)) {
            Serial.println("Failed to begin HTTP for firmware download.");
            continue; // 重试
        }

        int httpCode = http.GET();
        if (httpCode != HTTP_CODE_OK) {
            Serial.printf("Firmware download failed, HTTP code: %d\n", httpCode);
            http.end();
            continue; // 重试
        }

        int contentLength = http.getSize();
        if (contentLength <= 0) {
            Serial.println("Content length is zero, skipping update.");
            http.end();
            continue; // 重试
        }

        Serial.println("Update process started. Flashing firmware...");
        setLedColor(_strip.Color(0, 255, 0)); // 状态: 升级中 -> 常亮绿色

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

        // 手动读取流并写入 Update (在此过程中 LED 保持常亮绿色)
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
                delay(1); // 等待数据到达
            }
        }
        
        if (!Update.end()) {
            Serial.print("Update failed with error: ");
            Serial.println(Update.errorString());
            http.end();
            Update.abort(); 
            continue; // 重试
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
             continue; // 重试
        }
    }

end_update_free_buffer:
    free(buffer); 

    if (!success) {
        Serial.println("Failed to update firmware after all retries.");
        setLedColor(_strip.Color(255, 0, 0)); // 状态: 最终失败 -> 常亮红色
    }
}
