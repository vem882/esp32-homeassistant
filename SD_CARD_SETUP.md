# SD Card Setup Guide

This document describes the SD card requirements and setup for the ESP32 Home Assistant Thermostat Controller.

## Overview

The device uses a microSD card to store HTML files for the configuration web portal. This approach saves valuable ESP32 flash memory by moving static assets to external storage.

## Required Files

The following files must be present on the SD card root directory:

- `/config.html` - Main configuration web page  
- `/success.html` - Success confirmation page

## SD Card Preparation

1. **Format the SD Card**
   - Use FAT32 file system
   - Recommended: 1GB - 32GB capacity
   - Ensure the card is properly formatted

2. **Copy Required Files**
   - Copy the provided HTML files from the `sd_files/` directory to the root directory of the SD card
   - Required files: `config.html` and `success.html`
   - Ensure file names are exactly as specified (case-sensitive)

3. **Insert SD Card**
   - Insert the prepared SD card into the ESP32-2432S028R device
   - Ensure proper connection before powering on

## File Contents

### config.html
Modern, responsive configuration page with:
- WiFi network selection and credentials
- Home Assistant connection settings (IP, port, token)
- Entity configuration for temperature, climate, and weather
- Network scanning functionality
- Professional styling with gradients and animations

### success.html  
Confirmation page displayed after successful configuration with:
- Visual confirmation with checkmark animation
- Auto-refresh and restart notification
- Professional styling matching the configuration page

## File Verification

The device will automatically check for required files on startup:
- If files are missing, a notification will be displayed on the device screen
- The web portal will use minimal fallback HTML if SD files are not available
- Check the serial monitor for detailed SD card status information

## Troubleshooting

### SD Card Not Detected
- Verify the SD card is properly inserted
- Check that the card is formatted as FAT32
- Try a different SD card if issues persist

### Files Not Found
- Ensure file names are exactly `config.html` and `success.html`
- Verify files are in the root directory (not in subdirectories)
- Check file permissions and ensure files are readable
- Copy files from the `sd_files/` directory in this project

### Web Portal Issues
- If the configuration page doesn't load properly, check the serial monitor
- The device will fall back to basic HTML if SD files are corrupted
- Re-copy the HTML files to the SD card if needed

## File Updates

To update the web portal interface:
1. Modify the HTML files in the `sd_files/` directory
2. Copy the updated files to the SD card
3. Restart the ESP32 device

## Flash Memory Savings

By using SD card for HTML files:
- **Before**: ~15-20KB HTML code in flash memory
- **After**: ~2KB minimal fallback in flash memory
- **Net savings**: ~13-18KB flash memory freed

This allows the device to run on ESP32 modules with smaller flash capacity while maintaining a professional web interface.
