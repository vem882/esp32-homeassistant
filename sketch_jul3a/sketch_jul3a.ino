#include <SPI.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <NetworkClientSecure.h>
#include <ArduinoJson.h>
#include <time.h>
#include <SD.h>
#include <FS.h>
#include <Update.h>

// Firmware version and OTA configuration
#define FIRMWARE_VERSION "2025.07.05.31"
#define OTA_UPDATE_URL "https://raw.githubusercontent.com/vem882/esp32-homeassistant/main/version.json"
#define OTA_FIRMWARE_URL "https://raw.githubusercontent.com/vem882/esp32-homeassistant/main/firmware.bin"
#define UI_CONFIG_URL "https://raw.githubusercontent.com/vem882/esp32-homeassistant/main/sd_files/ui_config.json"

// Update intervals
const unsigned long UPDATE_INTERVAL = 3000;         // HA data update: 3 seconds
const unsigned long OTA_CHECK_INTERVAL = 200000;    // OTA check: 5 minutes
const unsigned long SCREENSAVER_TIMEOUT = 60000;    // Screensaver: 60 seconds

// Device identification
String deviceId = "";
unsigned long totalRuntime = 0;

// Forward declarations
void drawSVGIcon(const char* iconName, int x, int y, int size, uint16_t color);
String generateDeviceId();
void sendDeviceStats();
bool updateUIConfig();
bool downloadFirmwareToSD(const char* firmwareUrl);
bool applyFirmwareFromSD();
bool isNewerVersion(const char* latestVersion, const char* currentVersion);
void listSDCardContents();

// Hardware definitions
#define XPT2046_IRQ 36
#define XPT2046_MOSI 32
#define XPT2046_MISO 39
#define XPT2046_CLK 25
#define XPT2046_CS 33
#define SD_CS 5

static const uint16_t screenWidth = 320;
static const uint16_t screenHeight = 240;

SPIClass touchscreenSPI(VSPI);
XPT2046_Touchscreen touchscreen(XPT2046_CS, XPT2046_IRQ);
TFT_eSPI tft = TFT_eSPI();

// Configuration structure
struct Config {
  char ssid[32];
  char password[64];
  char timezone[64];
  char ha_host[32];
  int ha_port;
  char ha_token[256];
  char temperature_sensor[64];
  char climate_entity[64];
  char weather_entity[64];
  bool configured;
};

Config config;

// Application state
float currentTemp = 21.5;
float targetTemp = 21.0;
bool heating = false;
bool screensaverActive = false;
unsigned long lastActivity = 0;
unsigned long lastUpdate = 0;

// Touch handling
struct TouchPoint {
  int x, y;
  bool pressed;
};

TouchPoint touch;

// UI Elements loaded from SD
DynamicJsonDocument uiConfig(8192);
String currentScreen = "main_screen";

// Simple UI rendering engine
class UIRenderer {
private:
  uint16_t parseColor(const char* hexColor) {
    if (hexColor[0] == '#') hexColor++;
    return (uint16_t)strtol(hexColor, NULL, 16);
  }
  
  void drawElement(JsonObject element) {
    String type = element["type"].as<String>();
    
    if (type == "circle") {
      drawCircle(element);
    } else if (type == "label") {
      drawLabel(element);
    } else if (type == "button") {
      drawButton(element);
    } else if (type == "indicator") {
      drawIndicator(element);
    } else if (type == "icon") {
      drawIcon(element);
    }
  }
  
  void drawCircle(JsonObject element) {
    int x = element["x"];
    int y = element["y"];
    int radius = element["radius"];
    int borderWidth = element["border_width"] | 2;
    uint16_t borderColor = parseColor(element["border_color"] | "#FFFFFF");
    uint16_t fillColor = parseColor(element["fill_color"] | "#000000");
    
    tft.fillCircle(x, y, radius, fillColor);
    tft.drawCircle(x, y, radius, borderColor);
    if (borderWidth > 1) {
      for (int i = 1; i < borderWidth; i++) {
        tft.drawCircle(x, y, radius - i, borderColor);
      }
    }
  }
  
  void drawLabel(JsonObject element) {
    int x = element["x"];
    int y = element["y"];
    String text = element["text"].as<String>();
    int fontSize = element["font_size"] | 16;
    uint16_t color = parseColor(element["color"] | "#FFFFFF");
    String align = element["align"] | "left";
    
    tft.setTextColor(color, TFT_BLACK);
    tft.setTextSize(fontSize / 8); // Approximate scaling
    
    if (align == "center") {
      tft.drawCentreString(text, x, y, 2);
    } else if (align == "right") {
      tft.drawRightString(text, x, y, 2);
    } else {
      tft.drawString(text, x, y, 2);
    }
  }
  
  void drawButton(JsonObject element) {
    int x = element["x"];
    int y = element["y"];
    int width = element["width"];
    int height = element["height"];
    String text = element["text"].as<String>();
    uint16_t bgColor = parseColor(element["bg_color"] | "#333333");
    uint16_t textColor = parseColor(element["text_color"] | "#FFFFFF");
    
    tft.fillRoundRect(x - width/2, y - height/2, width, height, 5, bgColor);
    tft.setTextColor(textColor, bgColor);
    tft.drawCentreString(text, x, y - 4, 2);
  }
  
  void drawIndicator(JsonObject element) {
    bool visible = element["visible"] | false;
    if (!visible) return;
    
    int x = element["x"];
    int y = element["y"];
    int width = element["width"];
    int height = element["height"];
    uint16_t color = parseColor(element["color"] | "#FF0000");
    String shape = element["shape"] | "circle";
    
    if (shape == "circle") {
      tft.fillCircle(x, y, width/2, color);
    } else {
      tft.fillRect(x - width/2, y - height/2, width, height, color);
    }
  }
  
