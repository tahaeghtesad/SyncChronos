# VFD Clock & LED Matrix Display

An ESP8266-based Internet Clock supporting both **FUTABA 8-MD-06INKM** VFD displays and **MAX7219** LED Matrix modules.

![VFD Clock](docs/images/clock.jpg)

## Features

- 🕐 **NTP Time Sync** - Automatic time synchronization via WiFi
- �️ **Live Weather** - Real-time temperature and conditions from OpenWeatherMap
- 🌍 **Web Configuration** - Configure WiFi, API keys, and settings via a web portal
- 💡 **Brightness Control** - Adjustable brightness (VFD and LED Matrix)
- 📺 **Dual Display Support** - Compile-time selection for VFD or MAX7219
- 🔌 **Activity Indicators** - Visual feedback for network/weather interactions
- 💾 **Persistent Settings** - Configuration saved to flash memory

## Hardware Support

### 1. FUTABA 8-MD-06INKM VFD
The default display configuration.

| VFD Pin | ESP8266 Pin | GPIO | Function |
|---------|-------------|------|----------|
| GND     | GND         | -    | Ground   |
| VCC     | 3.3V        | -    | Power    |
| CS      | D8          | 15   | Chip Select |
| CLK     | D5          | 14   | Clock    |
| DIN     | D7          | 13   | MOSI     |
| RST     | D6          | 12   | Reset    |

### 2. MAX7219 LED Matrix (4x 8x8 Modules)
Alternative display option.

| MAX7219 Pin | ESP8266 Pin | GPIO | Function |
|-------------|-------------|------|----------|
| VCC         | 5V (USB)    | -    | Power    |
| GND         | GND         | -    | Ground   |
| DIN         | D7          | 13   | MOSI     |
| CS          | D8          | 15   | Chip Select |
| CLK         | D5          | 14   | Clock    |

## Software Setup

### 1. Prerequisites
- [PlatformIO](https://platformio.org/install) (VS Code Extension recommended)
- Python 3 (for verification scripts)

### 2. Configuration
The clock creates a WiFi Access Point named `VFD-Clock-Setup` on first boot. Connect to it and visit `http://192.168.4.1` to configure:
- WiFi Credentials
- OpenWeatherMap API Key
- Timezone
- Display Settings

Default settings can also be hardcoded in `src/config.h`.

### 3. Building & Uploading
Use the included `Makefile` for easy management:

```bash
# Build & Upload for VFD Display (Default)
make upload

# Build & Upload for MAX7219 Display
make upload-max

# Run Native Unit Tests (Local logic verfication)
make test-native

# Run Embedded Unit Tests (On Device)
make test-embedded

# Verify API Keys
make verify
```

### 4. Environments
The project uses PlatformIO environments to adhere to DRY principles:
- `esp8266`: Default environment for VFD.
- `esp8266_max7219`: Environment for MAX7219 (defines `-DUSE_MAX7219_DISPLAY`).
- `native`: Environment for running unit tests on the host machine.

## Testing

The project includes a robust test suite:

### Native Tests (Host)
Runs on your computer to verify logic (JSON parsing, config serialization) without hardware.
```bash
make test-native
```

### Embedded Tests (Device)
Runs on the ESP8266 to verify hardware integration (WiFi, Display drivers).
```bash
make test-embedded
```

## Serial Commands
Connect at **115200 baud**.

| Key | Action |
|-----|--------|
| `t` | Show Time |
| `d` | Show Date |
| `w` | Show Weather |
| `+` | Increase Brightness |
| `-` | Decrease Brightness |
| `r` | Force NTP & Weather Update |
| `R` | Factory Reset Config |

## License
MIT License - See [LICENSE](LICENSE) for details.
