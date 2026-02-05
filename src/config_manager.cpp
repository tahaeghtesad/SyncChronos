/**
 * Configuration Manager Implementation
 * 
 * Persistent configuration storage using LittleFS + JSON
 */

#include "config_manager.h"
#include "config.h"

#include <LittleFS.h>
#include <ArduinoJson.h>

#define CONFIG_FILE "/config.json"

// Global instance
ConfigManager configManager;

ConfigManager::ConfigManager() : _initialized(false) {
    setDefaults();
}

bool ConfigManager::begin() {
    Serial.println("ConfigManager: Initializing LittleFS...");
    
    if (!LittleFS.begin()) {
        Serial.println("ConfigManager: LittleFS mount failed, formatting...");
        LittleFS.format();
        if (!LittleFS.begin()) {
            Serial.println("ConfigManager: LittleFS format failed!");
            return false;
        }
    }
    
    _initialized = true;
    
    // Try to load existing config
    if (!load()) {
        Serial.println("ConfigManager: No config found, using defaults");
        save();  // Save defaults
    }
    
    return true;
}

bool ConfigManager::load() {
    if (!_initialized) return false;
    
    File file = LittleFS.open(CONFIG_FILE, "r");
    if (!file) {
        Serial.println("ConfigManager: Config file not found");
        return false;
    }
    
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, file);
    file.close();
    
    if (error) {
        Serial.printf("ConfigManager: JSON parse error: %s\n", error.c_str());
        return false;
    }
    
    // Load values with defaults as fallback
    strlcpy(_config.deviceName, doc["deviceName"] | "VFD Clock", sizeof(_config.deviceName));
    strlcpy(_config.wifiSsid, doc["wifiSsid"] | WIFI_SSID, sizeof(_config.wifiSsid));
    strlcpy(_config.wifiPassword, doc["wifiPassword"] | WIFI_PASSWORD, sizeof(_config.wifiPassword));
    strlcpy(_config.ntpServer, doc["ntpServer"] | NTP_SERVER, sizeof(_config.ntpServer));
    _config.timezoneOffset = doc["timezoneOffset"] | UTC_OFFSET_SECONDS;
    _config.brightness = doc["brightness"] | VFD_DEFAULT_BRIGHTNESS;
    _config.showSeconds = doc["showSeconds"] | true;
    strlcpy(_config.weatherApiKey, doc["weatherApiKey"] | WEATHER_API_KEY, sizeof(_config.weatherApiKey));
    _config.weatherLat = doc["weatherLat"] | WEATHER_LAT;
    _config.weatherLon = doc["weatherLon"] | WEATHER_LON;
    strlcpy(_config.weatherUnits, doc["weatherUnits"] | WEATHER_UNITS, sizeof(_config.weatherUnits));
    _config.weatherUpdateInterval = doc["weatherUpdateInterval"] | WEATHER_UPDATE_INTERVAL;
    
    // Weather display timing (defaults: start 8-28s, duration 15-25s)
    _config.weatherDisplayStartMin = doc["weatherDisplayStartMin"] | 8;
    _config.weatherDisplayStartMax = doc["weatherDisplayStartMax"] | 28;
    _config.weatherDurationMin = doc["weatherDurationMin"] | 15;
    _config.weatherDurationMax = doc["weatherDurationMax"] | 25;
    
    Serial.println("ConfigManager: Config loaded successfully");
    Serial.printf("  Device: %s\n", _config.deviceName);
    Serial.printf("  WiFi SSID: %s\n", _config.wifiSsid);
    Serial.printf("  Timezone: %ld\n", _config.timezoneOffset);
    
    return true;
}

bool ConfigManager::save() {
    if (!_initialized) return false;
    
    JsonDocument doc;
    
    doc["deviceName"] = _config.deviceName;
    doc["wifiSsid"] = _config.wifiSsid;
    doc["wifiPassword"] = _config.wifiPassword;
    doc["ntpServer"] = _config.ntpServer;
    doc["timezoneOffset"] = _config.timezoneOffset;
    doc["brightness"] = _config.brightness;
    doc["showSeconds"] = _config.showSeconds;
    doc["weatherApiKey"] = _config.weatherApiKey;
    doc["weatherLat"] = _config.weatherLat;
    doc["weatherLon"] = _config.weatherLon;
    doc["weatherUnits"] = _config.weatherUnits;
    doc["weatherUpdateInterval"] = _config.weatherUpdateInterval;
    doc["weatherDisplayStartMin"] = _config.weatherDisplayStartMin;
    doc["weatherDisplayStartMax"] = _config.weatherDisplayStartMax;
    doc["weatherDurationMin"] = _config.weatherDurationMin;
    doc["weatherDurationMax"] = _config.weatherDurationMax;
    
    File file = LittleFS.open(CONFIG_FILE, "w");
    if (!file) {
        Serial.println("ConfigManager: Failed to open config file for writing");
        return false;
    }
    
    serializeJson(doc, file);
    file.close();
    
    Serial.println("ConfigManager: Config saved");
    return true;
}

void ConfigManager::reset() {
    setDefaults();
    if (_initialized) {
        save();
    }
    Serial.println("ConfigManager: Config reset to defaults");
}

void ConfigManager::setDefaults() {
    strlcpy(_config.deviceName, "VFD Clock", sizeof(_config.deviceName));
    strlcpy(_config.wifiSsid, WIFI_SSID, sizeof(_config.wifiSsid));
    strlcpy(_config.wifiPassword, WIFI_PASSWORD, sizeof(_config.wifiPassword));
    strlcpy(_config.ntpServer, NTP_SERVER, sizeof(_config.ntpServer));
    _config.timezoneOffset = UTC_OFFSET_SECONDS;
    _config.brightness = VFD_DEFAULT_BRIGHTNESS;
    _config.showSeconds = true;
    strlcpy(_config.weatherApiKey, WEATHER_API_KEY, sizeof(_config.weatherApiKey));
    _config.weatherLat = WEATHER_LAT;
    _config.weatherLon = WEATHER_LON;
    strlcpy(_config.weatherUnits, WEATHER_UNITS, sizeof(_config.weatherUnits));
    _config.weatherUpdateInterval = WEATHER_UPDATE_INTERVAL;
    _config.weatherDisplayStartMin = 8;
    _config.weatherDisplayStartMax = 28;
    _config.weatherDurationMin = 15;
    _config.weatherDurationMax = 25;
}

void ConfigManager::setDeviceName(const char* name) {
    strlcpy(_config.deviceName, name, sizeof(_config.deviceName));
}

void ConfigManager::setWifiCredentials(const char* ssid, const char* password) {
    strlcpy(_config.wifiSsid, ssid, sizeof(_config.wifiSsid));
    strlcpy(_config.wifiPassword, password, sizeof(_config.wifiPassword));
}

void ConfigManager::setNtpServer(const char* server) {
    strlcpy(_config.ntpServer, server, sizeof(_config.ntpServer));
}

void ConfigManager::setTimezoneOffset(long offset) {
    _config.timezoneOffset = offset;
}

void ConfigManager::setBrightness(uint8_t brightness) {
    _config.brightness = brightness;
}

void ConfigManager::setWeatherApiKey(const char* key) {
    strlcpy(_config.weatherApiKey, key, sizeof(_config.weatherApiKey));
}

void ConfigManager::setWeatherLocation(float lat, float lon) {
    _config.weatherLat = lat;
    _config.weatherLon = lon;
}

void ConfigManager::setWeatherUnits(const char* units) {
    strlcpy(_config.weatherUnits, units, sizeof(_config.weatherUnits));
}
