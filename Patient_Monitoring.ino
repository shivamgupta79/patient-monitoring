#include <WiFi.h>
#include <HTTPClient.h>
#include <Wire.h>
#include "MAX30100_PulseOximeter.h"
#include <OneWire.h>
#include <DallasTemperature.h>
#include <NimBLEDevice.h>

// Wi-Fi credentials
const char* ssid = "Tute";
const char* password = "dknx1909";

// ThingsBoard Info
const char* thingsboardHost = "http://demo.thingsboard.io";
const char* tbAccessToken = "wicks79r3gDDd26ccsgd";

// n8n Webhook
const char* n8nWebhook = "https://your-n8n-url.com/webhook/patient-data";  // Replace with your URL

// Sensor pin definitions
#define ONE_WIRE_BUS 4
#define BUTTON_PIN 18
#define BUZZER_PIN 16
#define LED_PIN 17

// BLE Beacon MAC Address (Change this to your BLE tag address)
String knownBeaconAddress = "D0:39:72:BF:9F:60";
int lastRSSI = -100;
String location = "Unknown";

// Sensor objects
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature tempSensor(&oneWire);
PulseOximeter pox;

uint32_t lastSendTime = 0;
#define REPORTING_INTERVAL 2000

// 🔁 Send "hi" to dashboard once on startup
void sendInitialStatusToThingsBoard() {
  HTTPClient http;
  String serverPath = String(thingsboardHost) + "/api/v1/" + tbAccessToken + "/telemetry";

  http.begin(serverPath);
  http.addHeader("Content-Type", "application/json");

  String payload = "{\"status\":\"hi\"}";

  int httpCode = http.POST(payload);
  if (httpCode == 200 || httpCode == 204) {
    Serial.println("✅ Connected to Dashboard");
  } else {
    Serial.print("⚠️ Dashboard init send failed, code: ");
    Serial.println(httpCode);
  }

  http.end();
}

// BLE scanning function
void scanBLE() {
  NimBLEScan* pBLEScan = NimBLEDevice::getScan();
  NimBLEScanResults results = pBLEScan->start(2, false); // 2 seconds scan

  lastRSSI = -100;
  location = "Not Detected";

  for (int i = 0; i < results.getCount(); i++) {
    NimBLEAdvertisedDevice device = results.getDevice(i);
    if (device.getAddress().toString() == knownBeaconAddress) {
      lastRSSI = device.getRSSI();

      if (lastRSSI > -65) {
        location = "Room A";
      } else if (lastRSSI > -80) {
        location = "Near Door";
      } else {
        location = "Far";
      }
      break;
    }
  }
}

// Send sensor data to ThingsBoard
void sendToThingsBoard(float temperature, float heartRate, float spo2, bool emergency) {
  HTTPClient http;
  String serverPath = String(thingsboardHost) + "/api/v1/" + tbAccessToken + "/telemetry";

  http.begin(serverPath);
  http.addHeader("Content-Type", "application/json");

  String payload = "{";
  payload += "\"temperature\":" + String(temperature) + ",";
  payload += "\"heartrate\":" + String(heartRate) + ",";
  payload += "\"spo2\":" + String(spo2) + ",";
  payload += "\"emergency\":" + String(emergency ? 1 : 0) + ",";
  payload += "\"rssi\":" + String(lastRSSI) + ",";
  payload += "\"location\":\"" + location + "\"";
  payload += "}";

  int httpCode = http.POST(payload);
  Serial.print("ThingsBoard Response: ");
  Serial.println(httpCode);
  http.end();
}

// Send sensor data to n8n
void sendToN8n(float temperature, float heartRate, float spo2, bool emergency) {
  HTTPClient http;
  http.begin(n8nWebhook);
  http.addHeader("Content-Type", "application/json");

  String payload = "{";
  payload += "\"temperature\":" + String(temperature) + ",";
  payload += "\"heartrate\":" + String(heartRate) + ",";
  payload += "\"spo2\":" + String(spo2) + ",";
  payload += "\"emergency\":" + String(emergency ? 1 : 0) + ",";
  payload += "\"rssi\":" + String(lastRSSI) + ",";
  payload += "\"location\":\"" + location + "\"";
  payload += "}";

  int httpCode = http.POST(payload);
  Serial.print("n8n Response: ");
  Serial.println(httpCode);
  http.end();
}

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);

  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);

  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" connected!");

  tempSensor.begin();

  if (!pox.begin()) {
    Serial.println("❌ Failed to initialize MAX30100. Check wiring!");
    while (1);
  }

  pox.setIRLedCurrent(MAX30100_LED_CURR_7_6MA);

  NimBLEDevice::init("");  // Start BLE scanner

  sendInitialStatusToThingsBoard();
}

void loop() {
  pox.update();
  tempSensor.requestTemperatures();

  scanBLE();  // Update BLE location

  float temperature = tempSensor.getTempCByIndex(0);
  float heartRate = pox.getHeartRate();
  float spo2 = pox.getSpO2();
  bool emergency = digitalRead(BUTTON_PIN) == LOW;

  if (millis() - lastSendTime > REPORTING_INTERVAL) {
    lastSendTime = millis();

    Serial.printf("Sending -> Temp: %.2f°C, HR: %.2f bpm, SpO2: %.2f%%, Emergency: %s, RSSI: %d, Location: %s\n",
                  temperature, heartRate, spo2, emergency ? "YES" : "NO", lastRSSI, location.c_str());

    if (WiFi.status() == WL_CONNECTED) {
      sendToThingsBoard(temperature, heartRate, spo2, emergency);
      sendToN8n(temperature, heartRate, spo2, emergency);
    }

    if (emergency) {
      digitalWrite(BUZZER_PIN, HIGH);
      digitalWrite(LED_PIN, HIGH);
      delay(3000);
      digitalWrite(BUZZER_PIN, LOW);
      digitalWrite(LED_PIN, LOW);
    }
  }
}

