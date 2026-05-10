---
publishDate: 2026-05-09T00:00:00Z
title: Drug Cold-Chain Sentinel — A Multi-Sensor IoT Platform for Real-Time Vaccine & Medicine Integrity Monitoring
excerpt: A compact, self-contained IoT device that monitors vaccine shipments across 6 parameters simultaneously — temperature, humidity, shock, tamper, pressure, and altitude — with a self-hosted Wi-Fi dashboard accessible via QR code, zero internet required, and a permanent tamper-evident digital chain-of-custody log.
image: cover.jpg
tags:
  - cold-chain
  - vaccine-monitoring
  - iot
  - esp32
  - myosa
  - healthcare
  - biosensors
---

> A $15 device that gives every vaccine shipment a tamper-evident digital passport — deployable anywhere in the world, with zero internet, zero cloud, and zero infrastructure required.

---

## Acknowledgements

We express our sincere gratitude to our Faculty Mentor, **Neethu KC, Assistant Professor, Department of Electronics and Communication Engineering, Government Engineering College Thrissur**, for her continuous guidance, technical support, and encouragement throughout this project.

We thank the Department of Electronics and Communication Engineering and the management of **Government Engineering College Thrissur (GEC Thrissur), Kerala, India** for providing the necessary resources and a supportive environment for innovation.

We deeply appreciate the **IEEE MYOSA Event 5.0 organizers** and the **IEEE Sensors Council** for creating this incredible platform that bridges academic learning with real-world engineering challenges. Being selected among the top 15 teams globally has been a transformative and deeply motivating experience for our team.

Finally, we thank our teammates for their dedication through every sensor error, every library patch, and every debugging session — for turning an idea into a working device that we genuinely believe can save lives.

---

## Overview

Every year, the World Health Organization estimates that a significant proportion of vaccines arrive at clinics already compromised by cold-chain failures — with no visible sign of damage. The cause: undetected temperature excursions, physical mishandling, and unauthorized access during transit. Current commercial cold-chain monitors cost between $50 and $200 per trip and track only a single parameter.

The **Drug Cold-Chain Sentinel** is a compact, self-contained IoT monitoring device built on the MYOSA Mini IoT Kit (ESP32-WROOM-32E). It monitors vaccine and medicine shipments in real time across **6 parameters simultaneously** — temperature, humidity, shock and impact, tamper detection, barometric pressure, and altitude change. Every alert event is permanently stored as a timestamped JSON log in the device's onboard flash memory (SPIFFS), and a live dashboard is served over a self-hosted Wi-Fi hotspot that any smartphone can access by scanning a QR code on the box — with **zero internet, zero cloud, and zero external infrastructure required**.

The MYOSA platform is uniquely suited to this application. All six sensor boards connect via the MYOSA JST I2C daisy chain — no soldering, no complex wiring — stacked compactly enough to fit inside a standard medicine shipping box. The ESP32's onboard Wi-Fi eliminates the need for any external communication module, enabling the self-hosted QR-code dashboard that makes the system truly zero-infrastructure.

**Key features:**
* 6-parameter simultaneous monitoring — temperature, humidity, shock, tamper, pressure, altitude
* Self-hosted Wi-Fi dashboard — board creates its own hotspot at 192.168.4.1, no internet needed
* QR code access — anyone nearby scans the box and sees the live dashboard instantly on any phone
* Permanent on-device logging — SPIFFS flash memory stores every event even after power off
* OLED real-time display — 3 rotating pages showing status, readings, and network info
* Instant buzzer alarm — local audio alert on any threshold breach
* Digital chain-of-custody passport — exportable JSON log and Python-generated timeline chart
* Zero proprietary infrastructure — no router, no server, no cloud account needed
* Cost-effective — approximately $15 USD total, a fraction of $50–200 commercial alternatives

---

## Demo / Examples

### **Images**

<p align="center">
  <img src="/cover.jpg" width="800"><br/>
  <i>Drug Cold-Chain Sentinel — fully assembled device with all 6 sensors connected</i>