  void drawIcon(JsonObject element) {
    int x = element["x"];
    int y = element["y"];
    int size = element["size"] | 16;
    String iconName = element["icon"].as<String>();
    uint16_t color = parseColor(element["color"] | "#FFFFFF");
    
    if (iconName.length() > 0) {
      drawSVGIcon(iconName.c_str(), x - size/2, y - size/2, size, color);
    }
  }
  
public:
  void renderScreen(const char* screenName) {
    if (!uiConfig.containsKey(screenName)) return;
    
    JsonObject screen = uiConfig[screenName];
    
    // Clear screen with background color
    uint16_t bgColor = parseColor(screen["background"] | "#000000");
    tft.fillScreen(bgColor);
    
    // Draw all elements
    JsonArray elements = screen["elements"];
    for (JsonObject element : elements) {
      drawElement(element);
    }
  }
  
  void updateElement(const char* screenName, const char* elementName, const char* property, const char* value) {
    if (!uiConfig.containsKey(screenName)) return;
    
    JsonArray elements = uiConfig[screenName]["elements"];
    for (JsonObject element : elements) {
      if (element["name"] == elementName) {
        element[property] = value;
        break;
      }
    }
  }
  
  String checkTouch(int touchX, int touchY) {
    if (!uiConfig.containsKey(currentScreen.c_str())) return "";
    
    JsonArray elements = uiConfig[currentScreen.c_str()]["elements"];
    for (JsonObject element : elements) {
      if (!element.containsKey("action")) continue;
      
      String type = element["type"].as<String>();
      int x = element["x"];
      int y = element["y"];
      
      if (type == "button") {
        int width = element["width"];
        int height = element["height"];
        
        if (touchX >= x - width/2 && touchX <= x + width/2 &&
            touchY >= y - height/2 && touchY <= y + height/2) {
          return element["action"].as<String>();
        }
      } else if (type == "circle" && element["interactive"]) {
        int radius = element["radius"];
        
        if (sqrt(pow(touchX - x, 2) + pow(touchY - y, 2)) <= radius) {
          return element["action"].as<String>();
        }
      }
    }
    return "";
  }
};

UIRenderer ui;

// Core functions
void loadUIConfig() {
  if (!SD.exists("/ui_config.json")) {
    showError("UI CONFIG MISSING");
    return;
  }
  
  File file = SD.open("/ui_config.json");
  if (!file) {
    showError("UI CONFIG ERROR");
    return;
  }
  
  DeserializationError error = deserializeJson(uiConfig, file);
  file.close();
  
  if (error) {
    showError("UI JSON ERROR");
    return;
  }
}

void loadConfig() {
  Serial.println("Loading config.json...");
  
  if (!SD.exists("/config.json")) {
    Serial.println("ERROR: config.json not found on SD card");
    showError("CONFIG MISSING");
    return;
  }
  
  File file = SD.open("/config.json");
  if (!file) {
    Serial.println("ERROR: Failed to open config.json");
    showError("CONFIG ERROR");
    return;
  }
  
  Serial.println("Config file opened, parsing JSON...");
  
  DynamicJsonDocument doc(1024);
  DeserializationError error = deserializeJson(doc, file);
  file.close();
  
  if (error) {
    Serial.print("ERROR: JSON parsing failed: ");
    Serial.println(error.c_str());
    showError("JSON ERROR");
    return;
  }
  
  // Load and debug configuration values
  strncpy(config.ssid, doc["wifi"]["ssid"], sizeof(config.ssid));
  strncpy(config.password, doc["wifi"]["password"], sizeof(config.password));
  strncpy(config.ha_host, doc["homeassistant"]["host"], sizeof(config.ha_host));
  config.ha_port = doc["homeassistant"]["port"];
  strncpy(config.ha_token, doc["homeassistant"]["token"], sizeof(config.ha_token));
  strncpy(config.temperature_sensor, doc["entities"]["temperature_sensor"], sizeof(config.temperature_sensor));
  strncpy(config.climate_entity, doc["entities"]["climate_entity"], sizeof(config.climate_entity));
  strncpy(config.timezone, doc["system"]["timezone"], sizeof(config.timezone));
  
  config.configured = true;
  
  // Debug output (don't print sensitive data like password/token)
  Serial.println("Configuration loaded:");
  Serial.print("  WiFi SSID: ");
  Serial.println(config.ssid);
  Serial.print("  WiFi Password: ");
  Serial.println(strlen(config.password) > 0 ? "[SET]" : "[EMPTY]");
  Serial.print("  HA Host: ");
  Serial.println(config.ha_host);
  Serial.print("  HA Port: ");
  Serial.println(config.ha_port);
  Serial.print("  HA Token: ");
  Serial.println(strlen(config.ha_token) > 0 ? "[SET]" : "[EMPTY]");
  Serial.print("  Temperature Sensor: ");
  Serial.println(config.temperature_sensor);
  Serial.print("  Climate Entity: ");
  Serial.println(config.climate_entity);
  Serial.print("  Timezone: ");
  Serial.println(config.timezone);
}

void showError(const char* message) {
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_RED, TFT_BLACK);
  tft.drawCentreString(message, screenWidth/2, screenHeight/2, 2);
  while(true) delay(1000);
}

