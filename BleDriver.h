#ifndef BLEDRIVER_H
#define BLEDRIVER_H

#include <NimBLEDevice.h>
#include <NimBLEHIDDevice.h>
#include <Arduino.h>

// 定义全量参数结构体 (默认值仅作兜底)
// EN: Full option struct for all motion parameters (defaults are just fallbacks)
struct ActionOptions {
    // 屏幕参数
    // EN: Screen parameters
    int screenW = 1080;
    int screenH = 2248;
    
    // 时间参数 (单位: ms)
    // EN: Time parameters (ms)
    int delayHover = 20;    // 移动到位置后，按下前的悬停时间
    // EN: Hover time before press after moving to target
    int delayPress = 20;    // 按下后，开始滑动/抬起前的等待时间
    // EN: Delay after press before swipe/release
    int delayInterval = 10; // 滑动过程中每一步的间隔 (控制平滑度)
    // EN: Interval between swipe steps (controls smoothness)
    int delayRelease = 20;  // 抬起后的冷却时间
    // EN: Cooldown after release
    int delayMultiClickInterval = 30; // 多次点击之间的间隔
    // EN: Gap between multi-click presses
    int delayDoubleCheck = 20; // 防止断触的二次抬起延迟
    // EN: Extra delay before second release to avoid ghost touches

    // 算法参数
    // EN: Algorithm parameters
    int curveStrength = 15; // 贝塞尔曲线弯曲程度 (百分比 0-100)
    // EN: Bézier curve bending strength (0-100%)
};

class BleDriver {
public:
    void begin(String deviceName);
    bool isConnected();

    
    // 动作接口现在接收 options 结构体
    // EN: Action APIs now take the options struct
    void click(int x, int y, ActionOptions opts);
    void click(int x, int y, int count, ActionOptions opts);
    void swipe(int x1, int y1, int x2, int y2, int duration, ActionOptions opts);
    
    // 重置配对信息并重新广播
    void resetPairing();
    // 定时任务：关掉脉冲灯
    void tick();
    // WiFi 数据包闪 RX 灯
    void pulseRx(unsigned long durationMs);

private:
    NimBLEHIDDevice* _hid;
    NimBLECharacteristic* _input;
    bool _txLedOn = false;
    bool _rxLedOn = false;
    unsigned long _txLedOffAt = 0;
    unsigned long _rxLedOffAt = 0;
    
    void pulseLed(bool& ledFlag, unsigned long& offAt, int pin, unsigned long durationMs);
    void clearLeds();
    void sendRaw(int x, int y, uint8_t state);
    // mapVal 现在需要传入宽高
    // EN: mapVal now maps using provided screen size
    long mapVal(int val, int maxPixel); 
};

#endif