</p>

<p align="center">
  <img src="/dashboard-phone.jpg" width="800"><br/>
  <i>Live Wi-Fi dashboard at 192.168.4.1 — showing all sensor readings, ALERT ACTIVE badge, and event log on smartphone</i>
</p>

<p align="center">
  <img src="/oled-status-ok.jpg" width="800"><br/>
  <i>OLED display showing COLD CHAIN SENTINEL — STATUS OK with event count and IP address</i>
</p>

<p align="center">
  <img src="/oled-alert.jpg" width="800"><br/>
  <i>OLED display showing ALERT — TEMPERATURE OUT OF RANGE triggered in real time</i>
</p>

<p align="center">
  <img src="/serial-monitor-ok.jpg" width="800"><br/>
  <i>Arduino Serial Monitor confirming all 6 sensors initialized successfully with [OK] status</i>
</p>

<p align="center">
  <img src="/event-log.jpg" width="800"><br/>
  <i>Dashboard event log showing TAMPER_ALERT, SHOCK_ALERT, TEMP_ALERT with timestamps and sensor values</i>
</p>

<p align="center">
  <img src="/kit-components.jpg" width="800"><br/>
  <i>MYOSA Mini Kit components — motherboard, OLED, MPU6050, BMP180, APDS9960 sensor boards, and JST cables</i>
</p>

<p align="center">
  <img src="/enclosure.jpg" width="800"><br/>
  <i>Device assembled inside transparent jewelry box placed in cardboard medicine shipping box with QR code sticker</i>
</p>

### **Videos**

📽️ Drug Cold-Chain Sentinel — 5-minute recorded presentation covering the problem, solution, MYOSA platform exploitation, and live demo teaser

<video controls width="100%">
  <source src="/presentation-video.mp4" type="video/mp4">
</video>

📽️ Drug Cold-Chain Sentinel — 3-minute live demonstration showing all 5 demo steps: baseline, temperature excursion, shock event, tamper detection, and chain-of-custody review

<video controls width="100%">
  <source src="/demo-video.mp4" type="video/mp4">
</video>

---

## Features (Detailed)

### **1. Real-Time Multi-Parameter Cold-Chain Monitoring**

The MYOSA motherboard continuously polls all connected sensor boards via the I2C bus every 30 seconds. The AM2302 temperature and humidity sensor monitors the primary cold-chain parameters — alerting whenever temperature exceeds 8°C (vaccine spoilage threshold) or drops below 2°C (freezing damage threshold). Humidity is monitored with a maximum alert threshold of 85%. All readings update live on both the OLED screen and the Wi-Fi dashboard simultaneously, giving the field worker and remote supervisor a real-time view of shipment integrity.

<p align="center">
  <img src="/dashboard-phone.jpg" width="800"><br/>
  <i>Real-time dashboard showing live temperature, humidity, pressure, shock level, and tamper status</i>
</p>

### **2. Shock and Drop Detection**

The MPU6050 accelerometer measures linear acceleration across all 3 axes simultaneously (X, Y, Z). Total force magnitude is calculated as `sqrt(X² + Y² + Z²)`. When this exceeds 7 m/s², a `SHOCK_ALERT` is immediately logged with the exact magnitude value and timestamp. This detects drops, impacts, and rough handling events that could crack vaccine vials — events that temperature sensors alone can never detect.

### **3. Tamper Detection via Light Sensor**

The APDS9960 light and colour sensor is positioned facing the box lid. Inside the sealed medicine box, the sensor reads near-zero ambient light. When the lid is opened during transit, ambient light floods in and the clear channel reading exceeds 200 units — immediately triggering a `TAMPER_ALERT` with timestamp. This creates a permanent, unforgeable digital record of every unauthorized access event throughout the supply chain.

### **4. Altitude and Pressure Monitoring**

