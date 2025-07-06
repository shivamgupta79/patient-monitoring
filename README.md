# 🩺 Real-Time Patient Monitoring and BLE Indoor Tracking System (ESP32 + ThingsBoard + n8n)

A real-time patient health monitoring and indoor location tracking system built with **ESP32**, **MAX30100**, **DS18B20**, **BLE Beacons**, **ThingsBoard** for dashboards, and **n8n** for automation and emergency alerts (e.g., Telegram, WhatsApp).

---

## 📦 Components Used

| Component                | Description                         |
|--------------------------|-------------------------------------|
| ESP32 Dev Module         | Wi-Fi & BLE capable microcontroller |
| MAX30100 Sensor          | Heart rate & SpO₂ sensor            |
| DS18B20                  | Digital temperature sensor          |
| Push Button              | Manual emergency trigger            |
| LED + Buzzer             | Emergency indicator                 |
| BLE Tag / iBeacon        | For patient indoor location tracking |

---

## 🔌 Pin Configuration

| ESP32 Pin  | Component             |
|------------|-----------------------|
| GPIO 4     | DS18B20 Data          |
| GPIO 16    | Buzzer                |
| GPIO 17    | LED                   |
| GPIO 18    | Emergency Push Button |
| GPIO 21/22 | I2C for MAX30100      |

> ⚠️ Use pull-up resistor for DS18B20 and ensure MAX30100 uses default I2C pins.

---

## 🧠 Features

- ✅ Real-time body vitals monitoring: Temperature, Heart Rate, SpO₂
- 🛜 Sends data to ThingsBoard dashboard for visualization
- 🧭 Indoor location tracking using BLE beacon RSSI
- 📤 Sends all data to `n8n` for automation
- 🚨 Emergency alert system with LED + buzzer
- 📲 Send alerts to **Telegram**/**WhatsApp** via `n8n`

---

## 🌐 Data Sent Format

```json
{
  "temperature": 36.6,
  "heartrate": 72.5,
  "spo2": 97,
  "emergency": false,
  "rssi": -62,
  "location": "Room A"
}
📊 ThingsBoard Widgets Setup
Widget Type	Key	Suggested Range
Gauge	temperature	30 – 45 °C
Gauge	heartrate	40 – 130 bpm
Gauge	spo2	80 – 100 %
Control LED	emergency	0 = OFF, 1 = ON
Label Value	location	Room A / Near Door...
Time-series Chart	All	Historical Data

💡 Add a widget rule to color temperature red if > 38°C.

🚦 BLE Location Logic
RSSI Range	Location
> -65 dBm	Room A
-65 to -80 dBm	Near Door
< -80 dBm	Far Zone
Not detected	Not Detected

⚙️ Setup Instructions
1️⃣ Arduino Library Requirements
Install these from Library Manager:

MAX30100_PulseOximeter

OneWire

DallasTemperature

NimBLE-Arduino by H2zero

2️⃣ ThingsBoard Device
Create a device on https://demo.thingsboard.io

Replace your access token in the code:

cpp
Copy
Edit
const char* tbAccessToken = "your-token";
3️⃣ n8n Setup
Create a Webhook node at /webhook/patient-data

Connect it to logic (IF + Telegram or WhatsApp nodes)

Paste webhook URL in code:

cpp
Copy
Edit
const char* n8nWebhook = "https://your-n8n-url.com/webhook/patient-data";
4️⃣ BLE Beacon Address
Replace with your tag's MAC address:

cpp
Copy
Edit
String knownBeaconAddress = "D0:39:72:BF:9F:60";
🚀 How to Use
Connect sensors as per pin diagram.

Upload code to ESP32 via Arduino IDE.

Open Serial Monitor to see logs.

Open ThingsBoard dashboard to visualize live data.

Configure n8n to send alerts on:

High temp (>38°C)

Emergency button press

BLE zone = "Far" or "Not Detected"
Dashboard
![image](https://github.com/user-attachments/assets/2df457f2-0666-483d-9e19-8e99bcc459e2)
