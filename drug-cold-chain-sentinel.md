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

# Acknowledgements

We express our sincere gratitude to our Faculty Mentor, **Neethu KC, Assistant Professor, Department of Electronics and Communication Engineering, Government Engineering College Thrissur**, for her continuous guidance, technical support, and encouragement throughout this project.

We thank the Department of Electronics and Communication Engineering and the management of **Government Engineering College Thrissur (GEC Thrissur), Kerala, India** for providing the necessary resources and a supportive environment for innovation.

We deeply appreciate the **IEEE MYOSA Event 5.0 organizers** and the **IEEE Sensors Council** for creating this incredible platform that bridges academic learning with real-world engineering challenges. Being selected among the top 15 teams globally has been a transformative and deeply motivating experience for our team.

Finally, we thank our teammates for their dedication through every sensor error, every library patch, and every debugging session — for turning an idea into a working device that we genuinely believe can save lives.

---

# Overview

Every year, the World Health Organization estimates that a significant proportion of vaccines arrive at clinics already compromised by cold-chain failures — with no visible sign of damage. The cause: undetected temperature excursions, physical mishandling, and unauthorized access during transit. Current commercial cold-chain monitors cost between $50 and $200 per trip and typically track only a single parameter.

The **Drug Cold-Chain Sentinel** is a compact, self-contained IoT monitoring device built on the MYOSA Mini IoT Kit (ESP32-WROOM-32E). It monitors vaccine and medicine shipments in real time across **6 parameters simultaneously** — temperature, humidity, shock and impact, tamper detection, barometric pressure, and altitude change.

Every alert event is permanently stored as a timestamped JSON log in the device's onboard flash memory (SPIFFS), and a live dashboard is served over a self-hosted Wi-Fi hotspot that any smartphone can access by scanning a QR code on the box — with **zero internet, zero cloud, and zero external infrastructure required**.

The MYOSA platform is uniquely suited to this application. All six sensor boards connect via the MYOSA JST I2C daisy chain — no soldering, no complex wiring — stacked compactly within a standard medicine shipping box. The ESP32's onboard Wi-Fi eliminates the need for any external communication module, enabling the self-hosted QR-code dashboard that makes the system truly zero-infrastructure.

## Key Features

- 6-parameter simultaneous monitoring — temperature, humidity, shock, tamper, pressure, altitude
- Self-hosted Wi-Fi dashboard — board creates its own hotspot at `192.168.4.1`, no internet needed
- QR code access — anyone nearby scans the box and sees the live dashboard instantly on any phone
- Permanent on-device logging — SPIFFS flash memory stores every event even after power off
- OLED real-time display — rotating pages showing status, readings, and network info
- Instant buzzer alarm — local audio alert on any threshold breach
- Digital chain-of-custody passport — exportable JSON log and Python-generated timeline chart
- Zero proprietary infrastructure — no router, no server, no cloud account needed
- Cost-effective — approximately $15 USD total, a fraction of $50–200 commercial alternatives

---

# Demo / Examples

## Images

<p align="center">
  <img src="/cover.jpg" width="800"><br/>
  <i>Drug Cold-Chain Sentinel — fully assembled device with all 6 sensors connected</i>
</p>

<p align="center">
  <img src="/dashboard-phone.jpg" width="800"><br/>
  <i>Live Wi-Fi dashboard at 192.168.4.1 showing sensor readings and alert status</i>
</p>

<p align="center">
  <img src="/oled-status-ok.jpg" width="800"><br/>
  <i>OLED display showing normal operating status and network information</i>
</p>

<p align="center">
  <img src="/oled-alert.jpg" width="800"><br/>
  <i>OLED display showing live temperature alert condition</i>
</p>

<p align="center">
  <img src="/serial-monitor-ok.jpg" width="800"><br/>
  <i>Serial monitor confirming successful initialization of all sensors</i>
</p>

<p align="center">
  <img src="/event-log.jpg" width="800"><br/>
  <i>Dashboard event log showing timestamped alert events</i>
</p>

<p align="center">
  <img src="/kit-components.jpg" width="800"><br/>
  <i>MYOSA Mini Kit components and sensor modules used in the system</i>
</p>

<p align="center">
  <img src="/enclosure.jpg" width="800"><br/>
  <i>Device enclosure placed inside a medicine shipment box with QR code access</i>
</p>

## Videos

### Drug Cold-Chain Sentinel — 5-minute recorded presentation

<video controls width="100%">
  <source src="/presentation-video.mp4" type="video/mp4">
</video>

### Drug Cold-Chain Sentinel — 3-minute live demonstration

<video controls width="100%">
  <source src="/demo-video.mp4" type="video/mp4">
</video>

### Drug Cold-Chain Sentinel — Live dashboard walkthrough

<video controls width="100%">
  <source src="/dashboard-video.mp4" type="video/mp4">
</video>

### Drug Cold-Chain Sentinel — Chain-of-custody logging visualization

<video controls width="100%">
  <source src="/chain-of-custody-using-json.mp4" type="video/mp4">
</video>

---

# Features (Detailed)

## 1. Real-Time Multi-Parameter Cold-Chain Monitoring

Environmental parameters are updated periodically, while critical events such as shock and tamper conditions are monitored continuously in real time.

