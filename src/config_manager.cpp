/**
 * Configuration Manager Implementation
 * 
 * Persistent configuration storage using LittleFS + JSON
 */

#include "config_manager.h"
#include "config.h"

#ifndef NATIVE_TEST
#include <LittleFS.h>
#define CONFIG_FILE "/config.json"
#endif
#include <ArduinoJson.h>

// Debug macros
#ifdef NATIVE_TEST
#define DEBUG_PRINT(...)
#define DEBUG_PRINTLN(...)
#define DEBUG_PRINTF(...)
#else
#define DEBUG_PRINT(x) Serial.print(x)
#define DEBUG_PRINTLN(x) Serial.println(x)
#define DEBUG_PRINTF(...) Serial.printf(__VA_ARGS__)
#endif

// Global instance
ConfigManager configManager;

ConfigManager::ConfigManager() : _initialized(false) {
    setDefaults();
}

bool ConfigManager::begin() {
#ifdef NATIVE_TEST
    _initialized = true;
    return true;
#else
    DEBUG_PRINTLN("ConfigManager: Initializing LittleFS...");
    
    if (!LittleFS.begin()) {
        DEBUG_PRINTLN("ConfigManager: LittleFS mount failed, formatting...");
        LittleFS.format();
        if (!LittleFS.begin()) {
            DEBUG_PRINTLN("ConfigManager: LittleFS format failed!");
            return false;
        }
    }
    
    _initialized = true;
    
    // Try to load existing config
    if (!load()) {
        DEBUG_PRINTLN("ConfigManager: No config found, using defaults");
        save();  // Save defaults
    }
    
    return true;
#endif
}

bool ConfigManager::load() {
#ifdef NATIVE_TEST
    return false;
#else
    if (!_initialized) return false;
    
    File file = LittleFS.open(CONFIG_FILE, "r");
    if (!file) {
        DEBUG_PRINTLN("ConfigManager: Config file not found");
        return false;
    }
    
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, file);
    file.close();
    
    if (error) {
        DEBUG_PRINTF("ConfigManager: JSON parse error: %s\n", error.c_str());
        return false;
    }
    
    // Load values with defaults as fallback
    deserializeConfig(doc);
    
    DEBUG_PRINTLN("ConfigManager: Config loaded successfully");
    DEBUG_PRINTF("  Device: %s\n", _config.deviceName);
    DEBUG_PRINTF("  WiFi SSID: %s\n", _config.wifiSsid);
    DEBUG_PRINTF("  Timezone: %ld\n", _config.timezoneOffset);
    
    return true;
#endif
}

bool ConfigManager::save() {
#ifdef NATIVE_TEST
    return true;
#else
    if (!_initialized) return false;
    
    JsonDocument doc;
    
    serializeConfig(doc);
    
    File file = LittleFS.open(CONFIG_FILE, "w");
    if (!file) {
        DEBUG_PRINTLN("ConfigManager: Failed to open config file for writing");
        return false;
    }
    
    serializeJson(doc, file);
    file.close();
    
    DEBUG_PRINTLN("ConfigManager: Config saved");
    return true;
#endif
}

void ConfigManager::reset() {
    setDefaults();
    if (_initialized) {
        save();
    }
    DEBUG_PRINTLN("ConfigManager: Config reset to defaults");
}

void ConfigManager::setDefaults() {
    strncpy(_config.deviceName, "VFD Clock", sizeof(_config.deviceName) - 1);
    _config.deviceName[sizeof(_config.deviceName) - 1] = 0;
    
    strncpy(_config.wifiSsid, WIFI_SSID, sizeof(_config.wifiSsid) - 1);
    _config.wifiSsid[sizeof(_config.wifiSsid) - 1] = 0;
    
    strncpy(_config.wifiPassword, WIFI_PASSWORD, sizeof(_config.wifiPassword) - 1);
    _config.wifiPassword[sizeof(_config.wifiPassword) - 1] = 0;
    
    strncpy(_config.ntpServer, NTP_SERVER, sizeof(_config.ntpServer) - 1);
    _config.ntpServer[sizeof(_config.ntpServer) - 1] = 0;
    
    _config.timezoneOffset = UTC_OFFSET_SECONDS;
    _config.brightness = VFD_DEFAULT_BRIGHTNESS;
    _config.showSeconds = true;
    _config.showActivityIndicators = true;
    
    strncpy(_config.weatherApiKey, WEATHER_API_KEY, sizeof(_config.weatherApiKey) - 1);
    _config.weatherApiKey[sizeof(_config.weatherApiKey) - 1] = 0;
    
    _config.weatherLat = WEATHER_LAT;
    _config.weatherLon = WEATHER_LON;
    
    strncpy(_config.weatherUnits, WEATHER_UNITS, sizeof(_config.weatherUnits) - 1);
    _config.weatherUnits[sizeof(_config.weatherUnits) - 1] = 0;
    
    _config.weatherUpdateInterval = WEATHER_UPDATE_INTERVAL;
    _config.weatherDisplayStartMin = 8;
    _config.weatherDisplayStartMax = 28;
    _config.weatherDurationMin = 15;
    _config.weatherDurationMax = 25;
    
    // Clock source defaults
    _config.clockSource = 0;  // ESP8266 software clock
    
    // Tilt sensor defaults
    _config.tiltSensorPin = 0;  // Disabled
    _config.autoRotate = false;
}

void ConfigManager::setDeviceName(const char* name) {
    strncpy(_config.deviceName, name, sizeof(_config.deviceName) - 1);
    _config.deviceName[sizeof(_config.deviceName) - 1] = 0;
}

