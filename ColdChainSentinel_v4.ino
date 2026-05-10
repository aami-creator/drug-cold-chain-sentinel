// ============================================================
//   DRUG COLD-CHAIN SENTINEL  v2.0
//   IEEE International MYOSA Event 5.0
//   Team: Amrut | GEC Thrissur, Kerala
// ============================================================
//
//  LIBRARIES USED (all installed):
//  - AM2302-Sensor@1.5.0       ← your DHT/AM2303 sensor
//  - Adafruit Unified Sensor   ← required by MPU & BMP
//  - MPU6050@1.4.4             ← shock detection
//  - Adafruit BMP085@1.1.3     ← pressure/altitude
//  - Adafruit SSD1306          ← OLED screen
//  - ArduinoJson@7.4.3         ← JSON for dashboard
//  - ESPAsyncWebServer         ← Wi-Fi dashboard
//  - AsyncTCP                  ← required by above
//
// ============================================================

// ---- LIBRARIES --------------------------------------------
#include <Wire.h>
#include <WiFi.h>
#include <SPIFFS.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include <AM2302-Sensor.h>          // ← AM2302 / AM2303 / DHT22 (all same)
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP085_U.h>
#include <Adafruit_APDS9960.h>
#include <Adafruit_SSD1306.h>
#include <ArduinoJson.h>
#include <vector>

// ============================================================
//   SECTION 1: SETTINGS
//   Change these values if your pins are different
// ============================================================

// --- Wi-Fi Hotspot ---
const char* WIFI_SSID     = "ColdChainSentinel";
const char* WIFI_PASSWORD = "";           // empty = no password

// --- Pin numbers ---
#define DHT_PIN         16             // AM2302 data wire → GPIO 4
#define BUZZER_PIN      12             // Buzzer → GPIO 2
#define OLED_RESET_PIN -1             // -1 = share ESP32 reset pin

// --- OLED size ---
#define OLED_WIDTH    128
#define OLED_HEIGHT    64

// --- Alert thresholds ---
#define TEMP_MIN             2.0      // °C minimum (too cold)
#define TEMP_MAX             8.0      // °C maximum (too hot = vaccine spoiled)
#define HUMIDITY_MAX        85.0      // % maximum
#define SHOCK_THRESHOLD     7.0      // m/s² — above this = drop/impact
#define PRESSURE_DELTA_MAX   5.0      // hPa sudden change = altitude shift

// --- Timing ---
#define SENSOR_INTERVAL   30000       // read sensors every 30 seconds
#define OLED_INTERVAL      2000       // refresh OLED every 2 seconds

// --- Storage ---
#define LOG_FILE  "/log.json"


// ============================================================
//   SECTION 2: CREATE OBJECTS
// ============================================================

AM2302::AM2302_Sensor am2302{DHT_PIN}; // temp & humidity sensor
Adafruit_MPU6050       mpu;               // accelerometer (shock)
Adafruit_BMP085_Unified bmp(10085);       // pressure sensor
Adafruit_APDS9960      apds;              // light sensor (tamper)
Adafruit_SSD1306       display(OLED_WIDTH, OLED_HEIGHT, &Wire, OLED_RESET_PIN);
AsyncWebServer         server(80);        // web server on port 80


// ============================================================
//   SECTION 3: GLOBAL VARIABLES
// ============================================================

float  currentTemp      = 0.0;
float  currentHumidity  = 0.0;
float  currentPressure  = 0.0;
float  lastPressure     = 0.0;
float  accelX           = 0.0;
float  accelY           = 0.0;
float  accelZ           = 0.0;
bool   lightDetected    = false;

bool   tempAlert        = false;
bool   humidAlert       = false;
bool   shockAlert       = false;
bool   tamperAlert      = false;
bool   pressureAlert    = false;
bool   systemOK         = true;

bool   mpuAvailable     = false;
bool   bmpAvailable     = false;
bool   apdsAvailable    = false;

