# Smart-agriculture-system-with-plant-disease-prediction-using-leaf-weatness-sensor


import pypandoc

# The README content
readme_content = """# 🌱 Smart Irrigation & Plant Disease Prediction System

An **IoT-based Smart Irrigation System** using **ESP8266 (NodeMCU)**, **Blynk App**, and **Google Sheets integration**.  
The project automates irrigation based on **soil moisture, rainfall detection, and weather data** while also predicting potential **plant diseases** using environmental parameters.

---

## 🚀 Features
- ✅ **Automatic Irrigation Control** based on soil moisture and weather conditions  
- ✅ **Manual Override via Blynk App** for irrigation, humidity, and weather modes  
- ✅ **Real-time Weather Data** fetched from OpenWeather API  
- ✅ **Disease Prediction** for crops like Tomato, Potato, Rice, Wheat, Chili, and Grapes  
- ✅ **Web Dashboard** hosted on ESP8266 for monitoring & plant type selection  
- ✅ **LCD Display Support** for live updates on soil moisture, irrigation status, and weather  
- ✅ **Google Sheets Integration** for logging sensor data & predictions  

---

## 🛠️ Hardware Requirements
- ESP8266 NodeMCU  
- Soil Moisture Sensor  
- Raindrop Sensor  
- Relay Module + Water Pump  
- 16x2 LCD with I2C  
- Power Supply (5V)  

---

## 🔌 Circuit Connections
| Component             | ESP8266 Pin |
|-----------------------|-------------|
| Soil Moisture Sensor  | A0 (via D5 power pin) |
| Raindrop Sensor       | A0 (via D6 power pin) |
| LCD (I2C)             | SDA → D2, SCL → D1 |
| Relay Module          | D3 |
| Wi-Fi Connection      | SSID & Password in code |

---

## 📊 Data Flow
1. **Sensors** collect soil moisture & rainfall data.  
2. **ESP8266** processes inputs and fetches live weather data.  
3. **Decision Engine**:
   - Irrigation ON/OFF based on soil & humidity thresholds  
   - Plant disease prediction based on crop type, wetness duration, temperature, and humidity  
4. **Data Logging** → Sent to Google Sheets for history tracking  
5. **User Interaction** → Control & monitor via **Blynk App** or **Web Portal**  

---

## 🌦️ Supported Plant Disease Predictions
| Plant   | Disease           | Conditions (Duration, Temp, Humidity) |
|---------|------------------|---------------------------------------|
| Tomato  | Late Blight      | 9–15 hrs wetness, 20–25°C, 55–75% |
| Potato  | Early Blight     | 12–15 hrs wetness, 20–22°C, 50–60% |
| Rice    | Rice Blast       | 9–12 hrs wetness, 20–28°C, 90–100% |
| Wheat   | Leaf Rust        | 6–8 hrs wetness, 15–22°C, 80–100% |
| Chili   | Bacterial Wilt   | 4–12 hrs wetness, 22–30°C, 80–95% |
| Grape   | Downy Mildew     | 1–3 hrs wetness, 20–25°C, ≥95% |

---

## 📱 Blynk App Integration
- Displays real-time sensor values (soil moisture, weather, irrigation status)  
- Manual control of irrigation system  
- Event logging for weather alerts  

---

## 📡 Google Sheets Logging
The system uploads:  
- Soil Moisture Status  
- Irrigation Duration  
- Weather Data (Temp, Humidity, Description)  
- Plant Type & Predicted Disease  

This allows historical tracking and analysis.

---

## 🖥️ Web Portal Features
- Hosted on ESP8266 at local IP (check serial monitor for address)  
- Plant type selection dropdown  
- Last transmitted data summary  
- Disease prediction result  

---
