#define BLYNK_TEMPLATE_ID "TMPL3wYccwbrs"
#define BLYNK_TEMPLATE_NAME "SMART IRRIGATION SYSTEM"
#define BLYNK_AUTH_TOKEN "rBnaVFU2wvEpXp28qaUOtNcgXEaTGot6"

#include <Wire.h>
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <ESP8266WebServer.h>
#include <WiFiClientSecure.h>
#include <LiquidCrystal_I2C.h>



#define BLYNK_PRINT Serial

LiquidCrystal_I2C lcd(0x27, 16, 2); 

// WiFi Credentials
char auth[] = "rBnaVFU2wvEpXp28qaUOtNcgXEaTGot6";
char ssid[] = "Rohan"; 
char password[] = "i523m1bm"; 

// Google Script URL
String googleScriptURL = "https://script.google.com/macros/s/AKfycbyo232_BiX2H2MiPPtRYy_IVmOTyhlOVaB1bOnXXZqv1SLjol2Zxs1FSW3ZmWw0gesD/exec";
//https://script.google.com/macros/s/AKfycbyo232_BiX2H2MiPPtRYy_IVmOTyhlOVaB1bOnXXZqv1SLjol2Zxs1FSW3ZmWw0gesD/exec

// Weather API Credentials
String URL = "http://api.openweathermap.org/data/2.5/weather?";
String ApiKey = "d83478b8e33fccf86e6e4decfe209e7c";  // Replace with your OpenWeather API key
String lat = "12.97962688264605";          // Replace with your latitude
String lon = "77.52593235423015";          // Replace with your longitude

// Sensor configuration
//const int sensorPin = A0;
const int wetnessThreshold = 500;
// Variables
unsigned long previousWeatherUpdate = 0;
const unsigned long weatherUpdateInterval = 60000; // 60 seconds
bool isWet = false;
unsigned long wetStartTime = 0;
unsigned long currentWetTime = 0;
unsigned long wetDurationMinutes = 0;
unsigned long chunkStartTime = 0; // Start time for the current 8-min chunk

// Variables for chunk-based averaging
float chunkTemperatureTotal = 0.0;
float chunkHumidityTotal = 0.0;
int chunkCount = 0; // Number of readings in the current chunk

// Variables for overall averages
float overallTemperatureTotal = 0.0;
float overallHumidityTotal = 0.0;
int overallChunkCount = 0; // Number of chunks processed

// Sensor configuration
#define SOIL_MOISTURE_PIN_POWER D5  // Digital pin to power soil moisture sensor
#define SENSOR_PIN_POWER D6       // Digital pin to power raindrop sensor
#define SENSOR_PIN A0  
#define SDA_PIN D2
#define SCL_PIN D1

String weatherDescription = "";
float temperature = 0.0;
int humidity = 0;
float totalTemp = 0.0;
float totalHumidity = 0.0;
int countReadings = 0;
 float avgTemp = 0.0;
 int avgHumidity = 0;
bool isIrrigating = false;
bool weatherOverride = false;

BlynkTimer timer;
int manualControlValue = 0; // Variable to store manual control value
int manualWeatherMode = 0;  // Variable to store manual weather mode status
int manualHumidityMode = 0; // Variable to store manual humidity mode status
unsigned long manualControlStartTime = 0; // Variable to store the time when manual control was activated


// Web Server Initialization
ESP8266WebServer server(80);

// Selected Plant Type
String plantType = "Unknown";

// Disease Prediction Result
String predictedDisease = "";

WiFiClientSecure client;
HTTPClient http;