void readTouch() {
  touch.pressed = false;
  
  if (touchscreen.tirqTouched() && touchscreen.touched()) {
    TS_Point p = touchscreen.getPoint();
    touch.x = map(p.x, 200, 3700, 1, screenWidth);
    touch.y = map(p.y, 240, 3800, 1, screenHeight);
    touch.pressed = true;
    
    lastActivity = millis();
  }
}

void handleTouch() {
  if (!touch.pressed) return;
  
  String action = ui.checkTouch(touch.x, touch.y);
  
  if (action == "temp_increase") {
    if (targetTemp < 30.0) {
      targetTemp += 0.5;
      setTargetTemperature(targetTemp);
    }
  } else if (action == "temp_decrease") {
    if (targetTemp > 15.0) {
      targetTemp -= 0.5;
      setTargetTemperature(targetTemp);
    }
  } else if (action == "temperature_wheel") {
    handleTemperatureWheel();
  }
  
  if (screensaverActive) {
    screensaverActive = false;
    currentScreen = "main_screen";
    ui.renderScreen(currentScreen.c_str());
  }
}

void handleTemperatureWheel() {
  // Calculate angle from center of thermostat ring
  int centerX = 160, centerY = 120;
  float angle = atan2(touch.y - centerY, touch.x - centerX) * 180 / PI;
  
  // Convert angle to temperature (15°C to 30°C range)
  float newTemp = map(angle, -180, 180, 15.0, 30.0);
  newTemp = round(newTemp * 2) / 2.0; // Round to 0.5 increments
  
  if (newTemp != targetTemp) {
    targetTemp = newTemp;
    setTargetTemperature(targetTemp);
  }
}

void updateUI() {
  char buffer[32];
  
  // Update temperature display
  snprintf(buffer, sizeof(buffer), "%.1f°", currentTemp);
  ui.updateElement(currentScreen.c_str(), "temp_display", "text", buffer);
  
  snprintf(buffer, sizeof(buffer), "Target: %.1f°", targetTemp);
  ui.updateElement(currentScreen.c_str(), "target_temp", "text", buffer);
  
  // Update clock
  struct tm timeinfo;
  if (getLocalTime(&timeinfo)) {
    if (currentScreen == "main_screen") {
      strftime(buffer, sizeof(buffer), "%H:%M", &timeinfo);
      ui.updateElement("main_screen", "clock", "text", buffer);
    } else {
      strftime(buffer, sizeof(buffer), "%H:%M", &timeinfo);
      ui.updateElement("screensaver", "large_clock", "text", buffer);
      
      strftime(buffer, sizeof(buffer), "%a, %d %b", &timeinfo);
      ui.updateElement("screensaver", "date", "text", buffer);
    }
  }
  
  // Update heating indicator
  if (uiConfig.containsKey(currentScreen.c_str())) {
    JsonArray elements = uiConfig[currentScreen.c_str()]["elements"];
    for (JsonObject element : elements) {
      if (element["name"] == "heating_status") {
        element["visible"] = heating;
        break;
      }
    }
  }
  
  // Update WiFi strength indicator
  if (WiFi.status() == WL_CONNECTED) {
    int rssi = WiFi.RSSI();
    snprintf(buffer, sizeof(buffer), "%d", rssi);
    ui.updateElement(currentScreen.c_str(), "wifi_strength", "text", buffer);
    
    // Update WiFi icon based on signal strength
    String wifiIcon = "wifi_full";
    String wifiColor = "#00FF00"; // Green
    
    if (rssi > -50) {
      wifiIcon = "wifi_full";
      wifiColor = "#00FF00"; // Green
    } else if (rssi > -60) {
      wifiIcon = "wifi_good";
      wifiColor = "#FFFF00"; // Yellow
    } else if (rssi > -70) {
      wifiIcon = "wifi_weak";
      wifiColor = "#FF8000"; // Orange
    } else {
      wifiIcon = "wifi_poor";
      wifiColor = "#FF0000"; // Red
    }
    
    ui.updateElement(currentScreen.c_str(), "wifi_icon", "icon", wifiIcon.c_str());
    ui.updateElement(currentScreen.c_str(), "wifi_icon", "color", wifiColor.c_str());
  } else {
    ui.updateElement(currentScreen.c_str(), "wifi_strength", "text", "N/A");
    ui.updateElement(currentScreen.c_str(), "wifi_icon", "icon", "wifi_off");
    ui.updateElement(currentScreen.c_str(), "wifi_icon", "color", "#FF0000");
  }
  
  // Update firmware version display
  if (currentScreen == "main_screen") {
    String versionText = "v" + String(FIRMWARE_VERSION);
    ui.updateElement(currentScreen.c_str(), "firmware_version", "text", versionText.c_str());
  }
  
  // Re-render screen
  ui.renderScreen(currentScreen.c_str());
}

// Home Assistant API functions
void setTargetTemperature(float temp) {
  HTTPClient http;
  String url = "http://" + String(config.ha_host) + ":" + String(config.ha_port) + 
               "/api/services/climate/set_temperature";
  
  http.begin(url);
  http.addHeader("Authorization", "Bearer " + String(config.ha_token));
  http.addHeader("Content-Type", "application/json");
  
  DynamicJsonDocument doc(128);
  doc["entity_id"] = config.climate_entity;
  doc["temperature"] = temp;
  
  String jsonString;
  serializeJson(doc, jsonString);
  
  http.POST(jsonString);
  http.end();
}

