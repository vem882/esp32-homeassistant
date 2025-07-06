# SD Card Hardware Diagnosis

## Issue Summary
The ESP32 device is unable to initialize the SD card despite extensive software attempts. The MISO pin consistently reads 0, indicating a hardware issue.

## Hardware Diagnosis Results

### Pin States During Initialization
- **MISO (pin 12)**: Always reads 0 (Should be 1 when card is ready)
- **CS (pin 15)**: Reads 1 (Correct - deselected state)
- **CS Alternative (pin 5)**: Reads 1 (Correct - deselected state)
- **MOSI/SCK**: Properly controlled by ESP32

### Root Cause
The MISO pin state of 0 indicates:
1. **SD card is not responding** - Card may be faulty or not properly inserted
2. **Wiring issue** - MISO line may be disconnected or shorted
3. **Wrong pin assignment** - SD card slot may not be connected to pin 12
4. **Power issue** - SD card may not be receiving proper power

## Recommended Hardware Checks

### 1. Physical Inspection
- [ ] Verify SD card is properly inserted and locked in place
- [ ] Check for visible damage to SD card or slot
- [ ] Ensure SD card is not write-protected (physical switch)

### 2. Power Verification
- [ ] Measure 3.3V supply to SD card slot
- [ ] Check ground connections
- [ ] Verify current capacity (SD cards can draw 100-200mA during initialization)

### 3. Wiring Verification
```
Standard ESP32 SD Card Connections:
- VCC  → 3.3V
- GND  → GND
- MISO → GPIO 12
- MOSI → GPIO 13
- SCK  → GPIO 14
- CS   → GPIO 15
```

### 4. Test with Different Hardware
- [ ] Try different SD card (known working)
- [ ] Try different ESP32 board
- [ ] Use multimeter to check continuity between ESP32 pins and SD card slot

### 5. Alternative Pin Configuration
Some ESP32 boards use different pin assignments:
```
Alternative Configuration:
- MISO → GPIO 2
- MOSI → GPIO 15
- SCK  → GPIO 14
- CS   → GPIO 13
```

## Software Workarounds Implemented

The firmware now includes:
1. **Extensive fallback logic** - Device continues without SD card
2. **Periodic retry attempts** - Attempts to initialize SD card every 5 minutes
3. **Graceful degradation** - Core functionality (WiFi, Home Assistant) works without SD card
4. **Clear error reporting** - Detailed diagnostic information in serial output

## Testing SD Card Separately

Use the included `sd_card_test.ino` sketch to isolate SD card issues:
1. Upload `sd_card_test.ino` to ESP32
2. Open Serial Monitor
3. Follow diagnostic output to identify specific hardware issues

## Next Steps

1. **Hardware Fix**: Address the MISO pin connectivity issue
2. **Alternative Storage**: Consider using SPIFFS/LittleFS for configuration storage
3. **External SD Module**: Use a separate SD card module with verified connections

## Device Status
- **WiFi**: ✅ Working
- **Home Assistant Integration**: ✅ Working  
- **OTA Updates**: ✅ Working
- **Touch Interface**: ✅ Working
- **SD Card**: ❌ Hardware issue (MISO pin not responding)

The device is fully functional except for SD card-dependent features (custom UI configuration, local file storage).
