# ESP32 Home Assistant Thermostat Controller

A modern touchscreen thermostat controller for Home Assistant using ESP32-2432S028R with customizable UI loaded from SD card.

## Features

- **Customizable UI**: All interface elements defined in JSON configuration files
- **Home Assistant Integration**: Full climate control and monitoring
- **SD Card Configuration**: All settings and UI loaded from microSD to minimize flash usage
- **OTA Updates**: Automatic firmware updates from GitHub repository
- **WiFi Management**: Robust connection handling with detailed debug output
- **Touch & Rotary Input**: Support for both touch and rotary encoder temperature adjustment
- **Memory Optimized**: Designed for ESP32 flash constraints with external asset loading

## Hardware Requirements

- **ESP32-2432S028R** (CYD - Cheap Yellow Display)
- **Display**: ILI9341 TFT (320x240)
- **Touch**: XPT2046 resistive touchscreen
- **MicroSD Card**: FAT32 formatted (1-32GB recommended)
- **Optional**: Rotary encoder for temperature adjustment

## Software Dependencies

### Arduino Libraries (install via Library Manager):
1. **TFT_eSPI** - Bodmer's TFT library
2. **XPT2046_Touchscreen** - Paul Stoffregen's touch library
3. **ArduinoJson** - Benoit Blanchon's JSON library
4. **WiFi** - ESP32 Core (built-in)
5. **HTTPClient** - ESP32 Core (built-in)
6. **SD** - ESP32 Core (built-in)
7. **Update** - ESP32 Core (built-in, for OTA updates)

## Setup Instructions

### 1. Arduino IDE Configuration
1. Install ESP32 board support
2. Install required libraries via Tools > Manage Libraries
3. Configure TFT_eSPI for your display (see display documentation)

### 2. SD Card Preparation
1. Format microSD card as FAT32
2. Copy configuration files from `sd_files/` directory to SD card root:
   - `config.json` - Device configuration (WiFi, Home Assistant, etc.)
   - `ui_config.json` - UI layout and styling definitions
3. Insert SD card into ESP32 device

### 3. Configuration Setup
1. Edit `config.json` on SD card with your WiFi and Home Assistant details
2. Customize `ui_config.json` to match your preferred interface design
3. Upload the sketch to ESP32
4. Device will connect to WiFi and start functioning immediately

## Automated Build and Release System

The project includes a complete CI/CD pipeline using GitHub Actions that automatically builds and releases firmware when code changes are made.

### Features:
- **Automatic Builds**: Triggered on every push to the main branch
- **Version Management**: Automatically generates version numbers based on date and build count
- **Firmware Compilation**: Uses Arduino CLI to build firmware in the cloud
- **OTA Updates**: Automatically updates `version.json` and `firmware.bin` files
- **GitHub Releases**: Creates releases with firmware binaries and release notes
- **Pull Request Testing**: Tests builds before merging changes

### How It Works:
1. **Code Changes**: Push code changes to the `main` branch
2. **Automatic Build**: GitHub Actions compiles the firmware
3. **Version Update**: Updates `FIRMWARE_VERSION` in the source code automatically
4. **Release Creation**: Creates a GitHub release with the new firmware
5. **OTA Deployment**: Devices automatically update within 6 hours

### Version Format:
- **Automatic**: `YYYY.MM.DD.BUILD_NUMBER` (e.g., `2025.01.15.42`)
- **Manual**: `X.Y.Z` semantic versioning (e.g., `1.2.3`)

### Manual Releases:
For urgent updates or specific version numbers:
1. Go to the "Actions" tab in your GitHub repository
2. Run the "Manual Release" workflow
3. Specify version number and release notes
4. Enable "Force Update" for critical security updates

### Build Status:
- ✅ **Automatic**: Builds trigger on code pushes
- 🧪 **Testing**: Pull requests are tested before merge
- 📦 **Releases**: Firmware binaries available for download
- 🔄 **OTA**: Devices update automatically

For detailed information about the build system, see `.github/workflows/README.md`.

## OTA Update System

The device includes an automatic Over-The-Air update system that checks for new firmware versions from a public GitHub repository.

### Features:
- **Automatic Updates**: Checks for updates every 6 hours
- **Progress Display**: Shows download progress on the TFT screen
- **Error Handling**: Robust error detection and recovery
- **Version Management**: Semantic versioning with minimum version requirements
- **No Authentication**: Works with public GitHub repositories
- **GitHub Integration**: Seamlessly integrated with the automated build system

### Setup:
1. Fork or create a public GitHub repository
2. Update the OTA URLs in the code:
   ```cpp
   #define OTA_UPDATE_URL "https://raw.githubusercontent.com/yourusername/esp32-homeassistant/main/version.json"
   #define OTA_FIRMWARE_URL "https://raw.githubusercontent.com/yourusername/esp32-homeassistant/main/firmware.bin"
   ```