// Function to predict disease
String predictDisease(String plantType, float wetnessDuration, float temperature, int humidity) {
  if (plantType == "Tomato" && wetnessDuration >= 9 && wetnessDuration <= 15 &&
      temperature >= 20.0 && temperature <= 25.0 && humidity >= 55 && humidity <= 75) {
    return "Late Blight";
  }
  if (plantType == "Potato" && wetnessDuration >= 12 && wetnessDuration <= 15 &&
      temperature >= 20.0 && temperature <= 22.0 && humidity >= 50 && humidity <= 60) {
    return "Early Blight";
  }
  // Rice disease prediction
  if (plantType == "Rice" && wetnessDuration >= 540 && wetnessDuration <= 720 &&
      temperature >= 20.0 && temperature <= 28.0 && humidity >= 90 && humidity <= 100) {
    return "Rice Blast";
  }

  // Wheat disease prediction
  if (plantType == "Wheat" && wetnessDuration >= 360 && wetnessDuration <= 480 &&
      temperature >= 15.0 && temperature <= 22.0 && humidity >= 80 && humidity <= 100) {
    return "Leaf Rust";
  }
  if (plantType == "Chili" && wetnessDuration >= 240 && wetnessDuration <= 720 &&
      temperature >= 22.0 && temperature <= 30.0 && humidity >= 80 && humidity <= 95) {
    return "Bacterial Wilt";
  }
if (plantType == "Grape" && wetnessDuration >= 60 && wetnessDuration <= 180 &&
      temperature >= 20.0 && temperature <= 25.0 && humidity >= 95) {
    return "Downy Mildew";
  }
  return "No Disease Detected";
}

// Web Portal: Root Page
void handleRoot() {
  String html = "<!DOCTYPE html><html lang='en'>";
  html += "<head>";
  html += "<meta charset='UTF-8'>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>";
  html += "<title>Plant Disease Prediction</title>";
  html += "<style>";
  html += "body { font-family: 'Arial', sans-serif; margin: 0; padding: 0; background: linear-gradient(to bottom, #b6e388, #4caf50); color: #333; }";
  html += ".header { text-align: center; padding: 20px; animation: fadeInDown 1s ease-in-out; }";
  html += ".header img { width: 120px; margin-bottom: 10px; }";
  html += ".header h1 { color: #fff; font-size: 2.5em; margin: 10px 0; }";
  html += ".form-container { max-width: 400px; margin: 50px auto; background: #fff; padding: 30px; border-radius: 10px; box-shadow: 0 4px 8px rgba(0,0,0,0.2); animation: fadeInUp 1.5s ease-in-out; }";
  html += ".form-container h2 { margin-bottom: 20px; color: #4caf50; }";
  html += "select, input[type='submit'] { width: 100%; padding: 10px; margin-top: 10px; border: 1px solid #ccc; border-radius: 5px; font-size: 1em; }";
  html += "input[type='submit'] { background: #4caf50; color: #fff; font-weight: bold; cursor: pointer; transition: background 0.3s ease; }";
  html += "input[type='submit']:hover { background: #388e3c; }";
  html += ".data-container { max-width: 600px; margin: 50px auto; background: #f9f9f9; padding: 20px; border-radius: 10px; box-shadow: 0 4px 8px rgba(0,0,0,0.2); animation: fadeInUp 1.5s ease-in-out; }";
  html += ".data-container h3 { color: #4caf50; text-align: center; }";
  html += "table { width: 100%; border-collapse: collapse; margin-top: 10px; }";
  html += "table, th, td { border: 1px solid #ddd; }";
  html += "th, td { padding: 10px; text-align: center; }";
  html += "th { background-color: #4caf50; color: #fff; }";
  html += "@keyframes fadeInDown { from { opacity: 0; transform: translateY(-20px); } to { opacity: 1; transform: translateY(0); } }";
  html += "@keyframes fadeInUp { from { opacity: 0; transform: translateY(20px); } to { opacity: 1; transform: translateY(0); } }";
  html += ".footer { text-align: center; margin-top: 50px; font-size: 0.9em; color: #fff; animation: fadeIn 2s ease-in-out; }";
  html += "</style>";
  html += "</head>";
  html += "<body>";
  html += "<div class='header'>";
  
  // SVG Logo
  html += "<img src='data:image/svg+xml;utf8,";
  html += "<svg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"0 0 64 64\" width=\"120\" height=\"120\">";
  html += "<circle cx=\"32\" cy=\"32\" r=\"32\" fill=\"#4CAF50\"/>";
  html += "<path d=\"M32 12C24.82 12 19 17.82 19 25c0 5.5 3.28 10.26 8 12.48V49h-4v3h14v-3h-4V37.48c4.72-2.22 8-6.98 8-12.48 0-7.18-5.82-13-13-13z\" fill=\"#fff\"/>";
  html += "</svg>";
  html += "' alt='Plant Logo'>";
  
  html += "<h1>Plant Disease Prediction</h1>";
  html += "</div>";
  html += "<div class='form-container'>";
  html += "<h2>Select Plant Type</h2>";
  html += "<form action='/setPlantType' method='GET'>";
  html += "<select name='plant'>";
  html += "<option value='Tomato'>Tomato</option>";
  html += "<option value='Potato'>Potato</option>";
  html += "<option value='Rice'>Rice</option>";
  html += "<option value='Wheat'>Wheat</option>";
  html += "<option value='Chili'>Chili</option>";
  html += "<option value='Grape'>Grape</option>";
  html += "</select>";
  html += "<br><br>";
  html += "<input type='submit' value='Submit'>";
  html += "</form>";
  html += "</div>";

  // Display Sent Data Section
  html += "<div class='data-container'>";
  html += "<h3>Last Sent Data</h3>";
  html += "<table>";
  html += "<tr><th>Status</th><th>Duration (min)</th><th>Weather</th><th>Temp (°C)</th><th>Humidity (%)</th><th>Disease</th><th>Plant</th></tr>";
  html += "<tr>";
  html += "<td>" + String(isWet ? "Wet" : "Dry") + "</td>";
  html += "<td>" + String(wetDurationMinutes) + "</td>";
  html += "<td>" + weatherDescription + "</td>";
  html += "<td>" + String(temperature) + "</td>";
  html += "<td>" + String(humidity) + "</td>";
  html += "<td>" + predictedDisease + "</td>";
  html += "<td>" + plantType + "</td>";
  html += "</tr>";
  html += "</table>";
  html += "</div>";

  html += "<div class='footer'>© 2024 Smart Agriculture System</div>";
  html += "</body></html>";
  server.send(200, "text/html", html);
}


