# Raspberry Pi Pico W Environmental Monitoring System

This project implements a compact, Wi-Fi-enabled environmental monitoring system using the Raspberry Pi Pico W. The device measures temperature and pressure using the SPL06-007 sensor and displays information on a 1.5" OLED screen. It also serves data over a local web server.

---

## Features

- **Real-time Monitoring**: Displays temperature and pressure data on the OLED screen.
- **Wi-Fi Enabled**: Hosts a local web server for data retrieval via HTTP requests.
- **Companion App Ready**: Communicates with external applications via a token-based authentication system.
- **Customizable Settings**: Includes adjustable screensaver intervals and optional features such as beep notifications.
- **Security**: Protect access with a unique device token.

---

## Prerequisites

### Hardware
1. Raspberry Pi Pico W.
2. SPL06-007 temperature and pressure sensor.
3. SSD1351-compatible 1.5" OLED display.
4. Optional: Active buzzer for notifications.

### Software
1. [Arduino IDE](https://www.arduino.cc/en/software)
2. Required libraries:
   - `Adafruit_GFX`
   - `Adafruit_SSD1351`
   - `ArduinoJson`
   - `UptimeString`
   - `WiFi`
   - `LEAmDNS`
3. [Arduino Pico core](https://github.com/earlephilhower/arduino-pico) from Earle F. Philhower

---

## Installation and Setup

### Hardware Connections

| Component        | Pico W Pin | Description                         |
|-------------------|------------|-------------------------------------|
| OLED (SSD1351)   | Various    | Use SPI connections (SCLK, MOSI, DC, CS, RST). |
| SPL06-007 Sensor | I2C (SDA, SCL) | Connect to I2C pins.                 |
| Buzzer (optional) | GPIO 22   | Used for beep notifications.         |

### Software Setup

1.  Install the [Arduino Pico core](https://github.com/earlephilhower/arduino-pico) from Earle F. Philhower
2. Install the required libraries using the Arduino Library Manager.
3. Clone or download this repository and open `v2.ino` in the Arduino IDE.
4. Configure your Wi-Fi credentials in the `secrets.h` file:
   ```cpp
   #define WIFI_SSID "Your_SSID"
   #define WIFI_PASS "Your_PASSWORD"

5. Adjust customizable settings in secrets.h:
   ```cpp
   const unsigned long screensaverInterval = 600000;
   const bool beeperEnabled = false;

6. Upload the code to your Raspberry Pi Pico W using the Arduino IDE.
---
### Usage
**Device Startup**
- Upon boot, the device initializes the sensors and attempts to connect to the configured Wi-Fi network.
- The OLED screen will display the assigned IP address and a unique token for access.

**Accessing Data**
- Use a browser or application to send HTTP GET requests to the device's IP.

**Example endpoints:**
```bash
  http://[IP_ADDRESS]/?token=[DEVICE_TOKEN]
```
- `/`: Retrieves temperature and pressure data in JSON format (requires token).
- `/welcome`: Displays the welcome screen.
- `/reboot`: Reboots the device (requires token).
- `/bugCheck`: Triggers a simulated error (developer mode only).

**Example Output**
- JSON response from /:

```json
  {
    "sensor": "SPL06",
    "tempC": 23.45,
    "tempF": 74.21,
    "airPressure": 1013.25
  }
```
**Screensaver**
- The OLED will blank after the interval defined in screensaverInterval to prevent burn-in.
---
### Troubleshooting
- **Wi-Fi Connection Fails:** Ensure correct SSID/password and that the network operates on 2.4 GHz.
- **Invalid Token:** Verify the token displayed on the device during startup.
- **Sensor Issues:** Check I2C connections and ensure the SPL06-007 is powered.