The AM2302 temperature and humidity sensor monitors the primary cold-chain parameters — alerting whenever temperature exceeds 8°C or drops below 2°C. Humidity is monitored with a maximum threshold of 85%.

All readings update live on both the OLED display and the Wi-Fi dashboard simultaneously, giving both field workers and supervisors immediate visibility into shipment integrity.

<p align="center">
  <img src="/dashboard-phone.jpg" width="800"><br/>
  <i>Live dashboard showing temperature, humidity, pressure, shock level, and tamper status</i>
</p>

## 2. Shock and Drop Detection

The MPU6050 accelerometer measures acceleration across the X, Y, and Z axes continuously.

The total force magnitude is calculated using:

```plaintext
sqrt(X² + Y² + Z²)
```

When the calculated magnitude exceeds 7 m/s², a `SHOCK_ALERT` is immediately logged with timestamp and force value. This enables detection of drops and rough handling events that could damage sensitive medical products.

## 3. Tamper Detection via Light Sensor

The APDS9960 light sensor faces the inside lid of the shipment box.

Under sealed conditions, the sensor detects near-zero ambient light. If the box is opened during transit, ambient light increases sharply and triggers a `TAMPER_ALERT`.

This creates a permanent tamper-evident digital record of every unauthorized access event throughout the supply chain.

## 4. Altitude and Pressure Monitoring

The BMP180 sensor continuously monitors atmospheric pressure.

Sudden pressure changes greater than 5 hPa between consecutive readings generate a `PRESSURE_ALERT`, indicating possible altitude changes such as aircraft transport or rapid environmental variation.

## 5. Self-Hosted Wi-Fi Dashboard

The ESP32 operates in softAP mode and creates its own Wi-Fi network named:

```plaintext
ColdChainSentinel
```

The onboard web server hosts a dark-themed dashboard at:

```plaintext
http://192.168.4.1
```

The dashboard displays:
- Live sensor readings
- Alert status
- Event history
- Device condition

The page auto-refreshes every 10 seconds.

### Additional API Endpoints

```plaintext
192.168.4.1/data      → live sensor JSON
192.168.4.1/log       → raw event log
192.168.4.1/clearlog  → clear stored log
```

## 6. Permanent On-Device Event Logging (SPIFFS)

All events are stored as JSON entries inside the ESP32 SPIFFS filesystem.

Each entry contains:
- Timestamp
- Event type
- Description
- Sensor value
- Sequential event number

### Example Log Entries

```json
{"t":"00:14:22","e":"TEMP_ALERT","d":"TOO HOT","v":"9.10","n":47}
{"t":"00:16:05","e":"TAMPER_ALERT","d":"BOX OPENED","v":"1.00","n":48}
{"t":"00:18:33","e":"SHOCK_ALERT","d":"IMPACT DETECTED","v":"9.71","n":49}
{"t":"00:22:11","e":"PRESSURE_ALERT","d":"ALTITUDE CHANGE","v":"12.30","n":50}
```

## 7. Python Chain-of-Custody Report Generator

A companion Python script (`generate_chart.py`) downloads the device log and generates a multi-panel analytical report showing:
- Temperature history
- Event timeline
- Shock events
- Alert distribution
- Device summary

The report is generated fully offline without cloud dependency.

---

# Experimental Results

The prototype was tested through multiple simulated shipment scenarios including:
- Temperature excursions above safe vaccine storage limits
- Sudden impact and drop events
- Unauthorized box opening
- Pressure and altitude variation
- Long-duration continuous monitoring

## Results

- Real-time alert response within seconds
- Reliable SPIFFS event persistence after reboot
- Stable Wi-Fi dashboard operation without internet
- Accurate timestamped JSON logging
- Successful offline report generation using Python tools
- Stable sensor communication throughout extended operation

The complete system operated continuously using a standard 5V USB power bank.

---

# Usage Instructions

```plaintext
Step 1: Connect JST sensor chain
Motherboard → OLED → MPU6050 → APDS9960 → BMP180

Step 2: Connect AM2302 sensor
Red   → 3V3
Black → GND
Data  → GPIO16

Step 3: Connect buzzer
GND → GND
VCC → VIN
SIG → GPIO12

Step 4: Power on device
Use any 5V USB charger or power bank

Step 5: Connect to Wi-Fi
SSID: ColdChainSentinel

Step 6: Open dashboard
Browser → http://192.168.4.1

Step 7: Place inside shipment box
Position APDS9960 facing the box lid
Attach QR code outside the package
```

## Generate Report

```bash
python generate_chart.py
```

## Clear Log

```plaintext
http://192.168.4.1/clearlog
```

---

# File Structure

```plaintext
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
  ├── dashboard-video.mp4
  ├── chain-of-custody-using-json.mp4
  ├── ColdChainSentinel_v4.ino
  └── generate_chart.py
```

---

# License

MIT License — free to use, modify, and distribute with attribution.

Developed by Team Amrut for IEEE International MYOSA Event 5.0.

---

# Contribution Notes

This project was developed as a functional prototype for the IEEE International MYOSA Event 5.0 IEEE BioSensors 2026 Track.

The MPU6050 WHO_AM_I = 0x70 compatibility patch documented in the Requirements section may be required for newer chip variants.

Contributions and real-world deployment adaptations are welcome.

Team Amrut  
Government Engineering College Thrissur  
Kerala, India