// Web Portal: Set Plant Type
void handleSetPlantType() {
  if (server.hasArg("plant")) {
    plantType = server.arg("plant");
    Serial.println("Plant Type Selected: " + plantType);
    String html = "<!DOCTYPE html><html lang='en'>";
    html += "<head>";
    html += "<meta charset='UTF-8'>";
    html += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>";
    html += "<title>Plant Type Set</title>";
    html += "<style>";
    html += "body { text-align: center; font-family: Arial, sans-serif; margin: 0; padding: 0; background: #e8f5e9; color: #333; }";
    html += "h2 { color: #388E3C; margin-top: 50px; animation: fadeIn 1.5s ease-in-out; }";
    html += "a { display: inline-block; margin-top: 30px; text-decoration: none; background: #4CAF50; color: white; padding: 10px 20px; border-radius: 5px; font-size: 1em; transition: background 0.3s; }";
    html += "a:hover { background: #388E3C; }";
    html += "@keyframes fadeIn { from { opacity: 0; } to { opacity: 1; } }";
    html += "</style></head>";
    html += "<body>";
    html += "<h2>Plant Type Set to: " + plantType + "</h2>";
    html += "<a href='/'>Back to Home</a>";
    html += "</body></html>";
    server.send(200, "text/html", html);
  } else {
    server.send(400, "text/html", "<h2>Plant Type Not Selected!</h2><a href='/'>Back</a>");
  }
}



