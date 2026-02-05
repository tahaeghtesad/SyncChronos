/**
 * VFD Clock Configuration
 *
 * Hardware and software configuration settings
 */

#ifndef CONFIG_H
#define CONFIG_H

// =============================================================================
// WiFi Configuration
// =============================================================================
// Can be overridden via build_flags in platformio.ini
#ifndef WIFI_SSID
#define WIFI_SSID "Geely"
#endif

#ifndef WIFI_PASSWORD
#define WIFI_PASSWORD "DingDong"
#endif

#define WIFI_CONNECT_TIMEOUT 15000 // 15 seconds

// =============================================================================
// NTP Configuration
// =============================================================================
#define NTP_SERVER "pool.ntp.org"
#define NTP_UPDATE_INTERVAL 3600000 // 1 hour in ms
#define UTC_OFFSET_SECONDS -28800   // PST (UTC-8), adjust for your timezone

// =============================================================================
// VFD Display Pin Configuration (ESP8266)
// =============================================================================
// FUTABA 8-MD-06INKM uses SPI interface
// Adjust these pins based on your wiring

#define VFD_PIN_CS D8   // GPIO15 - Chip Select (directly adjacent to the clock)
#define VFD_PIN_CLK D5  // GPIO14 - SPI Clock (HSCLK)
#define VFD_PIN_DATA D7 // GPIO13 - SPI MOSI (HMOSI)
#define VFD_PIN_RST D6  // GPIO12 - Reset (optional, set to -1 if not used)

// Alternative pin definitions using GPIO numbers
// #define VFD_PIN_CS    15
// #define VFD_PIN_CLK   14
// #define VFD_PIN_DATA  13
// #define VFD_PIN_RST   12

// =============================================================================
// VFD Display Settings
// =============================================================================
#define VFD_NUM_DIGITS 8           // 8-MD-06INKM has 8 digits
#define VFD_DEFAULT_BRIGHTNESS 200 // 0-240 (240 = max brightness)
#define VFD_SPI_SPEED 1000000      // 1 MHz SPI clock

// =============================================================================
// Display Update Settings
// =============================================================================
#define DISPLAY_UPDATE_INTERVAL 100 // Update display every 100ms

// =============================================================================
// Debug Settings
// =============================================================================
#define DEBUG_SERIAL true
#define DEBUG_BAUD_RATE 115200

// =============================================================================
// Weather Configuration (OpenWeatherMap)
// =============================================================================
// Get a free API key from: https://openweathermap.org/api
#ifndef WEATHER_API_KEY
#define WEATHER_API_KEY "a964c5573241cc845e8d53941e28b6a0"
#endif

// Sunnyvale Heritage District coordinates
#define WEATHER_LAT 37.3688
#define WEATHER_LON -122.0363

// Update interval (10 minutes recommended to stay within free tier limits)
#define WEATHER_UPDATE_INTERVAL 600000  // 10 minutes in ms

// Units: "metric" for Celsius, "imperial" for Fahrenheit
#define WEATHER_UNITS "imperial"

#endif // CONFIG_H
