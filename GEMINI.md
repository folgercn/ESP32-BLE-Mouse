# GEMINI.md - ESP32 BLE Mouse Gateway

## Project Overview

This is a C++ project for the ESP32 microcontroller that turns it into a versatile Bluetooth Low Energy (BLE) mouse and touch input gateway. It cleverly identifies as a Wacom Digitizer tablet to ensure native compatibility with Android devices without requiring any special drivers or apps.

The primary function of this project is to allow for remote, automated control of an Android device. All actions, such as clicks and swipes, are triggered by sending JSON payloads to an HTTP server running on the ESP32. This makes it ideal for scenarios like automated testing, remote assistance, or large-scale device control.

The project is architected in a modular way, with clear separation of concerns:

*   **`ESP32-BLE-Mouse.ino`**: The main application logic, responsible for initializing the hardware, server, and other components.
*   **`BleDriver`**: Handles all BLE HID communication, emulating the Wacom device and translating commands into low-level HID reports.
*   **`NetHelper`**: Manages Wi-Fi connectivity, including a user-friendly one-time setup via the WiFiManager library.
*   **`AutoSwipe`**: Implements a configurable auto-swipe and auto-click feature for performing repetitive actions.
*   **`OtaUpdater`**: Handles Over-the-Air (OTA) firmware updates, checking for new versions and flashing them with visual feedback via a WS2818 LED. It now pauses BLE (NimBLE deinit) before flashing to free RAM, prints OTA JSON/URL and 10% progress, enforces a 15s stall timeout, and shows distinct LED states: cyan (download), green (flash), red (failure).

## Building and Running

### Hardware Requirements

*   An ESP32-based development board (e.g., ESP32-DevKitC, ESP32-WROOM).
*   A WS2818 compatible RGB LED connected to GPIO 48.

### Software Dependencies

This project is built using the Arduino IDE. You will need to install the following libraries via the Arduino Library Manager:

*   `NimBLE-Arduino`
*   `ArduinoJson`
*   `WiFiManager`
*   `Adafruit NeoPixel`

To install libraries in Arduino IDE:
1.  Open Arduino IDE.
2.  Go to `Sketch` > `Include Library` > `Manage Libraries...`.
3.  Search for each library name (e.g., "NimBLE-Arduino"), select it, and click `Install`.

### Build & Flash

1.  Open the project folder in Arduino IDE.
2.  Ensure the correct ESP32 board and COM port are selected under `Tools` > `Board` and `Tools` > `Port`.
3.  Compile and upload the firmware to the device by clicking the `Upload` button.

### First-Time Setup

1.  After flashing, the ESP32 will create a Wi-Fi hotspot with an SSID similar to `Wacom-Setup-XXXX`.
2.  Connect to this hotspot with a phone or computer. A captive portal should automatically open in your browser.
3.  If it doesn't, manually navigate to `192.168.4.1`.
4.  In the portal, select your local Wi-Fi network, enter the password, and save. You can also configure a static IP address.
5.  The ESP32 will then connect to your network and reboot.

### Running the Device

Once connected to your Wi-Fi network, the device is ready.
*   It will start advertising as a BLE device with a name like `Wacom-{MAC4}-{IP3}`.
*   The HTTP server will be listening for commands on port 80. You can find the device's IP address in your router's client list or in the Serial Monitor output.

## Development Conventions

*   **Modular Design:** The code is well-organized into classes (`BleDriver`, `NetHelper`, `AutoSwipeManager`, `OtaUpdater`), each handling a specific part of the functionality.
*   **JSON-Driven:** All actions are controlled via a well-defined JSON API. This keeps the device firmware generic and allows for complex logic to be handled by a controlling server.
*   **Configuration over Code:** Many parameters, such as screen resolution and action delays, are configurable at runtime via the JSON API, avoiding the need to recompile the firmware for minor adjustments.
*   **Bilingual Comments:** The code includes comments in both English and Chinese, making it accessible to a wider audience.
*   **Human-like Motion:** The swipe actions are not simple linear movements. They use Bézier curves and random jitter to simulate a more human-like interaction, which can be important for avoiding detection in automated systems.
*   **Bilingual Code Comments:** All new code must include detailed comments in both English and Chinese to maintain clarity and ease of maintenance.
*   **Documentation Synchronization:** Before any git commit, the `README.md` and `CHANGELOG.md` files must be updated to reflect the changes, with content provided in both English and Chinese.
*   **Zero Regression Policy:** When modifying existing code or critical paths, absolute diligence is required to prevent regressions and low-level errors. This includes thoroughly reviewing logic, dependencies, and side effects. Such errors are fundamentally unacceptable and must be meticulously avoided.
*   **OTA Stability Notes:** On boot the firmware logs PSRAM presence/size and free heaps. OTA uses a 1KB buffer to reduce TLS memory pressure; if HTTPS still OOMs, switch to HTTP or ensure PSRAM is enabled. BLE is paused during OTA to free RAM; LEDs show cyan/green/red for download/flash/fail and progress logs print every 10%.