void setup() {
   pinMode(SENSOR_PIN, INPUT);
  pinMode(SENSOR_PIN_POWER, OUTPUT);
  pinMode(SOIL_MOISTURE_PIN_POWER, OUTPUT);
  pinMode(D3, OUTPUT); 
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  Blynk.begin(auth, ssid, password);
  Wire.begin(SDA_PIN, SCL_PIN); // Initialize I2C with specified SDA and SCL pins
  lcd.init();                   // Initialize the LCD
  lcd.backlight();  
  timer.setInterval(2500L, sendSensor);

  // Wait for the Wi-Fi connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Wi-Fi connected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
  // Disable SSL verification (use cautiously)
  client.setInsecure();

 server.on("/", handleRoot);
 server.on("/setPlantType", handleSetPlantType);
 server.begin();
  Serial.println("Web Server Started!");



}
//    sendDataToGoogleSheets("Dry", wetDurationMinutes, weatherDescription, avgTemp, avgHumidity, predictedDisease, plantType);
// Send Data to Google Sheets
void sendDataToGoogleSheets(String status, unsigned long duration, String weatherDescription, float temperature, int humidity,String predictedDisease, String plantType) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    WiFiClientSecure client;
    client.setInsecure(); // Bypass SSL certificate verification

    // URL encode function to replace special characters
    String url = googleScriptURL + "?sts=write"
              + "&status=" + urlencode(status)
              + "&duration=" + String(duration)
              + "&weather=" + urlencode(weatherDescription)
              + "&temp=" + String(temperature)
              + "&humidity=" + String(humidity)
              + "&predictedDisease=" + urlencode(predictedDisease)
              + "&plantType=" + urlencode(plantType);

    Serial.print("Request URL: ");
    Serial.println(url);

    http.setFollowRedirects(HTTPC_FORCE_FOLLOW_REDIRECTS);

    // Initialize HTTP request and start the connection
    if (http.begin(client, url)) {
      int httpResponseCode = http.GET();  // Make GET request
      Serial.print("HTTP Response Code: ");
      Serial.println(httpResponseCode);

      // Check if the request was successful
      if (httpResponseCode > 0) {
        String payload = http.getString();  // Get the response payload
      } else {
        Serial.println("Error: " + http.errorToString(httpResponseCode));
      }
      http.end();  // End the HTTP session
    } else {
      Serial.println("HTTPClient initialization failed!");
    }
  } else {
    Serial.println("WiFi not connected!");
  }
}

// Helper function to URL encode special characters in the parameters
String urlencode(String str) {
  String encoded = "";
  char c;
  for (unsigned int i = 0; i < str.length(); i++) {
    c = str.charAt(i);
    if (c == ' ') encoded += "%20";
    else if (c == '!') encoded += "%21";
    else if (c == '"') encoded += "%22";
    else if (c == '#') encoded += "%23";
    else if (c == '$') encoded += "%24";
    else if (c == '%') encoded += "%25";
    else if (c == '&') encoded += "%26";
    else if (c == '\'') encoded += "%27";
    else if (c == '(') encoded += "%28";
    else if (c == ')') encoded += "%29";
    else if (c == '*') encoded += "%2A";
    else if (c == '+') encoded += "%2B";
    else if (c == ',') encoded += "%2C";
    else if (c == '/') encoded += "%2F";
    else if (c == ':') encoded += "%3A";
    else if (c == ';') encoded += "%3B";
    else if (c == '<') encoded += "%3C";
    else if (c == '=') encoded += "%3D";
    else if (c == '>') encoded += "%3E";
    else if (c == '?') encoded += "%3F";
    else if (c == '@') encoded += "%40";
    else if (c == '[') encoded += "%5B";
    else if (c == '\\') encoded += "%5C";
    else if (c == ']') encoded += "%5D";
    else if (c == '^') encoded += "%5E";
    else if (c == '_') encoded += "%5F";
    else if (c == '`') encoded += "%60";
    else if (c == '{') encoded += "%7B";
    else if (c == '|') encoded += "%7C";
    else if (c == '}') encoded += "%7D";
    else if (c == '~') encoded += "%7E";
    else encoded += c;
  }
  return encoded;
}