void getTemperature() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WARNING: WiFi not connected, skipping temperature update");
    return;
  }
  
  HTTPClient http;
  String url = "http://" + String(config.ha_host) + ":" + String(config.ha_port) + 
               "/api/states/" + String(config.temperature_sensor);
  
 // Serial.print("Getting temperature from: ");
 // Serial.println(url);
  
  http.begin(url);
  http.addHeader("Authorization", "Bearer " + String(config.ha_token));
  
  int httpCode = http.GET();
 // Serial.print("HTTP Response Code: ");
 // Serial.println(httpCode);
  
  if (httpCode == 200) {
    String payload = http.getString();
 //   Serial.print("Response payload: ");
 //   Serial.println(payload);
    
    DynamicJsonDocument doc(256);
    DeserializationError error = deserializeJson(doc, payload);
    
    if (!error) {
      String stateStr = doc["state"].as<String>();
      if (stateStr != "unavailable") {
        float newTemp = stateStr.toFloat();
        Serial.print("Temperature updated: ");
        Serial.print(currentTemp);
        Serial.print(" -> ");
        Serial.println(newTemp);
        currentTemp = newTemp;
      } else {
        Serial.println("Temperature sensor unavailable");
      }
    } else {
      Serial.print("JSON parsing error: ");
      Serial.println(error.c_str());
    }
  } else {
    Serial.print("HTTP request failed with code: ");
    Serial.println(httpCode);
  }
  
  http.end();
}

void setup() {
  // Initialize Serial for debugging
  Serial.begin(115200);
  Serial.println("=== ESP32 Thermostat Starting ===");
  
  // Initialize hardware
  if (!SD.begin(SD_CS)) {
    Serial.println("ERROR: SD Card initialization failed!");
    showError("SD CARD ERROR");
  }
  Serial.println("SD Card initialized successfully");
  
  // Load configurations
  Serial.println("Loading configuration...");
  loadConfig();
  loadUIConfig();
  
  // Initialize display
  tft.init();
  tft.setRotation(1);
  
  // Initialize touchscreen
  touchscreenSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  touchscreen.begin(touchscreenSPI);
  touchscreen.setRotation(1);
  
  // Connect to WiFi with detailed debugging
  connectToWiFi();
  
  // Setup time
  configTime(0, 0, "pool.ntp.org");
  setenv("TZ", config.timezone, 1);
  tzset();
  
  // Initial render
  ui.renderScreen("main_screen");
  
  // Generate device ID
  deviceId = generateDeviceId();
  Serial.print("Device ID: ");
  Serial.println(deviceId);
  
  // Update total runtime (load from EEPROM or similar could be added)
  totalRuntime = millis() / 1000; // Current session start time
  
  // Send device stats (optional, non-blocking)
  Serial.println("Sending device stats (optional)...");
  sendDeviceStats();
  
  // Check for OTA updates immediately on boot
  Serial.println("=== BOOT OTA CHECK STARTING ===");
  Serial.println("Checking for OTA updates on boot...");
  bool otaResult = checkForOTAUpdate();
  if (otaResult) {
    // OTA update successful, device will restart
    Serial.println("OTA update completed, restarting...");
    return;
  } else {
    Serial.println("OTA update not needed or failed - continuing with normal boot");
  }
  Serial.println("=== BOOT OTA CHECK COMPLETED ===");
  
  // Initial data
  getTemperature();
  lastActivity = millis();
  lastUpdate = millis();
  
  Serial.println("Setup completed successfully");
}

void loop() {
  readTouch();
  handleTouch();
  
  // Check WiFi connection
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi connection lost, attempting to reconnect...");
    WiFi.reconnect();
    delay(5000);
    return;
  }
  
  // Screensaver logic
  if (millis() - lastActivity > SCREENSAVER_TIMEOUT && !screensaverActive) {
    screensaverActive = true;
    currentScreen = "screensaver";
    ui.renderScreen(currentScreen.c_str());
  }
  
  // Update data periodically
  if (millis() - lastUpdate > UPDATE_INTERVAL) {
    getTemperature();
    updateUI();
    lastUpdate = millis();
  }
  
  // Check for UI config updates every hour
  static unsigned long lastUIConfigCheck = 0;
  if (millis() - lastUIConfigCheck > 3600000) { // 1 hour
    lastUIConfigCheck = millis();
    
    Serial.println("Checking for UI config updates...");
    if (updateUIConfig()) {
      Serial.println("UI config updated, re-rendering screen");
      ui.renderScreen(currentScreen.c_str());
    }
  }
  
  // Check for OTA updates every 5 minutes (300000 ms)
  static unsigned long lastOTACheck = 0;
  if (millis() - lastOTACheck > OTA_CHECK_INTERVAL) {
    lastOTACheck = millis();
    
    Serial.println("Performing scheduled OTA check...");
    if (checkForOTAUpdate()) {
      // OTA update successful, device will restart
      return;
    }
  }
  
  // Send device statistics every 24 hours (86400000 ms)
  static unsigned long lastStatsReport = 0;
  if (millis() - lastStatsReport > 86400000) { // 24 hours
    lastStatsReport = millis();
    
    Serial.println("Sending periodic device statistics...");
    sendDeviceStats();
  }
  
  delay(50);
}

