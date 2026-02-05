/**
 * Weather Manager Implementation
 * 
 * Fetches weather data from OpenWeatherMap API
 */

#include "weather_manager.h"
#include "config.h"
#include "config_manager.h"

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <ArduinoJson.h>

WeatherManager::WeatherManager()
    : _temperature(0.0f),
      _conditionCode(0),
      _lastUpdate(0),
      _valid(false) {
    strcpy(_conditionShort, "---");
}

void WeatherManager::begin() {
    Serial.println("Weather Manager initialized");
    // Fetch initial weather data
    fetch();
}

void WeatherManager::update() {
    // Check if it's time to update
    if (millis() - _lastUpdate >= configManager.getWeatherUpdateInterval()) {
        fetch();
    }
}

bool WeatherManager::fetch() {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("Weather: WiFi not connected");
        return false;
    }
    
    Serial.println("Fetching weather data...");
    
    // Build API URL
    String url = "http://api.openweathermap.org/data/2.5/weather";
    url += "?lat=" + String(configManager.getWeatherLat(), 4);
    url += "&lon=" + String(configManager.getWeatherLon(), 4);
    url += "&units=" + String(configManager.getWeatherUnits());
    url += "&appid=" + String(configManager.getWeatherApiKey());
    
    WiFiClient client;
    HTTPClient http;
    
    http.begin(client, url);
    http.setTimeout(10000);  // 10 second timeout
    
    int httpCode = http.GET();
    
    if (httpCode != HTTP_CODE_OK) {
        Serial.printf("Weather: HTTP error %d\n", httpCode);
        http.end();
        return false;
    }
    
    String payload = http.getString();
    http.end();
    
    // Parse JSON response
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, payload);
    
    if (error) {
        Serial.printf("Weather: JSON parse error: %s\n", error.c_str());
        return false;
    }
    
    // Extract weather data
    _temperature = doc["main"]["temp"].as<float>();
    _conditionCode = doc["weather"][0]["id"].as<int>();
    
    updateConditionShort();
    
    _lastUpdate = millis();
    _valid = true;
    
    Serial.printf("Weather: %.1f%s %s (code %d)\n", 
                  _temperature, 
                  strcmp(configManager.getWeatherUnits(), "imperial") == 0 ? "F" : "C",
                  _conditionShort,
                  _conditionCode);
    
    return true;
}

float WeatherManager::getTemperature() const {
    return _temperature;
}

int WeatherManager::getConditionCode() const {
    return _conditionCode;
}

const char* WeatherManager::getConditionShort() const {
    return _conditionShort;
}

bool WeatherManager::isValid() const {
    // Consider data stale after 2x update interval
    if (!_valid) return false;
    if (millis() - _lastUpdate > configManager.getWeatherUpdateInterval() * 2) return false;
    return true;
}

unsigned long WeatherManager::getLastUpdateAge() const {
    return millis() - _lastUpdate;
}

void WeatherManager::updateConditionShort() {
    // OpenWeatherMap condition codes:
    // https://openweathermap.org/weather-conditions
    // 
    // 2xx: Thunderstorm
    // 3xx: Drizzle
    // 5xx: Rain
    // 6xx: Snow
    // 7xx: Atmosphere (fog, mist, etc.)
    // 800: Clear
    // 80x: Clouds
    
    if (_conditionCode >= 200 && _conditionCode < 300) {
        strcpy(_conditionShort, "THN");  // Thunder
    } else if (_conditionCode >= 300 && _conditionCode < 400) {
        strcpy(_conditionShort, "DRZ");  // Drizzle
    } else if (_conditionCode >= 500 && _conditionCode < 600) {
        strcpy(_conditionShort, "RAN");  // Rain
    } else if (_conditionCode >= 600 && _conditionCode < 700) {
        strcpy(_conditionShort, "SNO");  // Snow
    } else if (_conditionCode >= 700 && _conditionCode < 800) {
        strcpy(_conditionShort, "FOG");  // Fog/mist/haze
    } else if (_conditionCode == 800) {
        strcpy(_conditionShort, "SUN");  // Clear sky
    } else if (_conditionCode > 800 && _conditionCode < 900) {
        strcpy(_conditionShort, "CLD");  // Clouds
    } else {
        strcpy(_conditionShort, "???");  // Unknown
    }
}
