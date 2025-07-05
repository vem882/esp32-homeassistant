# SD Card Files for ESP32 Thermostat

This directory contains the HTML files that must be copied to your microSD card for the ESP32 thermostat web configuration portal.

## Required Files

### config.html
The main configuration web page with:
- Modern, responsive design with CSS gradients and animations
- WiFi network scanning and credential input
- Home Assistant connection settings (IP, port, token)
- Entity configuration for temperature, climate, and weather sensors
- Professional styling with hover effects and transitions

### success.html  
Confirmation page displayed after successful configuration with:
- Visual success confirmation with animated checkmark
- Auto-refresh countdown and restart notification
- Matching professional styling

## Installation Instructions

1. **Format SD Card**: Use FAT32 file system (1-32GB recommended)
2. **Copy Files**: Copy both `config.html` and `success.html` to the **root directory** of your SD card
3. **Verify Names**: Ensure file names are exactly as shown (case-sensitive)
4. **Insert Card**: Place the SD card in your ESP32-2432S028R device before powering on

## File Verification

The ESP32 device will automatically check for these files on startup:
- If files are missing, a notification will appear on the device screen
- Missing files are logged to the serial monitor
- The device will fall back to minimal built-in HTML if files are not found

## Customization

You can modify these HTML files to customize the appearance and functionality:
- Edit the CSS styles for different colors and layouts
- Add or remove form fields as needed
- Modify JavaScript functions for enhanced features

After making changes, simply copy the updated files to the SD card and restart the ESP32.

## File Sizes

- `config.html`: ~7KB (full-featured configuration page)
- `success.html`: ~2KB (confirmation page)
- **Total**: ~9KB (saves significant ESP32 flash memory)

## Troubleshooting

- **Files not loading**: Check file names and ensure they're in the root directory
- **SD card not detected**: Verify FAT32 formatting and proper insertion
- **Web page looks basic**: Files may be corrupted, re-copy from this directory

For more detailed setup instructions, see `../SD_CARD_SETUP.md`.
