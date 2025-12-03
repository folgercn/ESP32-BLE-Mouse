#ifndef BLEDRIVER_H
#define BLEDRIVER_H

#include <NimBLEDevice.h>
#include <NimBLEHIDDevice.h>
#include <Arduino.h>

// 定义全量参数结构体 (默认值仅作兜底)
struct ActionOptions {
    // 屏幕参数
    int screenW = 1080;
    int screenH = 2248;
    
    // 时间参数 (单位: ms)
    int delayHover = 20;    // 移动到位置后，按下前的悬停时间
    int delayPress = 20;    // 按下后，开始滑动/抬起前的等待时间
    int delayInterval = 10; // 滑动过程中每一步的间隔 (控制平滑度)
    int delayRelease = 20;  // 抬起后的冷却时间
    int delayDoubleCheck = 20; // 防止断触的二次抬起延迟

    // 算法参数
    int curveStrength = 15; // 贝塞尔曲线弯曲程度 (百分比 0-100)
};

class BleDriver {
public:
    void begin(String deviceName);
    bool isConnected();
    
    // 动作接口现在接收 options 结构体
    void click(int x, int y, ActionOptions opts);
    void swipe(int x1, int y1, int x2, int y2, int duration, ActionOptions opts);

private:
    NimBLEHIDDevice* _hid;
    NimBLECharacteristic* _input;
    
    void sendRaw(int x, int y, uint8_t state);
    // mapVal 现在需要传入宽高
    long mapVal(int val, int maxPixel); 
};

#endif