// Add new WiFi connection function with detailed debugging
void connectToWiFi() {
  Serial.println("=== Starting WiFi Connection ===");
  
  // Validate configuration
  if (strlen(config.ssid) == 0) {
    Serial.println("ERROR: WiFi SSID is empty!");
    showError("WIFI SSID EMPTY");
    return;
  }
  
  if (strlen(config.password) == 0) {
    Serial.println("WARNING: WiFi password is empty!");
  }
  
  Serial.print("Attempting to connect to WiFi network: ");
  Serial.println(config.ssid);
  
  // Disconnect if already connected
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("Already connected to WiFi, disconnecting first...");
    WiFi.disconnect();
    delay(100);
  }
  
  // Set WiFi mode
  WiFi.mode(WIFI_STA);
  Serial.println("WiFi mode set to STA");
  
  // Start connection
  WiFi.begin(config.ssid, config.password);
  Serial.println("WiFi.begin() called, waiting for connection...");
  
  int attempts = 0;
  int maxAttempts = 30; // 15 seconds timeout
  
  while (WiFi.status() != WL_CONNECTED && attempts < maxAttempts) {
    delay(500);
    attempts++;
    Serial.print(".");
    
    // Print current status every 5 attempts
    if (attempts % 5 == 0) {
      Serial.println();
      Serial.print("Connection attempt ");
      Serial.print(attempts);
      Serial.print("/");
      Serial.print(maxAttempts);
      Serial.print(" - Status: ");
      printWiFiStatus();
    }
  }
  
  Serial.println(); // New line after dots
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("=== WiFi Connected Successfully! ===");
    Serial.print("  IP Address: ");
    Serial.println(WiFi.localIP());
    Serial.print("  Subnet Mask: ");
    Serial.println(WiFi.subnetMask());
    Serial.print("  Gateway: ");
    Serial.println(WiFi.gatewayIP());
    Serial.print("  DNS: ");
    Serial.println(WiFi.dnsIP());
    Serial.print("  RSSI: ");
    Serial.print(WiFi.RSSI());
    Serial.println(" dBm");
    Serial.print("  MAC Address: ");
    Serial.println(WiFi.macAddress());
  } else {
    Serial.println("=== WiFi Connection Failed! ===");
    Serial.print("Final status: ");
    printWiFiStatus();
    Serial.println("Possible reasons:");
    Serial.println("  - Wrong SSID or password");
    Serial.println("  - Network not available");
    Serial.println("  - Signal too weak");
    Serial.println("  - Network security issues");
    Serial.println("  - Router configuration problems");
    
    showError("WIFI ERROR");
  }
}

// Helper function to print WiFi status
void printWiFiStatus() {
  wl_status_t status = WiFi.status();
  switch (status) {
    case WL_IDLE_STATUS:
      Serial.print("WL_IDLE_STATUS");
      break;
    case WL_NO_SSID_AVAIL:
      Serial.print("WL_NO_SSID_AVAIL (Network not found)");
      break;
    case WL_SCAN_COMPLETED:
      Serial.print("WL_SCAN_COMPLETED");
      break;
    case WL_CONNECTED:
      Serial.print("WL_CONNECTED");
      break;
    case WL_CONNECT_FAILED:
      Serial.print("WL_CONNECT_FAILED (Wrong password?)");
      break;
    case WL_CONNECTION_LOST:
      Serial.print("WL_CONNECTION_LOST");
      break;
    case WL_DISCONNECTED:
      Serial.print("WL_DISCONNECTED");
      break;
    default:
      Serial.print("UNKNOWN_STATUS (");
      Serial.print(status);
      Serial.print(")");
      break;
  }
}

// OTA Update Functions with enhanced safety
bool checkForOTAUpdate() {
  HTTPClient http;
  NetworkClientSecure client;
  
  Serial.println("=== CHECKING FOR OTA UPDATE ===");
  Serial.print("Checking URL: ");
  Serial.println(OTA_UPDATE_URL);
  
  // Skip certificate verification for GitHub
  client.setInsecure();
  
  // Set timeout to prevent hanging
  http.setTimeout(15000);
  http.begin(client, OTA_UPDATE_URL);
  http.addHeader("User-Agent", "ESP32-HomeAssistant");
  
  int httpCode = http.GET();
  Serial.print("HTTP Response Code: ");
  Serial.println(httpCode);
  
  if (httpCode == HTTP_CODE_OK) {
    String payload = http.getString();
    Serial.println("Version info downloaded successfully");
    
    StaticJsonDocument<512> doc;
    DeserializationError error = deserializeJson(doc, payload);
    
    if (!error) {
      const char* latestVersion = doc["version"];
      const char* downloadUrl = doc["firmware_url"];
      const char* minVersion = doc["min_version"];
      bool forceUpdate = doc["force_update"] | false;
      
      Serial.print("Current version: ");
      Serial.println(FIRMWARE_VERSION);
      Serial.print("Latest version: ");
      Serial.println(latestVersion);
      
      // Check minimum version requirement
      if (minVersion && strcmp(FIRMWARE_VERSION, minVersion) < 0) {
        Serial.println("Current version is below minimum required version");
        if (!forceUpdate) {
          Serial.println("Skipping update due to version constraint");
          http.end();
          return false;
        }
      }
      
      // Check if update is needed
      if (forceUpdate || isNewerVersion(latestVersion, FIRMWARE_VERSION)) {
        Serial.println("New firmware available!");
        http.end();
        return performOTAUpdate(downloadUrl ? downloadUrl : OTA_FIRMWARE_URL);
      } else {
        Serial.println("Firmware is up to date");
      }
    } else {
      Serial.print("JSON parsing error: ");
      Serial.println(error.c_str());
    }
  } else {
    Serial.print("HTTP error: ");
    Serial.println(httpCode);
    Serial.println("OTA check failed - will retry later");
  }
  
  http.end();
  Serial.println("=== OTA CHECK COMPLETED ===");
  return false;
}

