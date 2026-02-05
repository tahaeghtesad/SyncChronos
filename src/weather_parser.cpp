#include "weather_parser.h"
#include <ArduinoJson.h>

WeatherData WeatherParser::parse(const String& json) {
    WeatherData data = {0.0f, 0, "---", false};
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, json.c_str());
    
    if (error) {
        return data; 
    }
    
    // Check for "weather" array and "main" object
    if (!doc["main"]["temp"].is<float>() || !doc["weather"][0]["id"].is<int>()) {
        return data;
    }
    
    data.temp = doc["main"]["temp"].as<float>();
    data.conditionCode = doc["weather"][0]["id"].as<int>();
    
    updateConditionShort(data.conditionCode, data.conditionShort);
    data.valid = true;
    
    return data;
}

void WeatherParser::updateConditionShort(int code, char* buffer) {
    // OpenWeatherMap condition codes
    if (code >= 200 && code < 300) {
        strcpy(buffer, "THN");  // Thunder
    } else if (code >= 300 && code < 400) {
        strcpy(buffer, "DRZ");  // Drizzle
    } else if (code >= 500 && code < 600) {
        strcpy(buffer, "RAN");  // Rain
    } else if (code >= 600 && code < 700) {
        strcpy(buffer, "SNO");  // Snow
    } else if (code >= 700 && code < 800) {
        strcpy(buffer, "FOG");  // Fog/mist/haze
    } else if (code == 800) {
        strcpy(buffer, "SUN");  // Clear sky
    } else if (code > 800 && code < 900) {
        strcpy(buffer, "CLD");  // Clouds
    } else {
        strcpy(buffer, "???");  // Unknown
    }
}