int    totalEvents      = 0;
unsigned long lastSensorRead = 0;
unsigned long lastOledUpdate = 0;
unsigned long bootTime       = 0;
int    oledPage         = 0;


// ============================================================
//   SECTION 4: HELPER FUNCTIONS
// ============================================================

// --- How long has the device been running? ---
String getUptime() {
  unsigned long sec = (millis() - bootTime) / 1000;
  int h = sec / 3600;
  int m = (sec % 3600) / 60;
  int s = sec % 60;
  char buf[20];
  snprintf(buf, sizeof(buf), "%dh %02dm %02ds", h, m, s);
  return String(buf);
}

// --- Timestamp from boot (we don't have a real clock) ---
String getTimestamp() {
  unsigned long sec = (millis() - bootTime) / 1000;
  int h = sec / 3600;
  int m = (sec % 3600) / 60;
  int s = sec % 60;
  char buf[16];
  snprintf(buf, sizeof(buf), "%02d:%02d:%02d", h, m, s);
  return String(buf);
}

// --- Beep buzzer ---
void buzz(int ms = 400, int times = 1) {
  for (int i = 0; i < times; i++) {
    digitalWrite(BUZZER_PIN, HIGH);
    delay(ms);
    digitalWrite(BUZZER_PIN, LOW);
    if (times > 1) delay(120);
  }
}

// --- Save one event to flash memory ---
void logEvent(String type, String detail, float val = 0.0) {
  totalEvents++;
  StaticJsonDocument<200> doc;
  doc["t"] = getTimestamp();
  doc["e"] = type;
  doc["d"] = detail;
  doc["v"] = String(val, 2);
  doc["n"] = totalEvents;
  String line;
  serializeJson(doc, line);

  File f = SPIFFS.open(LOG_FILE, FILE_APPEND);
  if (f) { f.println(line); f.close(); }
  Serial.println("[LOG] " + line);
}

// --- Read all sensors into global variables ---
void readAllSensors() {
  // AM2302 / AM2303 temperature & humidity
  auto status = am2302.read();
  if (status == AM2302::AM2302_READ_OK) {
    currentTemp     = am2302.get_Temperature();
    currentHumidity = am2302.get_Humidity();
  } else {
    Serial.println("[WARN] AM2302 read failed — check wiring on GPIO " + String(DHT_PIN));
  }

  // BMP180 pressure
  if (bmpAvailable) {
    sensors_event_t ev;
    bmp.getEvent(&ev);
    if (ev.pressure) {
      if (lastPressure == 0) lastPressure = ev.pressure;
      currentPressure = ev.pressure;
    }
  }

  // MPU6050 acceleration (Adafruit library)
  if (mpuAvailable) {
    sensors_event_t a, g, t;
    mpu.getEvent(&a, &g, &t);
    accelX = a.acceleration.x;
    accelY = a.acceleration.y;
    accelZ = a.acceleration.z;
  }

  // APDS9960 light (tamper = box opened → light enters)
  if (apdsAvailable) {
    uint16_t r, g, b, c;
    if (apds.colorDataReady()) {
      apds.getColorData(&r, &g, &b, &c);
      lightDetected = (c > 200);
    }
  }
}