void sendSensor() {
    int soilMoisture = readSoilMoisture();  
    soilMoisture = map(soilMoisture, 0, 920, 0, 100);
    soilMoisture = (soilMoisture - 100) * -1;

    if (manualControlValue == 1) {
        digitalWrite(D3, HIGH); 
        if (millis() - manualControlStartTime >= 3000) {
            digitalWrite(D3, LOW); 
            manualControlValue = 0; 
            Blynk.virtualWrite(V13, manualControlValue);
            Serial.println("Manual control deactivated, irrigation off.");
            isIrrigating = false;
        }
    } 
    // Soil Moisture Based Control (only if weatherOverride is false)
    else if (!weatherOverride && soilMoisture < 50 && manualHumidityMode == 0 && !isIrrigating) {
        digitalWrite(D3, HIGH);
        Serial.println("Irrigation started based on soil moisture.");
        isIrrigating = true;
    } 
    else if (soilMoisture >= 50 && isIrrigating) {
        digitalWrite(D3, LOW);
        Serial.println("Irrigation stopped based on soil moisture.");
        isIrrigating = false;
    }

    Blynk.virtualWrite(V5, soilMoisture);
    Serial.print("Soil Moisture Data: ");
    Serial.println(soilMoisture);

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("M:");
    lcd.setCursor(3, 0);
    lcd.print(soilMoisture);
    lcd.setCursor(8, 0);
    lcd.print("P:");
    lcd.setCursor(11, 0);
    lcd.print(manualControlValue == 1 ? "ON" : (digitalRead(D3) == HIGH ? "ON" : "OFF"));

    // Update weather info on LCD
    updateWeather();
}

void weatherBasedIrrigation(String weatherDescription, float humidity, float temperature) {
    if (weatherDescription.equals("clear sky")) {
        String message = String(weatherDescription) + " " + String(temperature) + "C," + String(humidity) + " %";
        for (int i = 0; i < message.length() - 15; i++) {
            lcd.setCursor(0, 1);
            lcd.print(message.substring(i, i + 16));
            delay(500);
        }

        int soilMoisture = readSoilMoisture();
        if (humidity < 30.0 && map(soilMoisture, 0, 920, 0, 100) > 50 && !isIrrigating) {
            Serial.println("Low humidity detected, starting irrigation.");
            startIrrigation();
            isIrrigating = true;
            weatherOverride = true;  // Weather takes control
        }
    } 
    else if (humidity >= 90.0 && isIrrigating) {
        Serial.println("High humidity detected, stopping irrigation.");
        stopIrrigation();
        isIrrigating = false;
        weatherOverride = true; // Weather takes control
    }
    else if (humidity <= 90.0 && !isIrrigating) {
        weatherOverride = false; // Release weather control
    }
}

void startIrrigation() {
    digitalWrite(D3, HIGH);
    Serial.println("Irrigation started.");
}

void stopIrrigation() {
    digitalWrite(D3, LOW);
    Serial.println("Irrigation stopped.");
}

/*
void startIrrigation() {
    Serial.println("Starting irrigation system.");
    digitalWrite(D3, HIGH);
    isIrrigating = true;
}

void stopIrrigation() {
    Serial.println("Stopping irrigation system.");
    digitalWrite(D3, LOW);
    isIrrigating = false;
}
*/

// Virtual pin handler for manual control
BLYNK_WRITE(V13) {
  manualControlValue = param.asInt();
  if (manualControlValue == 1) {
    manualControlStartTime = millis();
    Serial.println("Manual control activated.");
  }
}

// Virtual pin handler for manual weather mode control
BLYNK_WRITE(V14) {
  manualWeatherMode = param.asInt();
  Serial.println("Manual weather mode updated.");
}