3. GitHub Actions will automatically handle building and publishing updates

For detailed OTA setup instructions, see `OTA_UPDATE_GUIDE.md`.

## Configuration Files

### config.json
```json
{
  "wifi": {
    "ssid": "YourWiFiNetwork",
    "password": "YourWiFiPassword"
  },
  "homeassistant": {
    "host": "192.168.1.100",
    "port": 8123,
    "token": "your_long_lived_access_token",
    "climate_entity": "climate.your_thermostat",
    "temp_sensor": "sensor.your_temperature_sensor"
  },
  "device": {
    "name": "ESP32 Thermostat",
    "temp_unit": "F",
    "temp_min": 50,
    "temp_max": 90
  }
}
```

### ui_config.json
```json
{
  "theme": {
    "background_color": "#000000",
    "text_color": "#FFFFFF",
    "accent_color": "#FF6B35"
  },
  "layout": {
    "current_temp": {"x": 20, "y": 20, "width": 120, "height": 80},
    "target_temp": {"x": 180, "y": 20, "width": 120, "height": 80},
    "controls": {"x": 20, "y": 120, "width": 280, "height": 100}
  }
}
```

## Initial Configuration
1. Edit `config.json` on the SD card with your network and Home Assistant details
2. Customize `ui_config.json` for your preferred appearance
3. Insert SD card and power on the device
4. Monitor Serial output for connection status and debug information

## Troubleshooting

### Common Issues:
1. **Config file errors**: Check Serial output for JSON parsing errors
2. **WiFi connection**: Verify credentials and check debug output
3. **Home Assistant connection**: Ensure token is valid and entity names are correct
4. **OTA updates**: Check GitHub repository URLs and network connectivity

### Debug Output:
The device provides detailed debug information via Serial monitor for:
- WiFi connection status and errors
- Home Assistant API calls and responses
- OTA update progress and errors
- Configuration file parsing results

## Development

### Automated Development Workflow:
1. **Create Feature Branch**: `git checkout -b feature/new-feature`
2. **Make Changes**: Edit code in `sketch_jul3a/` directory
3. **Create Pull Request**: GitHub Actions will test the build automatically
4. **Merge to Main**: After approval, merge triggers automatic release
5. **Automatic Deployment**: Devices update within 6 hours

### Manual Development Workflow:
1. **Local Testing**: Build and test on development hardware
2. **Manual Release**: Use GitHub Actions "Manual Release" workflow
3. **Version Control**: Specify exact version numbers for releases
4. **Emergency Updates**: Use force update for critical fixes

### Build Process:
- **Local**: Use Arduino IDE or `release.ps1`/`release.bat` scripts
- **Automated**: GitHub Actions handles compilation and release
- **Testing**: Pull requests are automatically tested before merge

### Version Management:
- **Automatic Builds**: Date-based versioning (YYYY.MM.DD.BUILD)
- **Manual Releases**: Semantic versioning (X.Y.Z)
- **Source Code**: `FIRMWARE_VERSION` is automatically updated

### Building and Testing:
1. Make code changes in `sketch_jul3a/sketch_jul3a.ino`
2. Create pull request to test compilation
3. Merge to main branch for automatic release
4. Monitor GitHub Actions for build status
5. Check device logs for successful OTA updates

### Memory Optimization:
- All UI assets loaded from SD card
- Minimal flash usage for core functionality
- Efficient JSON parsing and memory management
- Configurable features to reduce memory footprint

## License

This project is licensed under the MIT License - see the LICENSE file for details.

## Contributing

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/new-feature`)
3. Make your changes
4. Test thoroughly (pull request will test automatically)
5. Submit a pull request
6. After approval, merge triggers automatic release

## Support

For issues and questions:
1. Check the troubleshooting section above
2. Review Serial debug output
3. Check GitHub Actions build logs
4. Create an issue on GitHub with detailed information

## Version Management

This project uses a manual version control system:

### How to Release a New Version

1. **Update Version in Code**: 
   - Edit `sketch_jul3a/sketch_jul3a.ino`
   - Change the `FIRMWARE_VERSION` define to your desired version (e.g., "2025.01.10.49")
   - Use format: `YYYY.MM.DD.BUILD` (e.g., 2025.01.10.49)

2. **Commit and Push**:
   - Commit your changes to the main branch
   - GitHub Actions will automatically:
     - Read the version from your sketch file
     - Build firmware using YOUR version number
     - Update `version.json` to match your version
     - Create release artifacts

3. **Manual Release** (optional):
   - Use GitHub Actions "Manual Release" workflow
   - Specify version and release notes
   - This will build firmware and create a GitHub release

### Important Notes
- **The sketch file version is the master version** - workflows read from it, never modify it
- Version format should be `YYYY.MM.DD.BUILD` (e.g., 2025.01.10.48)
- GitHub Actions will NOT change your sketch file - only build from it
- `version.json` is automatically updated to match your sketch version