// --- Check readings vs limits, raise alerts ---
void checkAlerts() {
  bool any = false;

  // Temperature
  bool newTemp = (currentTemp < TEMP_MIN || currentTemp > TEMP_MAX);
  if (newTemp && !tempAlert) {
    logEvent("TEMP_ALERT", currentTemp > TEMP_MAX ? "TOO HOT" : "TOO COLD", currentTemp);
    buzz(300, 3);
  }
  tempAlert = newTemp;
  if (tempAlert) any = true;

  // Humidity
  bool newHumid = (currentHumidity > HUMIDITY_MAX);
  if (newHumid && !humidAlert) {
    logEvent("HUMIDITY_ALERT", "HIGH HUMIDITY", currentHumidity);
    buzz(200, 2);
  }
  humidAlert = newHumid;
  if (humidAlert) any = true;

  // Shock
  if (mpuAvailable) {
    float mag = sqrt(accelX*accelX + accelY*accelY + accelZ*accelZ);
    bool newShock = (mag > SHOCK_THRESHOLD);
    if (newShock && !shockAlert) {
      logEvent("SHOCK_ALERT", "IMPACT DETECTED", mag);
      buzz(500, 2);
    }
    shockAlert = newShock;
    if (shockAlert) any = true;
  }

  // Tamper (box opened)
  if (apdsAvailable) {
    bool newTamper = lightDetected;
    if (newTamper && !tamperAlert) {
      logEvent("TAMPER_ALERT", "BOX OPENED", 1.0);
      buzz(1000, 1);
    }
    tamperAlert = newTamper;
    if (tamperAlert) any = true;
  }

  // Pressure change (altitude shift)
  if (bmpAvailable && lastPressure > 0 && currentPressure > 0) {
    float delta = abs(currentPressure - lastPressure);
    if (delta > PRESSURE_DELTA_MAX) {
      logEvent("PRESSURE_ALERT", "ALTITUDE CHANGE", delta);
      buzz(200, 1);
    }
    lastPressure = currentPressure;
  }

  systemOK = !any;
}


// ============================================================
//   SECTION 5: OLED DISPLAY
//   3 rotating pages, updates every 2 seconds
// ============================================================

void updateOLED() {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);

  if (oledPage == 0) {
    // Page 0 — main status
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print("COLD CHAIN SENTINEL");
    display.drawLine(0, 9, 127, 9, SSD1306_WHITE);
    display.setTextSize(2);
    display.setCursor(0, 14);
    if (systemOK) {
      display.println(" STATUS:");
      display.setCursor(0, 34);
      display.println("   OK!");
    } else {
      display.println("  ALERT!");
      display.setTextSize(1);
      display.setCursor(0, 36);
      if (tempAlert)   display.print("  TEMPERATURE OUT OF RANGE");
      else if (tamperAlert) display.print("  BOX OPENED - TAMPER!");
      else if (shockAlert)  display.print("  SHOCK/DROP DETECTED!");
      else if (humidAlert)  display.print("  HUMIDITY TOO HIGH!");
    }
    display.setTextSize(1);
    display.setCursor(0, 56);
    display.print("Events:"); display.print(totalEvents);
    display.print(" | 192.168.4.1");

  } else if (oledPage == 1) {
    // Page 1 — sensor readings
    display.setTextSize(1);
    display.setCursor(0,  0); display.print("---- READINGS ----");
    display.setCursor(0, 12); display.print("Temp : "); display.print(currentTemp, 1);     display.print(" C");
    display.setCursor(0, 22); display.print("Humid: "); display.print(currentHumidity, 1); display.print(" %");
    display.setCursor(0, 32); display.print("Press: "); display.print(currentPressure, 0); display.print(" hPa");
    float mag = sqrt(accelX*accelX + accelY*accelY + accelZ*accelZ);
    display.setCursor(0, 42); display.print("Accel: "); display.print(mag, 1);             display.print(" m/s2");
    display.setCursor(0, 52); display.print("Light: "); display.print(lightDetected ? "!! DETECTED !!" : "none");

  } else {
    // Page 2 — network info
    display.setTextSize(1);
    display.setCursor(0,  0); display.print("---- NETWORK ----");
    display.setCursor(0, 12); display.print("WiFi: ColdChainSentinel");
    display.setCursor(0, 22); display.print("IP  : 192.168.4.1");
    display.setCursor(0, 32); display.print("Open browser, type:");
    display.setCursor(0, 42); display.print("  192.168.4.1");
    display.setCursor(0, 52); display.print("Uptime: "); display.print(getUptime());
  }

  display.display();
  oledPage = (oledPage + 1) % 3;
}