// Virtual pin handler for manual humidity mode control
BLYNK_WRITE(V15) {
  manualHumidityMode = param.asInt();
  Serial.println("Manual humidity mode updated.");
}

// Fetch Weather Data
void updateWeather() {
  if (WiFi.status() == WL_CONNECTED) {
    WiFiClientSecure client;
    client.setInsecure();  // Allow insecure connection for HTTPS

    HTTPClient http;
   


 if (manualWeatherMode == 1) {
      Serial.println("Using Manual Weather Mode");
      // Use predefined weather conditions and humidity levels
      String weatherDescription = "clear sky"; // Change this to simulate different weather conditions
      float temperature = 25.0; // Change this to simulate different temperatures
     // float humidity = 25.0; // Change this to simulate different humidity levels
      float humidity = (manualHumidityMode == 1) ? 95.0 : 25.0; // Change humidity based on manual mode
      // Call weather-based irrigation logic
      weatherBasedIrrigation(weatherDescription, humidity, temperature);
 }else {
 String weatherApiURL = "https://api.openweathermap.org/data/2.5/weather?lat=" + lat + "&lon=" + lon + "&units=metric&appid=" + ApiKey;

    Serial.println("Fetching Weather Data from API: " + weatherApiURL);

    http.begin(client, weatherApiURL);  // Initialize HTTP connection
    http.setTimeout(10000);  // Timeout set to 10 seconds

 
    int httpCode = http.GET();  // Make the GET request
    if (httpCode > 0) {
      String payload = http.getString();
      Serial.print("HTTP Response Code: ");
      Serial.println(httpCode);
      Serial.println("Response Payload: " + payload);

      // Parse the JSON response
      DynamicJsonDocument doc(1024);
      deserializeJson(doc, payload);
      
      weatherDescription = doc["weather"][0]["description"].as<String>();
      temperature = doc["main"]["temp"].as<float>();
      humidity = doc["main"]["humidity"].as<int>();

      String message = String(weatherDescription) + " " + String(temperature) + "C," + String(humidity) + " %";
        
      // Scroll the message
      for (int i = 0; i < message.length() - 15; i++) {
        //lcd.clear();
        lcd.setCursor(0, 1);
        lcd.print(message.substring(i, i + 16)); // Display 16 characters of the message
          Blynk.logEvent("weather", message);
        delay(500); // Adjust the delay to control the scrolling speed

        weatherBasedIrrigation(weatherDescription, humidity, temperature);
      
      }

      // Accumulate readings for averaging
      totalTemp += temperature;
      totalHumidity += humidity;
      countReadings++;

      // Print the weather data to serial
      Serial.println("Weather Data Updated:");
      Serial.print("Description: ");
      Serial.println(weatherDescription);
      Serial.print("Temperature: ");
      Serial.print(temperature);
      Serial.println("°C");
      Serial.print("Humidity: ");
      Serial.print(humidity);
      Serial.println("%");
    } else {
      
        Serial.println("Error!");
        lcd.setCursor(0, 1);
        lcd.print("Can't Get DATA!");
        delay(2000);
      Serial.print("Error fetching weather data: ");
      Serial.println(http.errorToString(httpCode));
    }

    http.end();
 }
}
  }
  
