#include "BleDriver.h"

// Wacom 描述符 (保持不变)
// EN: Wacom HID report descriptor (do not change)
static const uint8_t hidReportDescriptor[] = {
  0x05, 0x0D, 0x09, 0x02, 0xA1, 0x01, 0x85, 0x01, 0x09, 0x20, 0xA1, 0x00,
  0x09, 0x42, 0x15, 0x00, 0x25, 0x01, 0x75, 0x01, 0x95, 0x01, 0x81, 0x02,
  0x09, 0x44, 0x81, 0x02, 0x09, 0x32, 0x81, 0x02, 0x95, 0x05, 0x81, 0x03,
  0x05, 0x01, 0x09, 0x30, 0x16, 0x00, 0x00, 0x26, 0xFF, 0x7F, 0x75, 0x10, 0x95, 0x01, 0x81, 0x02,
  0x09, 0x31, 0x81, 0x02, 0xC0, 0xC0
};

class ConnectionCallbacks : public NimBLEServerCallbacks {
    void onConnect(NimBLEServer* pServer) {
        Serial.println(">>> [BLE] Connected! <<<");
        pServer->updateConnParams(pServer->getPeerInfo(0).getConnHandle(), 6, 6, 0, 100);
    }
    void onDisconnect(NimBLEServer* pServer) {
        Serial.println(">>> [BLE] Disconnected! <<<");
        NimBLEDevice::startAdvertising();
    }
};

void BleDriver::begin(String deviceName) {
    Serial.println("[BLE] Init: " + deviceName);
    NimBLEDevice::init(deviceName.c_str());
    NimBLEDevice::setSecurityAuth(true, true, true);

    NimBLEServer* pServer = NimBLEDevice::createServer();
    pServer->setCallbacks(new ConnectionCallbacks());

    _hid = new NimBLEHIDDevice(pServer);
    _input = _hid->getInputReport(1);
    
    _hid->setManufacturer("Espressif");
    _hid->setPnp(0x02, 0xe502, 0xa111, 0x0210);
    _hid->setHidInfo(0x00, 0x01);
    _hid->setReportMap((uint8_t*)hidReportDescriptor, sizeof(hidReportDescriptor));
    _hid->startServices();

    NimBLEAdvertising* pAdvertising = NimBLEDevice::getAdvertising();
    pAdvertising->setAppearance(0x03C2);
    pAdvertising->addServiceUUID(_hid->getHidService()->getUUID());

    NimBLEAdvertisementData advData;
    advData.setName(deviceName.c_str());
    advData.setManufacturerData("ESP32");
    advData.setFlags(0x06);
    pAdvertising->setAdvertisementData(advData);

    pAdvertising->start();
    _hid->setBatteryLevel(100);
}

bool BleDriver::isConnected() {
    return NimBLEDevice::getServer()->getConnectedCount() > 0;
}

long BleDriver::mapVal(int val, int maxPixel) {
    return map(val, 0, maxPixel, 0, 32767);
}

void BleDriver::sendRaw(int x, int y, uint8_t state) {
    if (!isConnected() || _input == nullptr) return;

    uint8_t buffer[6];
    buffer[0] = state;
    buffer[1] = x & 0xFF; buffer[2] = (x >> 8) & 0xFF;
    buffer[3] = y & 0xFF; buffer[4] = (y >> 8) & 0xFF;
    
    _input->setValue(buffer, 5);
    _input->notify();
}

void BleDriver::click(int x, int y, ActionOptions opts) {
    // 使用传入的宽高进行映射
    // EN: Map coordinates according to provided screen size
    long tx = mapVal(x, opts.screenW);
    long ty = mapVal(y, opts.screenH);
    
    // 1. 悬停
    // EN: Step 1: hover at target position
    sendRaw(tx, ty, 0x04); 
    if(opts.delayHover > 0) delay(opts.delayHover);      
    
    // 2. 按下
    // EN: Step 2: press down
    sendRaw(tx, ty, 0x05); 
    if(opts.delayPress > 0) delay(opts.delayPress); // 这里复用 delayPress 作为点击持续时间
    // EN: Reuse delayPress as click duration
    
    // 3. 抬起
    // EN: Step 3: release
    sendRaw(tx, ty, 0x04); 
    if(opts.delayRelease > 0) delay(opts.delayRelease);
    
    // 4. 双重确认
    // EN: Step 4: extra release for double-check
    if(opts.delayDoubleCheck > 0) {
        delay(opts.delayDoubleCheck);
        sendRaw(tx, ty, 0x04); 
    }
}

void BleDriver::swipe(int x1, int y1, int x2, int y2, int duration, ActionOptions opts) {
    long tx1 = mapVal(x1, opts.screenW);
    long ty1 = mapVal(y1, opts.screenH);
    long tx2 = mapVal(x2, opts.screenW);
    long ty2 = mapVal(y2, opts.screenH);

    // 计算控制点 (贝塞尔曲线)
    // EN: Compute control point for quadratic Bézier curve
    long midX = (tx1 + tx2) / 2;
    long midY = (ty1 + ty2) / 2;
    
    // 计算偏移量：根据传入的百分比 curveStrength
    // EN: Compute offset based on curveStrength percentage
    long dist = sqrt(pow(tx2 - tx1, 2) + pow(ty2 - ty1, 2));
    long offset = dist * (opts.curveStrength / 100.0); 
    
    // 随机决定方向 (这个还是保留随机性比较好，或者你也想把随机种子放进 JSON?)
    // EN: Randomly choose bending direction (could also be seeded via JSON if needed)
    if (random(0, 2) == 0) offset = -offset;

    long cx = midX;
    long cy = midY;
    if (abs(tx2 - tx1) < abs(ty2 - ty1)) cx += offset;
    else cy += offset;

    // 1. 悬停
    // EN: Step 1: hover at start point
    sendRaw(tx1, ty1, 0x04); 
    if(opts.delayHover > 0) delay(opts.delayHover);

    // 2. 按下
    // EN: Step 2: press down before moving
    sendRaw(tx1, ty1, 0x05); 
    if(opts.delayPress > 0) delay(opts.delayPress);

    // 3. 移动
    // EN: Step 3: move along the Bézier curve
    int stepTime = opts.delayInterval;
    if (stepTime <= 0) stepTime = 10; // 保护
    // EN: Safety guard for minimum step interval
    
    int steps = duration / stepTime;
    if (steps < 2) steps = 2;
    // EN: Ensure at least two steps for a valid curve

    for (int i = 1; i <= steps; i++) {
        float t = (float)i / steps;
        float u = 1 - t;
        float tt = t * t;
        float uu = u * u;

        long curveX = (uu * tx1) + (2 * u * t * cx) + (tt * tx2);
        long curveY = (uu * ty1) + (2 * u * t * cy) + (tt * ty2);

        sendRaw(curveX, curveY, 0x05); 
        delay(stepTime); // 使用传入的步进间隔
        // EN: Use provided interval between each swipe step
    }
    
    // 4. 抬起
    // EN: Step 4: release at end point (with optional double-check)
    sendRaw(tx2, ty2, 0x04);
    if(opts.delayDoubleCheck > 0) {
        delay(opts.delayDoubleCheck);
        sendRaw(tx2, ty2, 0x04); 
    }
}