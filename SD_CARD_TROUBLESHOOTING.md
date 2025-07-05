# SD Card Troubleshooting Guide

## Common SD Card Issues and Solutions

### 1. SD Card Write Protection
**Problem**: ESP32 can read files but cannot write to SD card
**Symptoms**: 
- OTA updates fail with "SD card write test failed"
- Configuration files cannot be updated
- Icons display red placeholders

**Solutions**:
1. **Check physical write protection**: Ensure the SD card's write-protect switch is in the "unlocked" position
2. **Reformat the card**: Format the SD card as FAT32 on your computer
3. **Check file system**: Use `chkdsk` (Windows) or `fsck` (Linux) to repair file system errors
4. **Test with different card**: Try a different SD card to rule out hardware issues

### 2. SD Card File System Issues
**Problem**: Files exist but cannot be read or written
**Symptoms**:
- "Failed to open root directory" errors
- Icons show yellow placeholders (file exists but can't be opened)
- Configuration files fail to load

**Solutions**:
1. **Backup and reformat**: Copy all files to computer, format SD card as FAT32, copy files back
2. **Check file names**: Ensure file names are 8.3 format compatible (no long names)
3. **Verify file integrity**: Re-download or re-copy all files to SD card

### 3. Hardware Connection Issues
**Problem**: SD card not detected or intermittent failures
**Symptoms**:
- "SD Card initialization failed" error
- Random read/write failures
- Icons sometimes work, sometimes don't

**Solutions**:
1. **Check wiring**: Verify all SD card connections are secure
2. **Power supply**: Ensure stable 3.3V supply to SD card
3. **Add pull-up resistors**: Add 10kΩ pull-up resistors on data lines if needed
4. **Reduce SPI speed**: Lower the SPI clock speed in initialization

### 4. File Format Issues
**Problem**: Files are corrupted or wrong format
**Symptoms**:
- SVG icons don't display properly
- Configuration files cause JSON parsing errors
- Files exist but contain unexpected data

**Solutions**:
1. **Re-download files**: Download fresh copies of all SD card files
2. **Check file encoding**: Ensure files are saved with UTF-8 encoding
3. **Verify file sizes**: Compare file sizes with repository versions

## Diagnostic Steps

1. **Check Serial Monitor**: Look for detailed error messages during boot
2. **Test SD Card on Computer**: Verify the card works normally on a PC
3. **Check File Listing**: Look for the SD card contents listing in Serial output
4. **Test Write Capability**: Check if the SD card write test passes during boot
5. **Verify Icon Files**: Ensure all required SVG icon files are present in `/icons/` directory

## Required SD Card Files

Ensure these files exist on your SD card:
- `/config.json` - Main configuration file
- `/ui_config.json` - UI layout configuration
- `/icons/wifi_full.svg` - WiFi full signal icon
- `/icons/wifi_good.svg` - WiFi good signal icon
- `/icons/wifi_weak.svg` - WiFi weak signal icon
- `/icons/wifi_poor.svg` - WiFi poor signal icon
- `/icons/wifi_off.svg` - WiFi off icon
- `/icons/thermometer.svg` - Temperature sensor icon

## SD Card Recommendations

- **Size**: 1GB to 32GB (larger cards may have compatibility issues)
- **Speed**: Class 10 or higher recommended
- **Format**: FAT32 file system
- **Brand**: Use reputable brands (SanDisk, Samsung, etc.)
- **Avoid**: Avoid micro-SD adapters if possible, use full-size SD cards

## Debug Output

Enable debug output in Serial Monitor (115200 baud) to see:
- SD card initialization status
- File listing during boot
- Icon loading attempts
- Write test results
- Error messages with specific failure reasons
