/**
 * Weather Manager Header
 * 
 * Fetches current weather data from OpenWeatherMap API
 */

#ifndef WEATHER_MANAGER_H
#define WEATHER_MANAGER_H

#include <Arduino.h>

class WeatherManager {
public:
    WeatherManager();
    
    /**
     * Initialize the weather manager
     */
    void begin();
    
    /**
     * Update weather data if interval has elapsed
     * Call this regularly from loop()
     */
    void update();
    
    /**
     * Force an immediate weather update
     * @return true if successful
     */
    bool fetch();
    
    /**
     * Get current temperature
     * @return Temperature in configured units (F or C)
     */
    float getTemperature() const;
    
    /**
     * Get weather condition code
     * @return OpenWeatherMap condition code (e.g., 800 = clear)
     */
    int getConditionCode() const;
    
    /**
     * Get short weather description
     * @return 3-char code like "SUN", "CLD", "RAN", "SNO"
     */
    const char* getConditionShort() const;
    
    /**
     * Check if weather data is valid and recent
     * @return true if data is available and not stale
     */
    bool isValid() const;
    
    /**
     * Get time since last successful update
     * @return Milliseconds since last update
     */
    unsigned long getLastUpdateAge() const;

private:
    float _temperature;
    int _conditionCode;
    char _conditionShort[4];
    unsigned long _lastUpdate;
    bool _valid;
    
    /**
     * Convert condition code to short description
     */
    void updateConditionShort();
};

#endif // WEATHER_MANAGER_H
