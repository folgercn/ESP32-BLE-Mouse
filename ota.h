#ifndef OTA_H
#define OTA_H

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

class OtaUpdater {
public:
    void begin(long currentVersion);
    void checkAndUpdate();
    void tick();

private:
    long _currentVersion;
    unsigned long _lastCheckMillis = 0;
    const long _checkIntervalMillis = 12 * 60 * 60 * 1000; // 12 hours
    const int _ledPin = 48;
    const int _maxRetries = 3;

    Adafruit_NeoPixel _strip;

    // LED 控制辅助函数
    void setLedColor(uint32_t color);
    void ledOff();

    void performUpdate(const String& url, const String& md5);
};

#endif // OTA_H
