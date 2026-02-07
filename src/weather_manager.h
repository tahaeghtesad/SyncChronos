/**
 * Weather Manager Header
 * 
 * Fetches current weather data from OpenWeatherMap API
 * Uses non-blocking HTTP requests via state machine
 */

#ifndef WEATHER_MANAGER_H
#define WEATHER_MANAGER_H

#include <Arduino.h>
#include <WiFiClient.h>
#include "weather_parser.h"

// Fetch states for non-blocking operation
enum WeatherFetchState {
    FETCH_IDLE,
    FETCH_CONNECTING,
    FETCH_SENDING,
    FETCH_WAITING_RESPONSE,
    FETCH_READING_HEADERS,
    FETCH_READING_BODY,
    FETCH_COMPLETE,
    FETCH_ERROR
};

class WeatherManager {
public:
    WeatherManager();
    
    /**
     * Initialize the weather manager
     */
    void begin();
    
    /**
     * Process non-blocking weather operations
     * Call this regularly from loop() - handles state machine
     */
    void update();
    
    /**
     * Start a non-blocking weather fetch
     * Returns immediately, check isFetching() for status
     */
    void startFetch();
    
    /**
     * Check if a fetch is in progress
     */
    bool isFetching() const;
    
    /**
     * Force a blocking weather update (legacy)
     * @return true if successful
     */
    bool fetch();
    
    /**
     * Get current temperature
     */
    float getTemperature() const;
    
    /**
     * Get weather condition code
     */
    int getConditionCode() const;
    
    /**
     * Get short weather description (3 chars)
     */
    const char* getConditionShort() const;
    
    /**
     * Check if weather data is valid and recent
     */
    bool isValid() const;
    
    /**
     * Get time since last successful update
     */
    unsigned long getLastUpdateAge() const;

    /**
     * Parse JSON response and extract weather data
     * Exposed for unit testing
     */
    bool parseWeatherJson(const String& json);

private:
    // Weather data
    float _temperature;
    int _conditionCode;
    char _conditionShort[4];
    unsigned long _lastUpdate;
    bool _valid;
    
    // Non-blocking fetch state
    WeatherFetchState _fetchState;
    WiFiClient _client;
    String _responseBuffer;
    unsigned long _fetchStartTime;
    static const unsigned long FETCH_TIMEOUT = 10000;  // 10 seconds
    
    /**
     * Process the state machine for non-blocking fetch
     */
    void processFetchState();
    
    /**
     * Build the API request string
     */
    String buildRequest();
};

#endif // WEATHER_MANAGER_H