// ============================================================
//   SECTION 6: BUILD THE WEB DASHBOARD HTML PAGE
//   This is what your phone sees when you open 192.168.4.1
// ============================================================

String buildDashboard() {
  // Read last 20 log entries from flash memory
  String rows = "";
  File f = SPIFFS.open(LOG_FILE, FILE_READ);
  if (f) {
    std::vector<String> lines;
    while (f.available()) {
      String ln = f.readStringUntil('\n');
      ln.trim();
      if (ln.length() > 2) lines.push_back(ln);
    }
    f.close();
    int start = max(0, (int)lines.size() - 20);
    for (int i = (int)lines.size() - 1; i >= start; i--) {
      StaticJsonDocument<200> doc;
      if (!deserializeJson(doc, lines[i])) {
        String ev  = doc["e"].as<String>();
        String col = ev.indexOf("TEMP")     >= 0 ? "#ff6b6b" :
                     ev.indexOf("TAMPER")   >= 0 ? "#ffd93d" :
                     ev.indexOf("SHOCK")    >= 0 ? "#ff9f43" :
                     ev.indexOf("PRESSURE") >= 0 ? "#a29bfe" :
                     ev.indexOf("SYSTEM")   >= 0 ? "#55efc4" : "#74b9ff";
        rows += "<tr><td>" + doc["t"].as<String>() + "</td>";
        rows += "<td><span style='background:" + col + ";color:#111;padding:2px 8px;"
                "border-radius:10px;font-size:11px;font-weight:bold;'>" + ev + "</span></td>";
        rows += "<td>" + doc["d"].as<String>() + "</td>";
        rows += "<td>" + doc["v"].as<String>() + "</td></tr>\n";
      }
    }
  }
  if (rows == "") rows = "<tr><td colspan='4' style='text-align:center;color:#666;padding:16px;'>"
                         "No alerts logged — everything is fine!</td></tr>";

  String sc = systemOK ? "#00b894" : "#d63031";
  String st = systemOK ? "ALL OK" : "ALERT ACTIVE";
  float  mg = sqrt(accelX*accelX + accelY*accelY + accelZ*accelZ);

  // ---- HTML starts here ----
  String h = "<!DOCTYPE html><html><head>";
  h += "<meta charset='UTF-8'>";
  h += "<meta name='viewport' content='width=device-width,initial-scale=1'>";
  h += "<meta http-equiv='refresh' content='10'>";    // auto refresh every 10s
  h += "<title>Cold Chain Sentinel</title>";
  h += "<style>";
  h += "*{box-sizing:border-box;margin:0;padding:0;}";
  h += "body{font-family:-apple-system,Arial,sans-serif;background:#0d0d1a;color:#e0e0e0;padding:14px;max-width:520px;margin:auto;}";
  h += ".hdr{text-align:center;padding:18px 0 14px;}";
  h += ".hdr h1{font-size:18px;color:#fff;letter-spacing:1px;}";
  h += ".hdr p{font-size:12px;color:#666;margin-top:3px;}";
  h += ".badge{display:inline-block;padding:7px 22px;border-radius:20px;font-weight:bold;font-size:15px;margin:10px 0;}";
  h += ".grid{display:grid;grid-template-columns:1fr 1fr;gap:10px;margin:14px 0;}";
  h += ".card{background:#161625;border-radius:12px;padding:14px;border:1px solid #2a2a45;}";
  h += ".card.ok{border-color:#00b894;}";
  h += ".card.al{border-color:#d63031;}";
  h += ".lbl{font-size:11px;color:#666;text-transform:uppercase;letter-spacing:1px;margin-bottom:5px;}";
  h += ".val{font-size:26px;font-weight:bold;color:#fff;}";
  h += ".unit{font-size:13px;color:#666;margin-left:2px;}";
  h += ".sub{font-size:11px;color:#555;margin-top:3px;}";
  h += ".sec{font-size:11px;color:#555;text-transform:uppercase;letter-spacing:1px;margin:16px 0 8px;}";
  h += "table{width:100%;border-collapse:collapse;font-size:12px;}";
  h += "th{background:#161625;color:#555;padding:8px 6px;text-align:left;font-weight:normal;border-bottom:1px solid #2a2a45;}";
  h += "td{padding:8px 6px;border-bottom:1px solid #1e1e35;vertical-align:middle;}";
  h += ".ftr{text-align:center;font-size:11px;color:#444;margin-top:18px;padding-top:12px;border-top:1px solid #1e1e35;}";
  h += "</style></head><body>";

  h += "<div class='hdr'><h1>DRUG COLD-CHAIN SENTINEL</h1>";
  h += "<p>IEEE MYOSA Event 5.0 &nbsp;|&nbsp; Team Amrut &nbsp;|&nbsp; GEC Thrissur</p>";
  h += "<div class='badge' style='background:" + sc + ";color:#fff;'>" + st + "</div></div>";

  // 4 sensor cards
  h += "<div class='grid'>";

  // Temp card
  h += "<div class='card " + String(tempAlert ? "al" : "ok") + "'>";
  h += "<div class='lbl'>Temperature</div>";
  h += "<div class='val'>" + String(currentTemp, 1) + "<span class='unit'>°C</span></div>";
  h += "<div class='sub'>Safe range: 2 – 8 °C</div></div>";

  // Humidity card
  h += "<div class='card " + String(humidAlert ? "al" : "ok") + "'>";
  h += "<div class='lbl'>Humidity</div>";
  h += "<div class='val'>" + String(currentHumidity, 1) + "<span class='unit'>%</span></div>";
  h += "<div class='sub'>Limit: below 85%</div></div>";

  // Pressure card
  h += "<div class='card " + String(pressureAlert ? "al" : "ok") + "'>";
  h += "<div class='lbl'>Pressure</div>";
  h += "<div class='val'>" + String(currentPressure, 0) + "<span class='unit'>hPa</span></div>";
  h += "<div class='sub'>Altitude monitor</div></div>";

  // Shock card
  h += "<div class='card " + String(shockAlert ? "al" : "ok") + "'>";
  h += "<div class='lbl'>Shock level</div>";
  h += "<div class='val'>" + String(mg, 1) + "<span class='unit'>m/s²</span></div>";
  h += "<div class='sub'>Limit: below 15 m/s²</div></div>";

  h += "</div>"; // end grid

  // Tamper full-width card
  h += "<div class='card " + String(tamperAlert ? "al" : "ok") + "' style='margin-bottom:14px;'>";
  h += "<div class='lbl'>Tamper status (light sensor)</div>";
  h += "<div style='font-size:15px;font-weight:bold;margin-top:6px;color:";
  h += tamperAlert ? "#d63031" : "#00b894";
  h += ";'>";
  h += tamperAlert ? "WARNING — Box was opened! Tamper event logged."
                   : "OK — Box sealed, no tamper detected.";
  h += "</div></div>";

  // Event log table
  h += "<div class='sec'>Event log (latest 20)</div>";
  h += "<div class='card' style='padding:0;overflow:hidden;'>";
  h += "<table><tr><th>Time</th><th>Event</th><th>Detail</th><th>Value</th></tr>";
  h += rows;
  h += "</table></div>";

  // Footer
  h += "<div class='ftr'>Uptime: " + getUptime();
  h += " &nbsp;|&nbsp; Total events: " + String(totalEvents);
  h += "<br>Page refreshes every 10 seconds automatically</div>";
  h += "</body></html>";

  return h;
}