The BMP180 barometric pressure sensor monitors atmospheric pressure continuously. When pressure changes by more than 5 hPa between consecutive readings, a `PRESSURE_ALERT` is logged indicating an altitude change — for example, a package being loaded onto an aircraft. This adds an additional layer of supply chain visibility beyond what any single-sensor device can provide.

### **5. Self-Hosted Wi-Fi Dashboard**

The ESP32's onboard Wi-Fi chip operates in softAP (Access Point) mode, creating its own Wi-Fi network named "ColdChainSentinel". The ESPAsyncWebServer library serves a complete dark-themed HTML dashboard at 192.168.4.1. The page auto-refreshes every 10 seconds and displays: live sensor readings, tamper status, and the complete colour-coded event log table. A QR code sticker on the shipping box encodes the URL — anyone nearby scans it and the live dashboard loads on any smartphone browser with no app installation required.

Additional API endpoints:
- `192.168.4.1/data` — live sensor data as JSON
- `192.168.4.1/log` — complete raw event log for download
- `192.168.4.1/clearlog` — wipe the log for a new shipment

### **6. Permanent On-Device Event Logging (SPIFFS)**

Every alert event is written as a single JSON line to `/log.json` in the ESP32's SPIFFS flash file system. This file survives power cycles — when the device restarts, it reads the existing log and continues appending. Each entry contains timestamp, event type, detail string, sensor value, and sequential event number — creating a tamper-evident, permanent digital chain-of-custody passport for every shipment.

```json
{"t":"00:14:22","e":"TEMP_ALERT","d":"TOO HOT","v":"9.10","n":47}
{"t":"00:16:05","e":"TAMPER_ALERT","d":"BOX OPENED","v":"1.00","n":48}
{"t":"00:18:33","e":"SHOCK_ALERT","d":"IMPACT DETECTED","v":"9.71","n":49}
{"t":"00:22:11","e":"PRESSURE_ALERT","d":"ALTITUDE CHANGE","v":"12.30","n":50}
```

### **7. Python Chain-of-Custody Report Generator**

A companion Python script (`generate_chart.py`) connects to the board's Wi-Fi hotspot, downloads the complete log from `192.168.4.1/log`, and generates a 5-panel dark-themed analysis report saved as `coldchain-report.png`. The report includes: temperature timeline with safe zone highlighted (2–8°C), full event timeline scatter plot, event count summary bar chart, shock events chart, and live device summary panel — all without any internet connection.

---

## Usage Instructions

```plaintext
Step 1: Connect JST sensor chain (daisy chain in this exact order)
  Motherboard J2 → OLED → MPU6050 → APDS9960 → BMP180

Step 2: Connect AM2302 temperature sensor via jumper wires
  Red wire   → 3V3 pin  (left side header, 2nd from top)
  Black wire → GND pin  (left side header, top pin)
  Data wire  → D16 pin  (left side header, 3rd from top)

Step 3: Connect buzzer via jumper wires
  GND wire → GND pin  (right side header, top pin)
  5V wire  → VIN pin  (right side header, 2nd from top)
  SIG wire → D12 pin  (right side header)

Step 4: Power on via any 5V USB charger or power bank
  Device boots automatically
  OLED shows "SENTINEL READY!" with live temperature reading

Step 5: Access the live dashboard
  Phone: Settings → Wi-Fi → connect to "ColdChainSentinel" (no password)
  Browser: type 192.168.4.1
  Live dashboard loads instantly — no app required

Step 6: Place device inside medicine shipping box
  Position APDS9960 sensor facing the box lid for tamper detection
  Attach QR code sticker (http://192.168.4.1) on outside of box

Step 7: Monitor shipment
  All alert events are logged automatically to SPIFFS
  Buzzer sounds on any threshold breach
  Dashboard updates every 10 seconds automatically
```

To generate the chain-of-custody report after shipment:
```bash
# Connect laptop to "ColdChainSentinel" Wi-Fi first
python generate_chart.py
# Generates coldchain-report.png with full 5-panel event analysis
```

