@echo off
echo ESP32 Home Assistant OTA Release Helper
echo =====================================
echo.

REM Check if version is provided
if "%1"=="" (
    echo Usage: release.bat ^<version^> [release_notes]
    echo Example: release.bat 1.0.1 "Fixed temperature display bug"
    exit /b 1
)

set VERSION=%1
set RELEASE_NOTES=%2
if "%RELEASE_NOTES%"=="" set RELEASE_NOTES=Bug fixes and improvements

echo Building firmware for version %VERSION%...
echo.

REM Compile the sketch (adjust path as needed)
arduino-cli compile --fqbn esp32:esp32:esp32 sketch_jul3a\sketch_jul3a.ino --output-dir build

if errorlevel 1 (
    echo Compilation failed!
    pause
    exit /b 1
)

REM Copy firmware binary
copy build\sketch_jul3a.ino.bin firmware.bin

if not exist firmware.bin (
    echo Firmware binary not found!
    pause
    exit /b 1
)

echo Firmware built successfully!
echo.

REM Create version.json (basic version)
echo { > version.json
echo   "version": "%VERSION%", >> version.json
echo   "firmware_url": "https://raw.githubusercontent.com/vem882/esp32-homeassistant/main/firmware.bin", >> version.json
echo   "release_notes": "%RELEASE_NOTES%", >> version.json
echo   "min_version": "1.0.0", >> version.json
echo   "force_update": false >> version.json
echo } >> version.json

echo Version file created: version.json
echo.

echo Release Summary:
echo Version: %VERSION%
echo Release Notes: %RELEASE_NOTES%
echo.

REM Git operations
echo Adding files to git...
git add firmware.bin
git add version.json

echo Committing changes...
git commit -m "Release version %VERSION% - %RELEASE_NOTES%"

echo.
echo Files committed to git. Run 'git push' to publish the update.
echo OTA update will be available to devices within 6 hours after pushing.
echo.

pause
