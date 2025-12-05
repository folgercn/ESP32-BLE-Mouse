### **English User Manual: ESP32 BLE Mouse Gateway**

#### **1. First-Time Wi-Fi Setup**

When you power on the device for the first time, or after it has been reset, it needs to be configured to connect to your local Wi-Fi network.

1.  **Power On:** Connect the device to a 5V power source (e.g., a USB port or phone charger).
2.  **Status Light:** The status LED will begin **flashing red**, indicating that it is in Wi-Fi setup mode.
3.  **Connect to Hotspot:** On your phone or computer, open your Wi-Fi settings. Find and connect to a Wi-Fi network named **`Wacom-Setup-XXXXXX`**.
4.  **Configuration Page:** Once connected, a captive portal page should automatically open in your browser.
    *   If the page does not open automatically, please manually open a browser and navigate to **`192.168.4.1`**.
5.  **Enter Credentials:** On this page, select your home or office Wi-Fi network from the list, enter its password, and click "Save".
6.  **Reboot:** The device will save the settings and automatically reboot. The red flashing light will stop.

After rebooting, the device will automatically connect to the Wi-Fi network you provided.

#### **2. Bluetooth Pairing**

Once the device is connected to Wi-Fi, it will start advertising as a Bluetooth device.

1.  **Status Light:** The status LED will now be **flashing blue**. This means Wi-Fi is connected, but it is waiting for a Bluetooth connection.
2.  **Open Bluetooth Settings:** On your Android device, go to `Settings` -> `Bluetooth`.
3.  **Scan for Devices:** Scan for new devices. You should see a device named in the format **`Wacom-XXXX-YYY`** (where XXXX are characters from the device's MAC address and YYY is from its IP address).
4.  **Pair Device:** Tap on the device to pair it. Your Android system will recognize it as an "Input Device" or "Wacom Digitizer". No special apps are needed.
5.  **Connection Complete:** Once pairing is successful, the **blue flashing light will turn off**. The device is now fully operational.

#### **3. Configuring Auto Swipe**

You can configure the automatic swiping and liking features through a web interface.

1.  **Find Device IP:** The easiest way to find the device IP is to check the Bluetooth device name. The device name format is **`Wacom-XXXX-YYY`**, where **YYY** (the last 3 digits) represents the last segment of the device's IP address.
    *   **Example:** If the Bluetooth name is `Wacom-96DC-153`, then the device IP is likely `192.168.1.153` (assuming your router uses the `192.168.1.x` network range).
    *   **Alternative:** If the above method doesn't work, you can find the IP address in your router's administration page under "Connected Devices" or "DHCP Clients" list.
2.  **Access Web Page:** Open a web browser on a computer or phone that is connected to the same Wi-Fi network and navigate to `http://<YOUR_DEVICE_IP>/auto_swipe`. (Replace `<YOUR_DEVICE_IP>` with the actual IP address, e.g., `http://192.168.1.153/auto_swipe`).
3.  **Modify Settings:** Use the form on the page to enable/disable auto-swipe, change swipe intervals, speed, and configure the random liking behavior.
4.  **Save:** Click the "Save" button. The new settings will take effect immediately and are saved permanently on the device.

#### **4. System Reset**

The device provides multiple ways to reset settings:

1.  **Factory Reset (BOOT Button):** This is the most comprehensive reset method, clearing both Wi-Fi and Bluetooth settings.
    *   **Steps:** Press and hold the **BOOT button** (GPIO0) on the device for **2 seconds**.
    *   **Indicator:** The onboard LED will flash rapidly **25 times** to indicate the reset process has started.
    *   **Result:** All Wi-Fi credentials, Bluetooth pairings, and custom settings will be cleared. The device will automatically restart and enter Wi-Fi setup mode (red flashing LED).
    *   **Use Case:** Use this when you want to completely restore the device to factory settings or when you need to reconfigure everything from scratch.

2.  **Reset Wi-Fi Only (Web Interface):** This method only clears Wi-Fi settings, keeping Bluetooth pairings intact.
    *   **Steps:** Open a web browser and navigate to `http://<YOUR_DEVICE_IP>/reset_wifi` (replace `<YOUR_DEVICE_IP>` with your device's actual IP address).
    *   **Result:** Wi-Fi credentials will be cleared and the device will restart. After restart, it will enter Wi-Fi setup mode (red flashing LED). Bluetooth pairings remain unchanged.
    *   **Use Case:** Use this when you need to change Wi-Fi networks or fix Wi-Fi connection issues.

3.  **Reset Bluetooth Pairing Only (Web Interface):** This method only clears Bluetooth pairings, keeping Wi-Fi settings intact.
    *   **Steps:** Access the auto-swipe configuration page at `http://<YOUR_DEVICE_IP>/auto_swipe`, then click the **"Reset BLE Pairing"** button.
    *   **Result:** All Bluetooth pairings will be cleared. You will need to pair your phone again, but Wi-Fi settings remain unchanged.
    *   **Use Case:** Use this when you need to reconnect your phone or fix Bluetooth connection issues.

#### **5. Understanding the Status LED**

The single LED light on the device indicates its current status:

*   **Flashing Red:** Wi-Fi is not configured. The device is in Access Point (AP) mode, waiting for you to set it up.
*   **Flashing Blue:** Wi-Fi is connected, but no Bluetooth client (your phone) is connected yet. The device is ready for pairing.
*   **LED Off:** Normal operation. Both Wi-Fi and Bluetooth are connected and the device is ready to receive commands.
*   **Solid Green (During OTA):** The device is actively downloading and flashing a new firmware update. Do not power it off.
*   **Solid Red (Persistent):** A critical error occurred, most likely a failed firmware update. Please try resetting the device.

---

### **中文用户手册：ESP32 蓝牙鼠标网关**

#### **1. 首次 Wi-Fi 配置**

当您第一次为设备通电，或设备被重置后，它需要被配置连接到您本地的 Wi-Fi 网络。

1.  **通电**：将设备连接到一个 5V 电源（例如，USB 接口或手机充电器）。
2.  **状态灯**：此时，设备的状态指示灯会 **红色闪烁**，这表示它正处于 Wi-Fi 配置模式。
3.  **连接热点**：在您的手机或电脑上，打开 Wi-Fi 设置。找到并连接到一个名为 **`Wacom-Setup-XXXXXX`** 的 Wi-Fi 热点。
4.  **配置页面**：连接成功后，一个配置页面应该会自动在您的浏览器中弹出。
    *   如果页面没有自动弹出，请手动打开浏览器并访问 **`192.168.4.1`**。
5.  **输入凭据**：在配置页面上，从列表中选择您的家庭或办公室 Wi-Fi 网络，输入其密码，然后点击“Save” (保存)。
6.  **重启**：设备将保存设置并自动重启。重启后，红色闪烁的灯会熄灭。

重启后，设备将自动连接到您刚才提供的 Wi-Fi 网络。

#### **2. 蓝牙配对**

一旦设备成功连接到 Wi-Fi，它将作为一个蓝牙设备开始广播。

1.  **状态灯**：此时，设备的状态指示灯会变为 **蓝色闪烁**。这表示 Wi-Fi 已连接，但正在等待蓝牙客户端的连接。
2.  **打开蓝牙设置**：在您的安卓设备上，进入 `设置` -> `蓝牙`。
3.  **扫描设备**：扫描新的蓝牙设备。您应该能看到一个名称格式为 **`Wacom-XXXX-YYY`** 的设备（其中 XXXX 来自设备的 MAC 地址，YYY 来自其 IP 地址）。
4.  **配对设备**：点击该设备进行配对。您的安卓系统会将其识别为一个“输入设备”或“Wacom 数位板”。无需安装任何特殊 App。
5.  **连接完成**：配对成功后，**蓝色闪烁的灯将会熄灭**。至此，设备已完全准备就绪。

#### **3. 配置自动上划 (Auto Swipe)**

您可以通过一个网页界面来配置自动上划和随机点赞功能。

1.  **查找设备 IP**：最简单的方法是查看蓝牙设备名称。设备名称格式为 **`Wacom-XXXX-YYY`**，其中 **YYY**（最后3位数字）代表设备 IP 地址的末尾段。
    *   **示例**：如果蓝牙名称为 `Wacom-96DC-153`，那么设备 IP 很可能是 `192.168.1.153`（假设您的路由器使用 `192.168.1.x` 网段）。
    *   **备选方法**：如果上述方法不适用，您也可以在路由器的管理页面，在"已连接设备"或"DHCP 客户端"列表中查找设备的 IP 地址。
2.  **访问网页**：在任何一台连接到同一 Wi-Fi 网络的电脑或手机上，打开浏览器并访问 `http://<您的设备IP>/auto_swipe`。（请将 `<您的设备IP>` 替换为真实的 IP 地址，例如：`http://192.168.1.153/auto_swipe`）。
3.  **修改设置**：使用页面上的表单来启用/禁用自动上划，更改上划的间隔、速度，以及配置随机点赞的行为。
4.  **保存**：点击"Save" (保存) 按钮。新的设置将立即生效并被永久保存在设备上。

#### **4. 系统重置**

设备提供了多种重置设置的方法：

1.  **恢复出厂设置 (BOOT 按键)**：这是最彻底的重置方法，会清除 Wi-Fi 和蓝牙的所有设置。
    *   **操作步骤**：按住设备上的 **BOOT 按键**（GPIO0）**2 秒**。
    *   **指示灯**：板载 LED 会快速闪烁 **25 次**，表示重置过程已开始。
    *   **结果**：所有 Wi-Fi 凭据、蓝牙配对和自定义设置将被清除。设备会自动重启并进入 Wi-Fi 配置模式（红色闪烁指示灯）。
    *   **使用场景**：当您需要完全恢复设备到出厂设置，或需要从头开始重新配置所有内容时使用。

2.  **仅重置 Wi-Fi (网页接口)**：此方法仅清除 Wi-Fi 设置，保留蓝牙配对。
    *   **操作步骤**：打开浏览器并访问 `http://<您的设备IP>/reset_wifi`（请将 `<您的设备IP>` 替换为设备的实际 IP 地址）。
    *   **结果**：Wi-Fi 凭据将被清除，设备将重启。重启后，设备会进入 Wi-Fi 配置模式（红色闪烁指示灯）。蓝牙配对保持不变。
    *   **使用场景**：当您需要更换 Wi-Fi 网络或修复 Wi-Fi 连接问题时使用。

3.  **仅重置蓝牙配对 (网页接口)**：此方法仅清除蓝牙配对，保留 Wi-Fi 设置。
    *   **操作步骤**：访问自动上划配置页面 `http://<您的设备IP>/auto_swipe`，然后点击 **"重置蓝牙配对"** 按钮。
    *   **结果**：所有蓝牙配对将被清除。您需要重新配对手机，但 Wi-Fi 设置保持不变。
    *   **使用场景**：当您需要重新连接手机或修复蓝牙连接问题时使用。

#### **5. 理解状态指示灯 (LED)**

设备上的单个 LED 灯指示了它当前的工作状态：

*   **红色闪烁**：Wi-Fi 未配置。设备正处于热点 (AP) 模式，等待您进行连接和设置。
*   **蓝色闪烁**：Wi-Fi 已连接，但尚未有任何蓝牙客户端（您的手机）连接。设备已准备好等待配对。
*   **灯熄灭**：正常工作状态。Wi-Fi 和蓝牙均已成功连接，设备准备好接收指令。
*   **绿色常亮 (OTA 期间)**：设备正在下载并烧录新的固件更新。在此期间请勿断开电源。
*   **红色常亮 (持续)**：发生了严重错误，最常见的是固件更新失败。请尝试重置设备。