To clear the log for a new shipment:
```plaintext
Connect to ColdChainSentinel Wi-Fi
Open browser → type: 192.168.4.1/clearlog
Log cleared — device ready for next shipment
```

---

## Code Used

```cpp
// ============================================================
//   DRUG COLD-CHAIN SENTINEL  v2.0
//   IEEE International MYOSA Event 5.0
//   Team: Amrut | GEC Thrissur, Kerala
// ============================================================

#include <Wire.h>
#include <WiFi.h>
#include <SPIFFS.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include <AM2302-Sensor.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP085_U.h>
#include <Adafruit_APDS9960.h>
#include <Adafruit_SSD1306.h>
#include <ArduinoJson.h>
#include <vector>

// --- Pin assignments ---
#define DHT_PIN         16   // AM2302 data pin → GPIO 16
#define BUZZER_PIN      12   // Buzzer signal → GPIO 12
#define OLED_RESET_PIN  -1

// --- Alert thresholds ---
#define TEMP_MIN             2.0   // °C minimum (too cold)
#define TEMP_MAX             8.0   // °C maximum (vaccine spoiled)
#define HUMIDITY_MAX        85.0   // % maximum
#define SHOCK_THRESHOLD      7.0   // m/s² — above this = impact
#define PRESSURE_DELTA_MAX   5.0   // hPa sudden change = altitude shift

// --- Check all sensor readings against thresholds ---
void checkAlerts() {
  bool any = false;

  // Temperature alert
  bool newTemp = (currentTemp < TEMP_MIN || currentTemp > TEMP_MAX);
  if (newTemp && !tempAlert) {
    logEvent("TEMP_ALERT",
      currentTemp > TEMP_MAX ? "TOO HOT" : "TOO COLD", currentTemp);
    buzz(300, 3);  // 3 short beeps
  }
  tempAlert = newTemp;
  if (tempAlert) any = true;

  // Shock alert — magnitude across all 3 axes
  float mag = sqrt(accelX*accelX + accelY*accelY + accelZ*accelZ);
  if (mag > SHOCK_THRESHOLD && !shockAlert) {
    logEvent("SHOCK_ALERT", "IMPACT DETECTED", mag);
    buzz(500, 2);  // 2 long beeps
  }

  // Tamper alert — light sensor detects box opening
  if (lightDetected && !tamperAlert) {
    logEvent("TAMPER_ALERT", "BOX OPENED", 1.0);
    buzz(1000, 1);  // 1 alarm beep
  }

  // Pressure alert — altitude change detected
  float delta = abs(currentPressure - lastPressure);
  if (delta > PRESSURE_DELTA_MAX) {
    logEvent("PRESSURE_ALERT", "ALTITUDE CHANGE", delta);
    buzz(200, 1);
  }

  systemOK = !any;
}
```

Full source code: [`ColdChainSentinel_v4.ino`](ColdChainSentinel_v4.ino)

Python chart generator: [`generate_chart.py`](generate_chart.py)

---

## Tech Stack

* **MYOSA Motherboard (ESP32-WROOM-32E)** — Central controller, Wi-Fi softAP hotspot, SPIFFS flash storage
* **AM2302 / DHT22** — Temperature and humidity (primary cold-chain sensor, GPIO 16)
* **MPU6050** — 3-axis accelerometer for shock and drop detection
* **BMP180** — Barometric pressure sensor for altitude change detection
* **APDS9960** — Light and colour sensor for tamper detection
* **SSD1306 OLED** — 0.96-inch 128×64 real-time status display
* **Buzzer** — Actuator for instant local audio alarm
* **ESPAsyncWebServer + AsyncTCP** — Asynchronous self-hosted Wi-Fi dashboard
* **SPIFFS** — On-device flash file system for permanent event log
* **ArduinoJson 7.4.3** — JSON serialization for log entries and REST API
* **AM2302-Sensor 1.5.0** — Temperature sensor library
* **Adafruit MPU6050** (patched for WHO_AM_I = 0x70) — Accelerometer library
* **Adafruit BMP085 Unified 1.1.3** — Barometric pressure library
* **Adafruit Unified Sensor 1.1.15** — Sensor abstraction layer
* **Python 3 + matplotlib + requests** — Offline chain-of-custody report generator

