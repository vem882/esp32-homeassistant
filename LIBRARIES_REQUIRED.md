# Required Libraries for ESP32 Home Assistant Thermostat

This project requires the following Arduino libraries to be installed:

## Core ESP32 Libraries (should be included with ESP32 board package):
- WiFi
- WebServer  
- EEPROM
- HTTPClient
- SPI

## External Libraries (install via Arduino Library Manager):

1. **TFT_eSPI** by Bodmer
   - Used for the ILI9341 TFT display
   - Version 2.4.0 or newer

2. **XPT2046_Touchscreen** by Paul Stoffregen
   - Used for the XPT2046 touch controller
   - Version 1.4 or newer

3. **lvgl** by lvgl
   - Modern UI library for embedded systems
   - Version 9.0.0 or newer (API updated for v9.x)
   - Provides smooth animations and modern UI components
   - Note: If you have v8.x, please update to v9.x for compatibility

3. **ArduinoJson** by Benoit Blanchon
   - Used for parsing Home Assistant API responses
   - Version 6.19.0 or newer

## Installation Instructions:

### Arduino IDE:
1. Open Arduino IDE
2. Go to Tools > Manage Libraries
3. Search for each library by name and install

### PlatformIO:
Add to your `platformio.ini`:
```ini
lib_deps = 
    bodmer/TFT_eSPI@^2.4.0
    paulstoffregen/XPT2046_Touchscreen@^1.4
    bblanchon/ArduinoJson@^6.19.0
```

## TFT_eSPI Configuration:
Make sure your `User_Setup.h` file in the TFT_eSPI library folder is configured for the ESP32-2432S028R:

```cpp
#define ILI9341_DRIVER
#define TFT_WIDTH  240
#define TFT_HEIGHT 320
#define TFT_MISO 12
#define TFT_MOSI 13
#define TFT_SCLK 14
#define TFT_CS   15
#define TFT_DC    2
#define TFT_RST   4
#define LOAD_GLCD
#define LOAD_FONT2
#define LOAD_FONT4
#define LOAD_FONT6
#define LOAD_FONT7
#define LOAD_FONT8
#define LOAD_GFXFF
#define SMOOTH_FONT
#define SPI_FREQUENCY  27000000
```

## Board Configuration:
- Board: ESP32 Dev Module
- Upload Speed: 921600
- CPU Frequency: 240MHz
- Flash Frequency: 80MHz
- Flash Mode: QIO
- Flash Size: 4MB
- Partition Scheme: Default 4MB with spiffs

After installing all libraries and configuring TFT_eSPI, the sketch should compile successfully.
