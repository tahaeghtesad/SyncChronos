# VFD Clock

An ESP8266-based clock featuring the **FUTABA 8-MD-06INKM** dot matrix VFD display.

![VFD Clock](docs/images/clock.jpg)

## Features

- 🕐 **NTP Time Sync** - Automatic time synchronization via WiFi
- 🌍 **Timezone Support** - Configurable UTC offset
- 💡 **Brightness Control** - Adjustable via serial commands
- 📺 **Multiple Display Modes** - Time, date, seconds
- 🔌 **Low Power Standby** - VFD standby mode support

## Hardware

### Components

| Component | Description |
|-----------|-------------|
| ESP8266 | NodeMCU v2 or similar |
| VFD Display | FUTABA 8-MD-06INKM (8-digit dot matrix) |
| Power Supply | 5V USB for ESP8266, VFD internal boost |

### Wiring

The FUTABA 8-MD-06INKM uses SPI communication:

| VFD Pin | ESP8266 Pin | GPIO |
|---------|-------------|------|
| GND | GND | - |
| VCC | 3.3V | - |
| CS | D8 | GPIO15 |
| CLK | D5 | GPIO14 |
| DIN (MOSI) | D7 | GPIO13 |
| RST | D6 | GPIO12 |

> **Note**: The VFD module typically has onboard voltage regulation for the filament and anode voltages.

## Software Setup

### Prerequisites

1. Install [PlatformIO](https://platformio.org/install)
2. Clone this repository

### Configuration

1. Edit `src/config.h` and set your WiFi credentials:
   ```cpp
   #define WIFI_SSID "your_network_name"
   #define WIFI_PASSWORD "your_password"
   ```

2. Adjust timezone offset (in seconds from UTC):
   ```cpp
   #define UTC_OFFSET_SECONDS -28800  // PST (UTC-8)
   ```

### Building & Uploading

```bash
# Build the project
pio run

# Upload to ESP8266
pio run --target upload

# Monitor serial output
pio device monitor
```

Or use the PlatformIO IDE extension in VS Code.

## Serial Commands

Connect at 115200 baud to control the clock:

| Key | Action |
|-----|--------|
| `t` | Switch to time mode |
| `d` | Switch to date mode |
| `s` | Switch to time with seconds |
| `+` | Increase brightness |
| `-` | Decrease brightness |
| `r` | Force NTP resync |

## Project Structure

```
VFDClock/
├── platformio.ini      # PlatformIO configuration
├── src/
│   ├── main.cpp        # Main application
│   ├── config.h        # Configuration settings
│   ├── vfd_driver.h    # VFD driver header
│   ├── vfd_driver.cpp  # VFD driver implementation
│   ├── time_manager.h  # NTP time manager header
│   ├── time_manager.cpp
│   ├── wifi_manager.h  # WiFi connection manager
│   └── wifi_manager.cpp
├── lib/                # Project-specific libraries
├── include/            # Project header files
├── test/               # Unit tests
└── docs/               # Documentation
```

## VFD Display Details

The FUTABA 8-MD-06INKM specifications:
- **Type**: Dot matrix VFD (vacuum fluorescent display)
- **Digits**: 8 characters
- **Color**: Blue-green phosphor
- **Interface**: SPI (Mode 3, LSB first)
- **Controller**: PT6301 or compatible
- **Voltage**: 3.3V logic compatible

## Troubleshooting

### Display not working
1. Check wiring connections
2. Verify VFD power supply
3. Check SPI pin assignments in `config.h`

### Time not syncing
1. Verify WiFi credentials
2. Check internet connectivity
3. Try a different NTP server

### Garbled display
1. Reduce SPI speed in `config.h`
2. Check for loose connections
3. Verify SPI mode settings

## License

MIT License - See [LICENSE](LICENSE) for details.

## Acknowledgments

- FUTABA for the beautiful VFD display
- ESP8266 community for the Arduino core