---

## Requirements / Installation

### Hardware

```plaintext
- MYOSA Mini IoT Kit (motherboard + sensor boards + 4× JST cables)
- AM2302 / DHT22 temperature humidity sensor module (purchased separately)
- 3× female-to-male jumper wires (20cm)
- USB-C cable + any 5V USB charger or power bank
- Transparent jewelry box ~12×8×5cm (inner device enclosure)
- Cardboard medicine shipping box ~20×15×10cm (outer enclosure)
- Printed QR code sticker pointing to http://192.168.4.1
- Optional: 3.7V LiPo battery + TP4056 charging module for wireless operation
```

### Software Installation

```bash
# Step 1: Install Arduino IDE 2.x from arduino.cc/en/software

# Step 2: Add ESP32 board support
# File → Preferences → Additional Boards Manager URLs:
# https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
# Tools → Boards Manager → search "esp32" → install "esp32 by Espressif Systems" v2.0.17

# Step 3: Install libraries via Tools → Manage Libraries:
# AM2302-Sensor | Adafruit MPU6050 | Adafruit Unified Sensor
# Adafruit BMP085 Unified | Adafruit APDS9960 | Adafruit SSD1306 | ArduinoJson

# Step 4: Install ESPAsyncWebServer + AsyncTCP manually
# Download ZIP from github.com/me-no-dev/AsyncTCP
# Download ZIP from github.com/me-no-dev/ESPAsyncWebServer
# Sketch → Include Library → Add .ZIP Library for each

# Step 5: Install Python dependencies
pip install matplotlib requests
```

**Critical library patch — required for newer MPU6050 chip variants (WHO_AM_I = 0x70):**

```cpp
// File: Arduino/libraries/Adafruit_MPU6050/Adafruit_MPU6050.cpp
// FIND:
if (chip_id.read() != MPU6050_DEVICE_ID) { return false; }

// REPLACE WITH:
uint8_t chip_id_val = chip_id.read();
if (chip_id_val != MPU6050_DEVICE_ID &&
    chip_id_val != 0x70 &&
    chip_id_val != 0x72) {
  return false;
}
// This patch is required for newer MPU6050 chip batches
// that return WHO_AM_I register value 0x70 instead of standard 0x68
```

**Arduino IDE upload settings:**

```plaintext
Tools → Board            → ESP32 Dev Module
Tools → Port             → select your COM port
Tools → Partition Scheme → Default 4MB with spiffs   ← CRITICAL
Click Upload arrow → wait for "Done uploading"
```

---

## File Structure

```
/drug-cold-chain-sentinel
  ├── drug-cold-chain-sentinel.md
  ├── cover.jpg
  ├── dashboard-phone.jpg
  ├── oled-status-ok.jpg
  ├── oled-alert.jpg
  ├── serial-monitor-ok.jpg
  ├── event-log.jpg
  ├── kit-components.jpg
  ├── enclosure.jpg
  ├── presentation-video.mp4
  ├── demo-video.mp4
  ├── ColdChainSentinel_v4.ino
  └── generate_chart.py
```

---

## License

MIT License — free to use, modify, and distribute with attribution. Developed by Team Amrut for IEEE International MYOSA Event 5.0.

---

## Contribution Notes

This project was developed as a functional prototype for the IEEE International MYOSA Event 5.0, IEEE BioSensors 2026 Track. The MPU6050 WHO_AM_I = 0x70 library patch documented in the Requirements section above may be required for anyone using newer chip batches. Contributions and adaptations for real-world cold-chain deployment are welcome. Contact: Team Amrut, Government Engineering College Thrissur, Kerala, India.
