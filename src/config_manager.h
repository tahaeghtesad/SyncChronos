/**
 * Configuration Manager Header
 * 
 * Runtime configuration with persistent storage in LittleFS
 */

#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H

#include <Arduino.h>
#include <ArduinoJson.h>

// Maximum string lengths
#define CONFIG_STRING_MAX 64
#define CONFIG_SSID_MAX 32
#define CONFIG_PASSWORD_MAX 64
#define CONFIG_API_KEY_MAX 48

/**
 * Runtime configuration structure
 */
struct ClockConfig {
    // Device
    char deviceName[CONFIG_STRING_MAX];
    
    // WiFi (read-only after boot, requires restart to change)
    char wifiSsid[CONFIG_SSID_MAX];
    char wifiPassword[CONFIG_PASSWORD_MAX];
    
    // NTP/Time
    char ntpServer[CONFIG_STRING_MAX];
    long timezoneOffset;  // seconds from UTC
    
    // Display
    uint8_t brightness;  // 0-255
    bool showSeconds;    // Show seconds on clock face
    bool showActivityIndicators; // Blink colons during network activity
    
    // Weather
    char weatherApiKey[CONFIG_API_KEY_MAX];
    float weatherLat;
    float weatherLon;
    char weatherUnits[16];  // "metric" or "imperial"
    unsigned long weatherUpdateInterval;  // ms
    
    // Weather display timing (randomized)
    uint8_t weatherDisplayStartMin;  // Random start range min (seconds, e.g., 8)
    uint8_t weatherDisplayStartMax;  // Random start range max (seconds, e.g., 28)
    uint8_t weatherDurationMin;      // Random duration min (seconds, e.g., 15)
    uint8_t weatherDurationMax;      // Random duration max (seconds, e.g., 25)
};

/**
 * Configuration Manager
 * Handles loading/saving config to LittleFS
 */
class ConfigManager {
public:
    ConfigManager();
    
    /**
     * Initialize filesystem and load config
     * @return true if config loaded successfully
     */
    bool begin();
    
    /**
     * Load config from file
     * @return true if loaded successfully
     */
    bool load();
    
    /**
     * Save current config to file
     * @return true if saved successfully
     */
    bool save();
    
    /**
     * Reset config to defaults
     */
    void reset();
    
    // Accessors - all return references for easy modification
    ClockConfig& getConfig() { return _config; }
    const ClockConfig& getConfig() const { return _config; }
    
    // Convenience getters
    const char* getDeviceName() const { return _config.deviceName; }
    const char* getWifiSsid() const { return _config.wifiSsid; }
    const char* getWifiPassword() const { return _config.wifiPassword; }
    const char* getNtpServer() const { return _config.ntpServer; }
    long getTimezoneOffset() const { return _config.timezoneOffset; }
    uint8_t getBrightness() const { return _config.brightness; }
    bool getShowSeconds() const { return _config.showSeconds; }
    bool getShowActivityIndicators() const { return _config.showActivityIndicators; }
    const char* getWeatherApiKey() const { return _config.weatherApiKey; }
    float getWeatherLat() const { return _config.weatherLat; }
    float getWeatherLon() const { return _config.weatherLon; }
    const char* getWeatherUnits() const { return _config.weatherUnits; }
    unsigned long getWeatherUpdateInterval() const { return _config.weatherUpdateInterval; }
    uint8_t getWeatherDisplayStartMin() const { return _config.weatherDisplayStartMin; }
    uint8_t getWeatherDisplayStartMax() const { return _config.weatherDisplayStartMax; }
    uint8_t getWeatherDurationMin() const { return _config.weatherDurationMin; }
    uint8_t getWeatherDurationMax() const { return _config.weatherDurationMax; }
    
    // Convenience setters
    void setDeviceName(const char* name);
    void setWifiCredentials(const char* ssid, const char* password);
    void setNtpServer(const char* server);
    void setTimezoneOffset(long offset);
    void setBrightness(uint8_t brightness);
    void setWeatherApiKey(const char* key);
    void setWeatherLocation(float lat, float lon);
    void setWeatherUnits(const char* units);

    // Exposed for testing
    void deserializeConfig(const JsonDocument& doc);
    void serializeConfig(JsonDocument& doc) const;

private:
    ClockConfig _config;
    bool _initialized;
    
    void setDefaults();
};

// Global instance
extern ConfigManager configManager;

#endif // CONFIG_MANAGER_H