bool performOTAUpdate(const char* firmwareUrl) {
  Serial.println("Starting OTA update...");
  
  // First download firmware to SD card
  if (!downloadFirmwareToSD(firmwareUrl)) {
    Serial.println("Failed to download firmware to SD card");
    showOTAError("Download failed");
    return false;
  }
  
  // Then apply firmware from SD card
  return applyFirmwareFromSD();
}

void updateOTAProgress(int progress) {
  // Clear previous progress
  tft.fillRect(10, 120, 220, 50, TFT_BLACK);
  
  // Show percentage
  tft.setCursor(10, 120);
  tft.setTextColor(TFT_WHITE);
  tft.print("Progress: ");
  tft.print(progress);
  tft.print("%");
  
  // Draw progress bar
  int barWidth = (progress * 200) / 100;
  tft.fillRect(10, 150, barWidth, 15, TFT_GREEN);
  tft.drawRect(10, 150, 200, 15, TFT_WHITE);
  
  // Show time estimate (rough)
  if (progress > 5) {
    tft.setCursor(10, 180);
    tft.setTextSize(1);
    tft.print("Please wait...");
    tft.setTextSize(2);
  }
}

void showOTAError(const char* error) {
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_RED);
  tft.setTextSize(2);
  tft.setCursor(10, 50);
  tft.println("Update Failed!");
  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(1);
  tft.setCursor(10, 80);
  tft.println(error);
  tft.setCursor(10, 100);
  tft.println("Check Serial for details");
  tft.setCursor(10, 120);
  tft.println("Device will continue with");
  tft.setCursor(10, 140);
  tft.println("current firmware");
}

// SVG icon rendering support
void drawSVGIcon(const char* iconName, int x, int y, int size, uint16_t color) {
  String iconPath = "/icons/" + String(iconName) + ".svg";
  
  if (SD.exists(iconPath)) {
    // Simple SVG parsing for basic shapes
    File svgFile = SD.open(iconPath);
    if (svgFile) {
      String svgContent = svgFile.readString();
      svgFile.close();
      
      // WiFi icons
      if (String(iconName).indexOf("wifi") >= 0) {
        if (String(iconName) == "wifi_full") {
          // Draw full WiFi signal (3 arcs)
          tft.drawCircle(x + size/2, y + size, size/4, color);
          tft.drawCircle(x + size/2, y + size, size/2, color);
          tft.drawCircle(x + size/2, y + size, 3*size/4, color);
          tft.fillCircle(x + size/2, y + size, 2, color);
        } else if (String(iconName) == "wifi_good") {
          // Draw good WiFi signal (2 arcs)
          tft.drawCircle(x + size/2, y + size, size/4, color);
          tft.drawCircle(x + size/2, y + size, size/2, color);
          tft.fillCircle(x + size/2, y + size, 2, color);
        } else if (String(iconName) == "wifi_weak") {
          // Draw weak WiFi signal (1 arc)
          tft.drawCircle(x + size/2, y + size, size/4, color);
          tft.fillCircle(x + size/2, y + size, 2, color);
        } else if (String(iconName) == "wifi_poor") {
          // Draw poor WiFi signal (just dot)
          tft.fillCircle(x + size/2, y + size, 2, color);
        } else if (String(iconName) == "wifi_off") {
          // Draw WiFi off (X mark)
          tft.drawLine(x, y, x + size, y + size, color);
          tft.drawLine(x + size, y, x, y + size, color);
        }
      }
      // Thermometer icon
      else if (String(iconName) == "thermometer") {
        // Draw simple thermometer
        tft.drawRect(x + size/2 - 2, y, 4, size - 4, color);
        tft.fillCircle(x + size/2, y + size - 2, 3, color);
      }
      // Basic SVG circle parsing (simplified)
      else if (svgContent.indexOf("<circle") >= 0) {
        tft.fillCircle(x + size/2, y + size/2, size/2, color);
      }
      // Basic SVG rectangle parsing
      else if (svgContent.indexOf("<rect") >= 0) {
        tft.fillRect(x, y, size, size, color);
      }
      // Basic SVG path for simple icons
      else {
        // Fallback: draw a simple icon placeholder
        tft.drawRect(x, y, size, size, color);
        tft.drawLine(x, y, x + size, y + size, color);
        tft.drawLine(x + size, y, x, y + size, color);
      }
    }
  } else {
    // Icon not found, draw placeholder
    tft.drawRect(x, y, size, size, TFT_RED);
    tft.drawLine(x + 2, y + 2, x + size - 2, y + size - 2, TFT_RED);
  }
}

// Generate unique device ID
String generateDeviceId() {
  String mac = WiFi.macAddress();
  mac.replace(":", "");
  String chipId = String((uint32_t)ESP.getEfuseMac(), HEX);
  return "ESP32_" + mac.substring(6) + "_" + chipId.substring(0, 4);
}

// Send device statistics to repository
void sendDeviceStats() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi not connected, skipping device stats");
    return;
  }
  
  // Note: GitHub API requires authentication to create issues
  // For now, we'll just log the stats locally
  Serial.println("=== DEVICE STATISTICS ===");
  Serial.print("Device ID: ");
  Serial.println(deviceId);
  Serial.print("Firmware Version: ");
  Serial.println(FIRMWARE_VERSION);
  Serial.print("Current Session Uptime: ");
  Serial.print(millis() / 1000 / 3600);
  Serial.println(" hours");
  Serial.print("Free Heap: ");
  Serial.print(ESP.getFreeHeap());
  Serial.println(" bytes");
  Serial.print("WiFi RSSI: ");
  Serial.print(WiFi.RSSI());
  Serial.println(" dBm");
  Serial.print("Current Temperature: ");
  Serial.print(currentTemp);
  Serial.println("°C");
  Serial.print("Target Temperature: ");
  Serial.print(targetTemp);
  Serial.println("°C");
  Serial.print("Heating Status: ");
  Serial.println(heating ? "ON" : "OFF");
  Serial.println("=== END DEVICE STATISTICS ===");
  
  // TODO: Implement proper stats reporting to a backend service
  // For now, stats are logged to Serial for debugging
}

