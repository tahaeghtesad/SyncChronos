/**
 * Weather Manager Implementation
 * 
 * Non-blocking HTTP requests using WiFiClient state machine
 */

#include "weather_manager.h"
#include "config.h"
#include "config_manager.h"

#include <ESP8266WiFi.h>
#include <ArduinoJson.h>

// OpenWeatherMap API host
static const char* OWM_HOST = "api.openweathermap.org";
static const uint16_t OWM_PORT = 80;

WeatherManager::WeatherManager()
    : _temperature(0.0f),
      _conditionCode(0),
      _lastUpdate(0),
      _valid(false),
      _fetchState(FETCH_IDLE),
      _fetchStartTime(0) {
    strcpy(_conditionShort, "---");
}

void WeatherManager::begin() {
    Serial.println("Weather Manager initialized (non-blocking)");
    // Start initial fetch (non-blocking)
    startFetch();
}

void WeatherManager::update() {
    // Process fetch state machine
    if (_fetchState != FETCH_IDLE) {
        processFetchState();
    }
    
    // Start periodic updates (only if not already fetching)
    if (_fetchState == FETCH_IDLE) {
        if (millis() - _lastUpdate >= configManager.getWeatherUpdateInterval()) {
            startFetch();
        }
    }
}

void WeatherManager::startFetch() {
    if (_fetchState != FETCH_IDLE) {
        return;  // Already fetching
    }
    
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("Weather: WiFi not connected");
        return;
    }
    
    Serial.println("Weather: Starting non-blocking fetch...");
    _fetchState = FETCH_CONNECTING;
    _fetchStartTime = millis();
    _responseBuffer = "";
}

bool WeatherManager::isFetching() const {
    return _fetchState != FETCH_IDLE;
}

void WeatherManager::processFetchState() {
    // Check for timeout
    if (millis() - _fetchStartTime > FETCH_TIMEOUT) {
        Serial.println("Weather: Fetch timeout");
        _client.stop();
        _fetchState = FETCH_IDLE;
        return;
    }
    
    switch (_fetchState) {
        case FETCH_CONNECTING:
            if (!_client.connected()) {
                if (_client.connect(OWM_HOST, OWM_PORT)) {
                    Serial.println("Weather: Connected to server");
                    _fetchState = FETCH_SENDING;
                } else {
                    // Still connecting, or failed
                    // connect() is non-blocking on ESP8266 when using WiFiClient
                }
            }
            break;
            
        case FETCH_SENDING: {
            String request = buildRequest();
            _client.print(request);
            _fetchState = FETCH_WAITING_RESPONSE;
            break;
        }
        
        case FETCH_WAITING_RESPONSE:
            if (_client.available()) {
                _fetchState = FETCH_READING_HEADERS;
            }
            break;
            
        case FETCH_READING_HEADERS:
            // Read headers until blank line
            while (_client.available()) {
                String line = _client.readStringUntil('\n');
                if (line == "\r" || line.length() == 0) {
                    _fetchState = FETCH_READING_BODY;
                    break;
                }
            }
            break;
            
        case FETCH_READING_BODY:
            // Read body in chunks
            while (_client.available()) {
                char c = _client.read();
                _responseBuffer += c;
                
                // Limit buffer size
                if (_responseBuffer.length() > 2048) {
                    Serial.println("Weather: Response too large");
                    _client.stop();
                    _fetchState = FETCH_IDLE;
                    return;
                }
            }
            
            // Check if connection closed (response complete)
            if (!_client.connected()) {
                _fetchState = FETCH_COMPLETE;
            }
            break;
            
        case FETCH_COMPLETE:
            _client.stop();
            if (parseResponse(_responseBuffer)) {
                _lastUpdate = millis();
                _valid = true;
                Serial.printf("Weather: %.1f%s %s (code %d)\n", 
                              _temperature, 
                              strcmp(configManager.getWeatherUnits(), "imperial") == 0 ? "F" : "C",
                              _conditionShort,
                              _conditionCode);
            } else {
                Serial.println("Weather: Parse failed");
            }
            _responseBuffer = "";
            _fetchState = FETCH_IDLE;
            break;
            
        case FETCH_ERROR:
        case FETCH_IDLE:
        default:
            _fetchState = FETCH_IDLE;
            break;
    }
}

String WeatherManager::buildRequest() {
    String path = "/data/2.5/weather";
    path += "?lat=" + String(configManager.getWeatherLat(), 4);
    path += "&lon=" + String(configManager.getWeatherLon(), 4);
    path += "&units=" + String(configManager.getWeatherUnits());
    path += "&appid=" + String(configManager.getWeatherApiKey());
    
    String request = "GET " + path + " HTTP/1.1\r\n";
    request += "Host: " + String(OWM_HOST) + "\r\n";
    request += "Connection: close\r\n";
    request += "\r\n";
    
    return request;
}

bool WeatherManager::parseResponse(const String& json) {
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, json);
    
    if (error) {
        Serial.printf("Weather: JSON error: %s\n", error.c_str());
        return false;
    }
    
    if (!doc["main"]["temp"].is<float>()) {
        Serial.println("Weather: Missing temperature");
        return false;
    }
    
    _temperature = doc["main"]["temp"].as<float>();
    _conditionCode = doc["weather"][0]["id"].as<int>();
    
    updateConditionShort();
    return true;
}

// Legacy blocking fetch (kept for compatibility)
bool WeatherManager::fetch() {
    startFetch();
    
    // Block until complete or timeout
    unsigned long start = millis();
    while (_fetchState != FETCH_IDLE && millis() - start < FETCH_TIMEOUT) {
        processFetchState();
        yield();  // Allow ESP8266 background tasks
    }
    
    return _valid && (millis() - _lastUpdate < 5000);
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
    if (!_valid) return false;
    if (millis() - _lastUpdate > configManager.getWeatherUpdateInterval() * 2) return false;
    return true;
}

unsigned long WeatherManager::getLastUpdateAge() const {
    return millis() - _lastUpdate;
}

void WeatherManager::updateConditionShort() {
    // OpenWeatherMap condition codes
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
