#include "BleDriver.h"

// Wacom 描述符 (保持不变)
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
        // 请求极速模式 (降低延迟)
        pServer->updateConnParams(pServer->getPeerInfo(0).getConnHandle(), 6, 6, 0, 100);
    }
    void onDisconnect(NimBLEServer* pServer) {
        Serial.println(">>> [BLE] Disconnected! <<<");
        // 断开后立刻重新广播，允许别人连接
        NimBLEDevice::startAdvertising();
    }
};

void BleDriver::begin(String deviceName) {
    Serial.println("[BLE] Init: " + deviceName);
    NimBLEDevice::init(deviceName.c_str());
    
    // 保留历史配对，便于手机自动重连；如需手动清除，走 BOOT 长按或网页重置
    
    // 开启安全认证 (安卓必须)
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
    pAdvertising->setAppearance(0x03C2); // Mouse appearance
    pAdvertising->addServiceUUID(_hid->getHidService()->getUUID());

    // === 修复2：拆分广播包，兼容三星和苹果 ===
    
    // 包1：主广播包 (必须包含 UUID，否则苹果/三星不认)
    NimBLEAdvertisementData advData;
    advData.setFlags(0x06); // General Discovery Mode
    advData.setPartialServices(NimBLEUUID("1812")); // 明确告诉手机：我是 HID 设备
    advData.setAppearance(0x03C2);
    pAdvertising->setAdvertisementData(advData);

    // 包2：扫描响应包 (放入长名字)
    // 手机扫到主包后，会主动问“你叫啥”，此时发这个包
    NimBLEAdvertisementData scanData;
    scanData.setName(deviceName.c_str());
    pAdvertising->setScanResponseData(scanData);

    pAdvertising->start();
    _hid->setBatteryLevel(100);
}

bool BleDriver::isConnected() {
    return NimBLEDevice::getServer()->getConnectedCount() > 0;
}

void BleDriver::resetPairing() {
    Serial.println("[BLE] Reset pairing + restart advertising");
    NimBLEDevice::deleteAllBonds();
    NimBLEDevice::startAdvertising();
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
    click(x, y, 1, opts);
}

void BleDriver::click(int x, int y, int count, ActionOptions opts) {
    long tx = mapVal(x, opts.screenW);
    long ty = mapVal(y, opts.screenH);
    
    if (count < 1) count = 1;

    sendRaw(tx, ty, 0x04); 
    if(opts.delayHover > 0) delay(opts.delayHover);      
    
    for (int i = 0; i < count; i++) {
        sendRaw(tx, ty, 0x05); 
        if(opts.delayPress > 0) delay(opts.delayPress); 
        
        sendRaw(tx, ty, 0x04); 
        if (i < count - 1) {
            if(opts.delayMultiClickInterval > 0) delay(opts.delayMultiClickInterval);
        }
    }
    
    if(opts.delayRelease > 0) delay(opts.delayRelease);
    
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

    long midX = (tx1 + tx2) / 2;
    long midY = (ty1 + ty2) / 2;
    long dist = sqrt(pow(tx2 - tx1, 2) + pow(ty2 - ty1, 2));
    long offset = dist * (opts.curveStrength / 100.0); 
    if (random(0, 2) == 0) offset = -offset;

    long cx = midX;
    long cy = midY;
    if (abs(tx2 - tx1) < abs(ty2 - ty1)) cx += offset;
    else cy += offset;

    sendRaw(tx1, ty1, 0x04); 
    if(opts.delayHover > 0) delay(opts.delayHover);

    sendRaw(tx1, ty1, 0x05); 
    if(opts.delayPress > 0) delay(opts.delayPress);

    int stepTime = opts.delayInterval;
    if (stepTime <= 0) stepTime = 10;
    
    int steps = duration / stepTime;
    if (steps < 2) steps = 2;

    for (int i = 1; i <= steps; i++) {
        float t = (float)i / steps;
        float u = 1 - t;
        float tt = t * t;
        float uu = u * u;

        long curveX = (uu * tx1) + (2 * u * t * cx) + (tt * tx2);
        long curveY = (uu * ty1) + (2 * u * t * cy) + (tt * ty2);

        sendRaw(curveX, curveY, 0x05); 
        delay(stepTime);
    }
    
    sendRaw(tx2, ty2, 0x04);
    if(opts.delayDoubleCheck > 0) {
        delay(opts.delayDoubleCheck);
        sendRaw(tx2, ty2, 0x04); 
    }
}