// ============================================================
//   SECTION 7: JSON API  →  http://192.168.4.1/data
// ============================================================

String buildJSON() {
  StaticJsonDocument<512> doc;
  doc["temp"]        = currentTemp;
  doc["humidity"]    = currentHumidity;
  doc["pressure"]    = currentPressure;
  doc["accel_mag"]   = sqrt(accelX*accelX + accelY*accelY + accelZ*accelZ);
  doc["light"]       = lightDetected;
  doc["temp_alert"]  = tempAlert;
  doc["shock_alert"] = shockAlert;
  doc["tamper"]      = tamperAlert;
  doc["events"]      = totalEvents;
  doc["uptime"]      = getUptime();
  doc["system_ok"]   = systemOK;
  String out;
  serializeJson(doc, out);
  return out;
}


// ============================================================
//   SECTION 8: SETUP — runs ONCE when board powers on
// ============================================================

void setup() {
  Serial.begin(115200);
  Serial.println("\n\n===== COLD CHAIN SENTINEL v2.0 STARTING =====");
  bootTime = millis();

  // Buzzer
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);
  buzz(80, 2);   // startup beep beep

  // I2C bus (shared by all sensors and OLED)
  Wire.begin();

  // OLED display
  if (display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("[OK] OLED ready");
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.setTextSize(1);
    display.setCursor(0,  0); display.println("COLD CHAIN SENTINEL");
    display.setCursor(0, 14); display.println("  Starting up...");
    display.setCursor(0, 28); display.println("  Please wait...");
    display.display();
  } else {
    Serial.println("[WARN] OLED not found — check JST cable");
  }

  // AM2302 / AM2303 sensor
  // The AM2302 library initialises automatically — just wait 2 seconds
  delay(2000);
  auto status = am2302.read();
  if (status == AM2302::AM2302_READ_OK) {
    Serial.println("[OK] AM2302 ready — Temp: " + String(am2302.get_Temperature()) + "C");
    currentTemp     = am2302.get_Temperature();
    currentHumidity = am2302.get_Humidity();
  } else {
    Serial.println("[WARN] AM2302 not responding — check data wire on GPIO " + String(DHT_PIN));
    Serial.println("       Wire colors: Red=3.3V  Black=GND  Yellow=GPIO4");
  }

  // MPU6050 accelerometer (Adafruit library)
  // Scanner found it at 0x69 so we use that address directly
  if (mpu.begin(0x69)) {
    mpuAvailable = true;
    mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
    mpu.setGyroRange(MPU6050_RANGE_500_DEG);
    mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
    Serial.println("[OK] MPU6050 ready");
  } else {
    Serial.println("[WARN] MPU6050 not found — shock detection off");
  }

  // BMP180 pressure
  if (bmp.begin()) {
    bmpAvailable = true;
    Serial.println("[OK] BMP180 ready");
  } else {
    Serial.println("[WARN] BMP180 not found — pressure monitoring off");
  }

  // APDS9960 light sensor
  if (apds.begin()) {
    apdsAvailable = true;
    apds.enableColor(true);
    apds.enableProximity(true);
    Serial.println("[OK] APDS9960 ready");
  } else {
    Serial.println("[WARN] APDS9960 not found — tamper detection off");
  }

  // SPIFFS flash storage
  if (SPIFFS.begin(true)) {
    Serial.println("[OK] SPIFFS storage ready");
    // Count existing log entries
    File f = SPIFFS.open(LOG_FILE, FILE_READ);
    if (f) {
      while (f.available()) { f.readStringUntil('\n'); totalEvents++; }
      f.close();
      if (totalEvents > 0)
        Serial.println("[OK] Found " + String(totalEvents) + " previous log entries");
    }
  } else {
    Serial.println("[ERROR] SPIFFS failed! Go to Tools > Partition Scheme > Default 4MB with spiffs");
  }

  // Log that device started
  logEvent("SYSTEM_START", "Sentinel powered on", 1.0);

  // Start Wi-Fi hotspot
  WiFi.softAP(WIFI_SSID, WIFI_PASSWORD);
  Serial.println("[WIFI] Hotspot started: " + String(WIFI_SSID));
  Serial.println("[WIFI] Board IP: " + WiFi.softAPIP().toString());

  // Web server routes
  server.on("/", HTTP_GET, [](AsyncWebServerRequest* req) {
    req->send(200, "text/html", buildDashboard());
  });
  server.on("/data", HTTP_GET, [](AsyncWebServerRequest* req) {
    req->send(200, "application/json", buildJSON());
  });
  server.on("/log", HTTP_GET, [](AsyncWebServerRequest* req) {
    req->send(SPIFFS, LOG_FILE, "text/plain");
  });
  server.on("/clearlog", HTTP_GET, [](AsyncWebServerRequest* req) {
    SPIFFS.remove(LOG_FILE);
    totalEvents = 0;
    logEvent("LOG_CLEARED", "Log cleared by user", 0);
    req->send(200, "text/plain", "Log cleared successfully!");
  });
  server.begin();
  Serial.println("[OK] Web server started → http://192.168.4.1");

  // First sensor read
  readAllSensors();
  checkAlerts();

  // Show ready screen
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0,  0); display.println("  SENTINEL READY!");
  display.setCursor(0, 12); display.println("WiFi:ColdChainSentinel");
  display.setCursor(0, 22); display.println("Open: 192.168.4.1");
  display.setCursor(0, 34); display.print("Temp: "); display.print(currentTemp,1); display.println("C");
  display.setCursor(0, 44); display.print("Humid:"); display.print(currentHumidity,1); display.println("%");
  display.setCursor(0, 54); display.print("Status: "); display.println(systemOK?"ALL OK":"ALERT!");
  display.display();

  // Ready fanfare beeps
  buzz(80,1); delay(80); buzz(80,1); delay(80); buzz(160,1);

  Serial.println("===== SETUP COMPLETE — SENTINEL IS LIVE =====\n");
  Serial.println("Connect phone to WiFi: ColdChainSentinel");
  Serial.println("Then open browser and go to: http://192.168.4.1\n");
}