void loop() {
  Blynk.run();
  timer.run();
  server.handleClient();
  int raindrop = readRaindropSensor();
 int soilMoisture = readSoilMoisture();
  Serial.print("Sensor Value: ");
  Serial.println(raindrop);
     // float avgTemp = overallTemperatureTotal / overallChunkCount;
     // int avgHumidity = overallHumidityTotal / overallChunkCount;

  if (raindrop <= wetnessThreshold) {
    if (!isWet) {
      wetStartTime = millis();
      isWet = true;

      sendDataToGoogleSheets("wet", wetDurationMinutes, weatherDescription, avgTemp, avgHumidity, predictedDisease, plantType);
     
      wetStartTime = millis();
      chunkStartTime = millis(); // Initialize chunk timer
      Serial.println("Wetness detected. Timer started.");
    }

    
    // Update weather data
    updateWeather();

    // Add to the current chunk's totals
    chunkTemperatureTotal += temperature;
    chunkHumidityTotal += humidity;
    chunkCount++;

    // Check if 8 minutes (480,000 ms) have passed
    if (millis() - chunkStartTime >= 480000) {
      // Calculate chunk averages
      float chunkTemperatureAvg = chunkTemperatureTotal / chunkCount;
      float chunkHumidityAvg = chunkHumidityTotal / chunkCount;

      // Update overall totals
      overallTemperatureTotal += chunkTemperatureAvg;
      overallHumidityTotal += chunkHumidityAvg;
      overallChunkCount++;

      // Display chunk averages
      Serial.println("8-minute chunk completed.");
      Serial.print("Chunk Avg Temperature: ");
      Serial.print(chunkTemperatureAvg);
      Serial.println(" °C");
      Serial.print("Chunk Avg Humidity: ");
      Serial.print(chunkHumidityAvg);
      Serial.println(" %");

      // Reset chunk variables
      chunkTemperatureTotal = 0.0;
      chunkHumidityTotal = 0.0;
      chunkCount = 0;
      chunkStartTime = millis(); // Start a new chunk
    }
  } else {
    if (isWet) {
      
   
     isWet = false;
      currentWetTime = millis();
      wetDurationMinutes = (currentWetTime - wetStartTime) / 60000; // Final duration in minutes
      Serial.print("Wetness Duration: ");
      Serial.print(wetDurationMinutes); // Convert milliseconds to seconds
      Serial.println(" min");
      // Calculate average temperature and humidity
      float avgTemp = overallTemperatureTotal / overallChunkCount;
      int avgHumidity = overallHumidityTotal / overallChunkCount;

// Predict disease based on plant type, wetness duration, temperature, and humidity
      predictedDisease = predictDisease(plantType, wetDurationMinutes, avgTemp, avgHumidity);


      // Send data to Google Sheets with averages

      sendDataToGoogleSheets("Dry", wetDurationMinutes, weatherDescription, avgTemp, avgHumidity, predictedDisease, plantType);

      // Reset totals for next session
      Serial.println("Wetness period ended.");
      Serial.print("Overall Avg Temperature: ");
      Serial.print(avgTemp);
      Serial.println(" °C");
      Serial.print("Overall Avg Humidity: ");
      Serial.print(avgHumidity);
      Serial.println(" %");

      // Reset overall variables for the next wetness period
      overallTemperatureTotal = 0.0;
      overallHumidityTotal = 0.0;
      overallChunkCount = 0;
    }
  }

  delay(5000); // Sampling interval
}

int readSoilMoisture() {
  digitalWrite(SOIL_MOISTURE_PIN_POWER, HIGH);  // Turn on soil moisture sensor
  delay(10);  // Stabilize sensor
  
  int soilMoisture = analogRead(SENSOR_PIN);  // Read soil moisture
  Serial.print("Soil Moisture: ");
  Serial.println(soilMoisture);
  
  digitalWrite(SOIL_MOISTURE_PIN_POWER, LOW);  // Turn off soil moisture sensor
  return soilMoisture;
}

// Function to read raindrop sensor
int readRaindropSensor() {
  digitalWrite(SENSOR_PIN_POWER, HIGH);  // Turn on raindrop sensor
  delay(50);  // Stabilize sensor
  
  int raindrop = analogRead(SENSOR_PIN);  // Read raindrop sensor
  Serial.print("Raindrop Sensor: ");
  Serial.println(raindrop);
  
  digitalWrite(SENSOR_PIN_POWER, LOW);  // Turn off raindrop sensor
  return raindrop;
}