// Download and update UI config from repository
bool updateUIConfig() {
  HTTPClient http;
  NetworkClientSecure client;
  
  Serial.println("Checking for UI config updates...");
  
  // Skip certificate verification for GitHub
  client.setInsecure();
  
  http.setTimeout(15000);
  http.begin(client, UI_CONFIG_URL);
  http.addHeader("User-Agent", "ESP32-HomeAssistant");
  
  int httpCode = http.GET();
  
  if (httpCode == HTTP_CODE_OK) {
    String payload = http.getString();
    
    // Save new UI config to SD card
    File file = SD.open("/ui_config.json", FILE_WRITE);
    if (file) {
      file.print(payload);
      file.close();
      
      // Reload UI config
      loadUIConfig();
      
      Serial.println("UI config updated successfully");
      http.end();
      return true;
    }
  } else {
    Serial.print("UI config update failed, HTTP code: ");
    Serial.println(httpCode);
  }
  
  http.end();
  return false;
}

// Download firmware to SD card for safer updates
bool downloadFirmwareToSD(const char* firmwareUrl) {
  HTTPClient http;
  NetworkClientSecure client;
  
  Serial.println("Downloading firmware to SD card...");
  
  // Skip certificate verification for GitHub
  client.setInsecure();
  
  // Check available space on SD card
  uint64_t totalBytes = SD.totalBytes();
  uint64_t usedBytes = SD.usedBytes();
  uint64_t freeBytes = totalBytes - usedBytes;
  
  Serial.print("SD Card Space - Total: ");
  Serial.print(totalBytes);
  Serial.print(", Used: ");
  Serial.print(usedBytes);
  Serial.print(", Free: ");
  Serial.print(freeBytes);
  Serial.println(" bytes");
  
  // List SD card contents for debugging
  listSDCardContents();
  
  // Delete old firmware file if exists to free up space
  if (SD.exists("/firmware_new.bin")) {
    File oldFile = SD.open("/firmware_new.bin");
    size_t oldSize = oldFile.size();
    oldFile.close();
    SD.remove("/firmware_new.bin");
    Serial.print("Deleted old firmware file, freed ");
    Serial.print(oldSize);
    Serial.println(" bytes");
    freeBytes += oldSize;
  }
  
  // Check minimum free space (1.5MB should be enough for most ESP32 firmware)
  if (freeBytes < 1500000) {
    Serial.print("Insufficient space on SD card for firmware download. Need at least 1.5MB, have ");
    Serial.print(freeBytes);
    Serial.println(" bytes");
    
    // Try to free up more space by cleaning up temporary files
    Serial.println("Attempting to free up space by cleaning temporary files...");
    
    // Clean up any temporary files
    if (SD.exists("/temp.bin")) {
      SD.remove("/temp.bin");
      Serial.println("Deleted /temp.bin");
    }
    
    // Re-check space after cleanup
    usedBytes = SD.usedBytes();
    freeBytes = totalBytes - usedBytes;
    Serial.print("After cleanup, free space: ");
    Serial.print(freeBytes);
    Serial.println(" bytes");
    
    if (freeBytes < 1500000) {
      Serial.println("Still insufficient space after cleanup");
      return false;
    }
  }
  
  http.setTimeout(60000); // 60 second timeout for large file
  http.begin(client, firmwareUrl);
  http.addHeader("User-Agent", "ESP32-HomeAssistant");
  
  int httpCode = http.GET();
  
  if (httpCode == HTTP_CODE_OK) {
    int contentLength = http.getSize();
    
    Serial.print("Firmware content length: ");
    Serial.print(contentLength);
    Serial.println(" bytes");
    
    if (contentLength > 0) {
      // Check if we have enough space for this specific firmware
      if (contentLength > freeBytes) {
        Serial.print("Not enough space for this firmware. Need ");
        Serial.print(contentLength);
        Serial.print(" bytes, have ");
        Serial.print(freeBytes);
        Serial.println(" bytes");
        http.end();
        return false;
      }
      
      Serial.print("Firmware size: ");
      Serial.println(contentLength);
      
      File firmwareFile = SD.open("/firmware_new.bin", FILE_WRITE);
      if (!firmwareFile) {
        Serial.println("Failed to create firmware file on SD card");
        http.end();
        return false;
      }
      
      WiFiClient* stream = http.getStreamPtr();
      size_t written = 0;
      uint8_t buffer[1024];
      int lastProgress = -1;
      
      // Show download progress on screen
      tft.fillScreen(TFT_BLACK);
      tft.setTextColor(TFT_WHITE);
      tft.setTextSize(2);
      tft.setCursor(10, 50);
      tft.println("Downloading");
      tft.setCursor(10, 80);
      tft.println("to SD Card...");
      
      while (http.connected() && (written < contentLength)) {
        size_t available = stream->available();
        if (available > 0) {
          size_t readBytes = stream->readBytes(buffer, min(available, sizeof(buffer)));
          
          if (readBytes > 0) {
            size_t writtenBytes = firmwareFile.write(buffer, readBytes);
            written += writtenBytes;
            
            // Update progress display
            int progress = (written * 100) / contentLength;
            if (progress != lastProgress) {
              lastProgress = progress;
              
              // Update screen
              tft.fillRect(10, 120, 220, 50, TFT_BLACK);
              tft.setCursor(10, 120);
              tft.print("Progress: ");
              tft.print(progress);
              tft.print("%");
              
              // Draw progress bar
              int barWidth = (progress * 200) / 100;
              tft.fillRect(10, 150, barWidth, 15, TFT_GREEN);
              tft.drawRect(10, 150, 200, 15, TFT_WHITE);
              
              Serial.print("Download progress: ");
              Serial.print(progress);
              Serial.println("%");
            }
            
            if (writtenBytes != readBytes) {
              Serial.println("Error writing to SD card");
              firmwareFile.close();
              http.end();
              return false;
            }
          }
        }
        yield(); // Feed watchdog
      }
      
      firmwareFile.close();
      
      if (written == contentLength) {
        Serial.println("Firmware downloaded to SD card successfully");
        http.end();
        return true;
      } else {
        Serial.println("Firmware download incomplete");
        SD.remove("/firmware_new.bin");
      }
    } else {
      Serial.print("Invalid firmware size or insufficient space: ");
      Serial.println(contentLength);
    }
  } else {
    Serial.print("HTTP error downloading firmware: ");
    Serial.println(httpCode);
  }
  
  http.end();
  return false;
}