// ============================================================
//   SECTION 9: LOOP — runs forever
// ============================================================

void loop() {
  unsigned long now = millis();

  // Every 30 seconds — read sensors + check alerts
  if (now - lastSensorRead >= SENSOR_INTERVAL) {
    lastSensorRead = now;
    readAllSensors();
    checkAlerts();
    // Print to Serial Monitor for debugging
    Serial.print("[DATA] T="); Serial.print(currentTemp,1);
    Serial.print("C  H=");     Serial.print(currentHumidity,1);
    Serial.print("%  P=");     Serial.print(currentPressure,0);
    Serial.print("hPa  A=");   Serial.print(sqrt(accelX*accelX+accelY*accelY+accelZ*accelZ),1);
    Serial.print("m/s2  Light="); Serial.print(lightDetected?"YES":"no");
    Serial.print("  Status="); Serial.println(systemOK?"OK":"ALERT");
  }

  // Every 2 seconds — refresh OLED screen
  if (now - lastOledUpdate >= OLED_INTERVAL) {
    lastOledUpdate = now;
    updateOLED();
  }

  delay(10);
}

// ============================================================
//   UPLOAD CHECKLIST:
//
//   Before uploading go to:
//   Tools > Partition Scheme > Default 4MB with spiffs   ← MUST DO
//   Tools > Board > ESP32 Dev Module
//   Tools > Port > select your COM port
//
//   After uploading:
//   Tools > Serial Monitor > set baud to 115200
//   You will see each sensor come online one by one
//
//   Then on your phone:
//   Settings > Wi-Fi > connect to "ColdChainSentinel"
//   Open Chrome/Safari > type: 192.168.4.1
//   Your live dashboard will appear!
//
//   Dashboard pages:
//   http://192.168.4.1         ← live dashboard
//   http://192.168.4.1/data    ← raw JSON data
//   http://192.168.4.1/log     ← full event log text
//   http://192.168.4.1/clearlog ← wipe the log
// ============================================================
