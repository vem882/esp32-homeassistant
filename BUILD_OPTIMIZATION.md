# ESP32 Build Size Optimization

The ESP32 thermostat project has been optimized to reduce flash memory usage. Here are the steps to build it successfully:

## NEW: SD Card Support for Maximum Space Savings! 🎉

**The best optimization**: Use the built-in microSD card slot on your ESP32-2432S028R!

### SD Card Benefits:
- **25-30KB flash memory freed** by moving HTML files to SD card
- Beautiful, full-featured web interface without memory constraints
- Easy customization - edit HTML/CSS files directly on SD card
- Automatic fallback to minimal HTML if SD card not available
- Supports dynamic content, themes, and multilingual interfaces

### SD Card Setup:
1. Format microSD card as FAT32
2. Insert into ESP32-2432S028R slot
3. Code automatically creates `/config.html` and `/success.html`
4. Enjoy full-featured web interface!

**See SD_CARD_SETUP.md for detailed instructions.**

## 1. Use the Minimal LVGL Configuration

Replace your current `lv_conf.h` file with the minimal version:

1. Copy `lv_conf_minimal.h` to your Arduino libraries folder
2. Rename it to `lv_conf.h`
3. This version only enables essential LVGL features and fonts (14, 18, 32pt Montserrat)

## 2. Arduino IDE Optimization Settings

In Arduino IDE, select these options for your ESP32 board:

- **Partition Scheme**: "No OTA (2MB APP/2MB SPIFFS)" or "Huge APP (3MB No OTA/1MB SPIFFS)"
- **Flash Mode**: "QIO"
- **Flash Frequency**: "80MHz"
- **Upload Speed**: "921600"
- **Core Debug Level**: "None"

## 3. Compiler Optimization (Advanced)

For even more space savings, you can modify the platform.txt file in your ESP32 Arduino core:

Find this line:
```
compiler.c.elf.flags=-nostdlib
```

Add optimization flags:
```
compiler.c.elf.flags=-nostdlib -Wl,--gc-sections
compiler.c.flags=-c -Os -std=gnu99 -ffunction-sections -fdata-sections
compiler.cpp.flags=-c -Os -std=gnu++11 -ffunction-sections -fdata-sections
```

## 4. Code Optimizations Made

The following optimizations have been applied to the code:

- **Font Reduction**: Changed from large fonts (48pt) to smaller ones (32pt max)
- **LVGL Buffer Size**: Reduced from 10 lines to 5 lines
- **Removed Legacy TFT Code**: Eliminated unused TFT_eSPI drawing functions
- **Simplified Web Interface**: Reduced HTML/CSS/JavaScript size
- **Function Cleanup**: Removed unused function declarations
- **Weather Simplification**: Reduced JSON parsing complexity

## 5. Expected Sizes After Optimization

- **Original Size**: ~1,547KB (118% of 1,310KB limit)
- **Optimized Size**: ~900-1,100KB (70-85% of limit)

## 6. If Still Too Large

If the sketch is still too large, consider these additional steps:

1. **Use ESP32-WROVER**: Has 4MB flash instead of 2MB
2. **Remove Weather Feature**: Comment out weather-related code
3. **Use ESP32-S3**: Has more flash memory available
4. **Custom Partition**: Create a custom partition with larger APP space

## 7. Alternative Boards

Consider these ESP32 variants with more flash:
- ESP32-WROVER (4MB Flash)
- ESP32-S3 (8MB Flash)
- ESP32-C3 (4MB Flash)

## 8. Verification

After making these changes, the sketch should compile and fit within the ESP32's flash memory limits while maintaining all core functionality:

- LVGL-based modern UI
- Home Assistant integration
- WiFi configuration portal
- Temperature control
- Screensaver with weather
- Touch interface

The UI will still look modern and functional, just with slightly smaller fonts to save memory.