void ConfigManager::setWifiCredentials(const char* ssid, const char* password) {
    strncpy(_config.wifiSsid, ssid, sizeof(_config.wifiSsid) - 1);
    _config.wifiSsid[sizeof(_config.wifiSsid) - 1] = 0;
    
    strncpy(_config.wifiPassword, password, sizeof(_config.wifiPassword) - 1);
    _config.wifiPassword[sizeof(_config.wifiPassword) - 1] = 0;
}

void ConfigManager::setNtpServer(const char* server) {
    strncpy(_config.ntpServer, server, sizeof(_config.ntpServer) - 1);
    _config.ntpServer[sizeof(_config.ntpServer) - 1] = 0;
}

void ConfigManager::setTimezoneOffset(long offset) {
    _config.timezoneOffset = offset;
}

void ConfigManager::setBrightness(uint8_t brightness) {
    _config.brightness = brightness;
}

void ConfigManager::setWeatherApiKey(const char* key) {
    strncpy(_config.weatherApiKey, key, sizeof(_config.weatherApiKey) - 1);
    _config.weatherApiKey[sizeof(_config.weatherApiKey) - 1] = 0;
}

void ConfigManager::setWeatherLocation(float lat, float lon) {
    _config.weatherLat = lat;
    _config.weatherLon = lon;
}

void ConfigManager::setWeatherUnits(const char* units) {
    strncpy(_config.weatherUnits, units, sizeof(_config.weatherUnits) - 1);
    _config.weatherUnits[sizeof(_config.weatherUnits) - 1] = 0;
}

void ConfigManager::deserializeConfig(const JsonDocument& doc) {
    strncpy(_config.deviceName, doc["deviceName"] | "VFD Clock", sizeof(_config.deviceName) - 1);
    _config.deviceName[sizeof(_config.deviceName) - 1] = 0;
    
    strncpy(_config.wifiSsid, doc["wifiSsid"] | WIFI_SSID, sizeof(_config.wifiSsid) - 1);
    _config.wifiSsid[sizeof(_config.wifiSsid) - 1] = 0;
    
    strncpy(_config.wifiPassword, doc["wifiPassword"] | WIFI_PASSWORD, sizeof(_config.wifiPassword) - 1);
    _config.wifiPassword[sizeof(_config.wifiPassword) - 1] = 0;
    
    strncpy(_config.ntpServer, doc["ntpServer"] | NTP_SERVER, sizeof(_config.ntpServer) - 1);
    _config.ntpServer[sizeof(_config.ntpServer) - 1] = 0;
    
    _config.timezoneOffset = doc["timezoneOffset"] | UTC_OFFSET_SECONDS;
    _config.brightness = doc["brightness"] | VFD_DEFAULT_BRIGHTNESS;
    _config.showSeconds = doc["showSeconds"] | true;
    _config.showActivityIndicators = doc["showActivityIndicators"] | true;
    
    strncpy(_config.weatherApiKey, doc["weatherApiKey"] | WEATHER_API_KEY, sizeof(_config.weatherApiKey) - 1);
    _config.weatherApiKey[sizeof(_config.weatherApiKey) - 1] = 0;
    
    _config.weatherLat = doc["weatherLat"] | WEATHER_LAT;
    _config.weatherLon = doc["weatherLon"] | WEATHER_LON;
    
    strncpy(_config.weatherUnits, doc["weatherUnits"] | WEATHER_UNITS, sizeof(_config.weatherUnits) - 1);
    _config.weatherUnits[sizeof(_config.weatherUnits) - 1] = 0;
    
    _config.weatherUpdateInterval = doc["weatherUpdateInterval"] | WEATHER_UPDATE_INTERVAL;
    _config.weatherDisplayStartMin = doc["weatherDisplayStartMin"] | 8;
    _config.weatherDisplayStartMax = doc["weatherDisplayStartMax"] | 28;
    _config.weatherDurationMin = doc["weatherDurationMin"] | 15;
    _config.weatherDurationMax = doc["weatherDurationMax"] | 25;
    
    // Clock source
    _config.clockSource = doc["clockSource"] | 0;
    
    // Tilt sensor
    _config.tiltSensorPin = doc["tiltSensorPin"] | 0;
    _config.autoRotate = doc["autoRotate"] | false;
}

void ConfigManager::serializeConfig(JsonDocument& doc) const {
    doc["deviceName"] = _config.deviceName;
    doc["wifiSsid"] = _config.wifiSsid;
    doc["wifiPassword"] = _config.wifiPassword;
    doc["ntpServer"] = _config.ntpServer;
    doc["timezoneOffset"] = _config.timezoneOffset;
    doc["brightness"] = _config.brightness;
    doc["showSeconds"] = _config.showSeconds;
    doc["showActivityIndicators"] = _config.showActivityIndicators;
    doc["weatherApiKey"] = _config.weatherApiKey;
    doc["weatherLat"] = _config.weatherLat;
    doc["weatherLon"] = _config.weatherLon;
    doc["weatherUnits"] = _config.weatherUnits;
    doc["weatherUpdateInterval"] = _config.weatherUpdateInterval;
    doc["weatherDisplayStartMin"] = _config.weatherDisplayStartMin;
    doc["weatherDisplayStartMax"] = _config.weatherDisplayStartMax;
    doc["weatherDurationMin"] = _config.weatherDurationMin;
    doc["weatherDurationMax"] = _config.weatherDurationMax;
    
    // Clock source
    doc["clockSource"] = _config.clockSource;
    
    // Tilt sensor
    doc["tiltSensorPin"] = _config.tiltSensorPin;
    doc["autoRotate"] = _config.autoRotate;
}
