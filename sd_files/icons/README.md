# SVG Icons for ESP32 Thermostat

This folder contains SVG icons used by the ESP32 thermostat device. The icons are loaded from the SD card and rendered on the display.

## Available Icons

### WiFi Status Icons
- `wifi_full.svg` - Full WiFi signal strength (>-50 dBm)
- `wifi_good.svg` - Good WiFi signal strength (-50 to -60 dBm)
- `wifi_weak.svg` - Weak WiFi signal strength (-60 to -70 dBm)
- `wifi_poor.svg` - Poor WiFi signal strength (<-70 dBm)
- `wifi_off.svg` - WiFi disconnected

### Other Icons
- `thermometer.svg` - Temperature icon

## Usage in ui_config.json

To use an icon in the UI configuration, add an element with type "icon":

```json
{
  "type": "icon",
  "name": "wifi_icon",
  "x": 280,
  "y": 20,
  "size": 16,
  "icon": "wifi_full",
  "color": "#00FF00"
}
```

## Icon Properties

- `name`: Unique identifier for the icon element
- `x`, `y`: Position on screen
- `size`: Icon size in pixels
- `icon`: Icon filename (without .svg extension)
- `color`: Icon color in hex format (#RRGGBB)

## Adding New Icons

1. Create a new SVG file in this folder
2. Use simple SVG elements (circles, rectangles, paths)
3. Reference the icon in ui_config.json
4. The device will automatically load and render the icon

## Color Support

Icons support dynamic color changes based on status:
- WiFi icons change color based on signal strength
- Green: Strong signal
- Yellow: Good signal
- Orange: Weak signal
- Red: Poor/No signal