// Apply firmware update from SD card
bool applyFirmwareFromSD() {
  if (!SD.exists("/firmware_new.bin")) {
    Serial.println("No firmware file found on SD card");
    return false;
  }
  
  File firmwareFile = SD.open("/firmware_new.bin", FILE_READ);
  if (!firmwareFile) {
    Serial.println("Failed to open firmware file from SD card");
    return false;
  }
  
  size_t firmwareSize = firmwareFile.size();
  Serial.print("Applying firmware from SD card, size: ");
  Serial.println(firmwareSize);
  
  // Initialize update process
  if (!Update.begin(firmwareSize)) {
    Serial.println("Update begin failed");
    firmwareFile.close();
    return false;
  }
  
  // Show update progress on screen
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(2);
  tft.setCursor(10, 50);
  tft.println("Installing");
  tft.setCursor(10, 80);
  tft.println("Firmware...");
  
  size_t written = 0;
  uint8_t buffer[1024];
  int lastProgress = -1;
  
  while (firmwareFile.available()) {
    size_t readBytes = firmwareFile.read(buffer, sizeof(buffer));
    
    if (readBytes > 0) {
      size_t writtenBytes = Update.write(buffer, readBytes);
      written += writtenBytes;
      
      // Update progress display
      int progress = (written * 100) / firmwareSize;
      if (progress != lastProgress) {
        lastProgress = progress;
        
        // Update screen
        tft.fillRect(10, 120, 220, 50, TFT_BLACK);
        tft.setCursor(10, 120);
        tft.print("Progress: ");
        tft.print(progress);
        tft.print("%");
        
        // Draw progress bar
        int barWidth = (progress * 200) / 100;
        tft.fillRect(10, 150, barWidth, 15, TFT_BLUE);
        tft.drawRect(10, 150, 200, 15, TFT_WHITE);
        
        Serial.print("Install progress: ");
        Serial.print(progress);
        Serial.println("%");
      }
      
      if (writtenBytes != readBytes) {
        Serial.println("Error writing firmware");
        firmwareFile.close();
        return false;
      }
    }
    
    yield(); // Feed watchdog
  }
  
  firmwareFile.close();
  
  if (Update.end(true)) {
    Serial.println("Firmware update successful!");
    
    // Clean up
    SD.remove("/firmware_new.bin");
    
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(10, 50);
    tft.setTextColor(TFT_GREEN);
    tft.println("Update Complete!");
    tft.setCursor(10, 80);
    tft.setTextColor(TFT_WHITE);
    tft.println("Restarting...");
    
    delay(3000);
    ESP.restart();
    return true;
  } else {
    Serial.print("Firmware update failed: ");
    Serial.println(Update.errorString());
    SD.remove("/firmware_new.bin");
    return false;
  }
}

// Version comparison function supporting both X.Y.Z and YYYY.MM.DD.BUILD formats
bool isNewerVersion(const char* latestVersion, const char* currentVersion) {
  String latest = String(latestVersion);
  String current = String(currentVersion);
  
  // Check if it's date-based version (YYYY.MM.DD.BUILD)
  if (latest.indexOf('.') > 0 && latest.length() > 8) {
    // Date-based version comparison
    latest.replace(".", "");
    current.replace(".", "");
    
    // Convert to numbers for comparison
    long latestNum = latest.toInt();
    long currentNum = current.toInt();
    
    return latestNum > currentNum;
  } else {
    // Traditional semantic version comparison
    return strcmp(latestVersion, currentVersion) > 0;
  }
}

// Helper function to list SD card contents for debugging
void listSDCardContents() {
  Serial.println("=== SD Card Contents ===");
  File root = SD.open("/");
  if (!root) {
    Serial.println("Failed to open root directory");
    return;
  }
  
  File file = root.openNextFile();
  while (file) {
    if (file.isDirectory()) {
      Serial.print("DIR: ");
      Serial.println(file.name());
    } else {
      Serial.print("FILE: ");
      Serial.print(file.name());
      Serial.print(" (");
      Serial.print(file.size());
      Serial.println(" bytes)");
    }
    file = root.openNextFile();
  }
  root.close();
  Serial.println("=== End SD Card Contents ===");
}