# OTA Update System Documentation

## Overview
The ESP32 Home Assistant thermostat controller includes an Over-The-Air (OTA) update system that automatically checks for and installs firmware updates from a public GitHub repository.

## How It Works

### 1. Version Checking
- The device checks for updates every 6 hours
- It downloads `version.json` from the configured GitHub repository
- Compares the remote version with the current firmware version
- If a newer version is available, it initiates the update process

### 2. Update Process
- Downloads the firmware binary file from the GitHub repository
- Shows progress on the TFT display with a progress bar
- Writes the firmware to flash memory using ESP32's Update library
- Automatically restarts the device after successful update

### 3. Error Handling
- Network errors are logged to Serial and shown on display
- Invalid firmware files are rejected
- Insufficient space warnings are displayed
- Failed updates show error messages on screen

## Configuration

### GitHub Repository Setup
1. Create a public GitHub repository (or use an existing one)
2. Upload the following files to the repository root:
   - `version.json` - Contains version information
   - `firmware.bin` - The compiled firmware binary

### version.json Format
```json
{
  "version": "1.0.1",
  "firmware_url": "https://raw.githubusercontent.com/yourusername/esp32-homeassistant/main/firmware.bin",
  "release_notes": "Bug fixes and improvements",
  "min_version": "1.0.0",
  "force_update": false
}
```

### Fields Explanation
- `version`: The version number of the firmware (semantic versioning recommended)
- `firmware_url`: Direct URL to the firmware binary file
- `release_notes`: Description of changes (optional)
- `min_version`: Minimum required version for update (optional)
- `force_update`: Whether to force update regardless of version (optional)

### Code Configuration
Update these constants in your Arduino sketch:

```cpp
#define FIRMWARE_VERSION "1.0.0"
#define OTA_UPDATE_URL "https://raw.githubusercontent.com/yourusername/esp32-homeassistant/main/version.json"
#define OTA_FIRMWARE_URL "https://raw.githubusercontent.com/yourusername/esp32-homeassistant/main/firmware.bin"
```

Replace `yourusername` and `esp32-homeassistant` with your actual GitHub username and repository name.

## Building and Uploading Firmware

### 1. Build the Firmware
1. Open your project in Arduino IDE
2. Go to `Sketch` → `Export compiled Binary`
3. The firmware.bin file will be created in your sketch folder

### 2. Upload to GitHub
1. Rename the compiled binary to `firmware.bin`
2. Upload it to your GitHub repository root
3. Update the version number in `version.json`
4. Commit and push the changes

### 3. Version Management
- Always increment the version number when uploading new firmware
- Use semantic versioning (e.g., 1.0.0, 1.0.1, 1.1.0)
- The device will only update if the remote version is newer

## Security Considerations

### Why Public Repository?
- No authentication tokens needed
- Simple and reliable
- GitHub provides good CDN performance
- Easy to manage and version control

### Risks and Mitigations
- **Risk**: Anyone can see your firmware
- **Mitigation**: Don't include sensitive credentials in firmware (use SD card config)

- **Risk**: Malicious firmware replacement
- **Mitigation**: Use a private repository if security is critical, or implement firmware signing

## Troubleshooting

### Common Issues

1. **Update Check Fails**
   - Check WiFi connection
   - Verify GitHub repository URL is correct
   - Ensure version.json is accessible publicly

2. **Firmware Download Fails**
   - Check firmware.bin file exists in repository
   - Verify file is not corrupted
   - Ensure sufficient free space on device

3. **Update Process Hangs**
   - Check serial output for detailed error messages
   - Verify firmware.bin is valid for your ESP32 model
   - Try a smaller firmware size

### Debug Information
The device outputs detailed debug information to Serial during OTA updates:
- Version comparison results
- Download progress
- Error messages
- Update success/failure status

### Manual Update Check
To force an immediate update check, you can modify the code to call `checkForOTAUpdate()` directly, or reset the `lastOTACheck` timer.

## Best Practices

1. **Test Updates**: Always test firmware updates on a development device first
2. **Backup**: Keep previous working firmware versions in your repository
3. **Gradual Rollout**: Consider version-specific rollouts for critical updates
4. **Monitor**: Watch serial output during development for update issues
5. **Recovery**: Keep a way to manually flash firmware if OTA fails

## Example Workflow

1. **Development**:
   - Write and test new features
   - Update `FIRMWARE_VERSION` in code
   - Compile and test locally

2. **Release**:
   - Export compiled binary
   - Rename to `firmware.bin`
   - Upload to GitHub repository
   - Update `version.json` with new version number
   - Commit and push changes

3. **Deployment**:
   - Devices will automatically check for updates
   - Updates will be applied within 6 hours
   - Monitor serial output for any issues

This OTA system provides a robust, maintenance-free way to keep your ESP32 devices updated without requiring physical access or complex infrastructure.
