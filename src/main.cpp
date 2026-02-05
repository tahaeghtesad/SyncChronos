/**
 * VFD Clock - Main Application
 * 
 * ESP8266 + Display (VFD or LED Matrix)
 * 
 * Features:
 * - WiFi connectivity for NTP time sync
 * - Configurable timezone
 * - Brightness control
 * - Multiple display modes (time, date, temp, etc.)
 * 
 * Build with -D USE_MAX7219_DISPLAY to use LED matrix instead of VFD
 */

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <NTPClient.h>

#include "config.h"
#include "config_manager.h"
#include "display_driver.h"
#include "time_manager.h"
#include "wifi_manager.h"
#include "weather_manager.h"
#include "web_server.h"

// Conditional display driver selection
#ifdef USE_MAX7219_DISPLAY
    #include "max7219_driver.h"
    MAX7219Driver display;
#else
    #include "vfd_driver.h"
    VFDDriver display;
#endif

// Global objects
TimeManager timeManager;
WiFiManager wifiManager;
WeatherManager weatherManager;

// Display mode
enum DisplayMode {
    MODE_TIME,
    MODE_DATE,
    MODE_SECONDS,
    MODE_WEATHER,
    MODE_CUSTOM
};

DisplayMode currentMode = MODE_TIME;
DisplayMode previousMode = MODE_TIME;  // For returning after weather display
unsigned long lastDisplayUpdate = 0;

// Scheduled display timing
bool showingScheduledWeather = false;
unsigned long scheduledWeatherStart = 0;
const unsigned long WEATHER_DISPLAY_DURATION = 30000;  // 30 seconds

// Sync timing
unsigned long lastNtpSync = 0;
const unsigned long NTP_SYNC_INTERVAL = 900000;  // 15 minutes

// Forward declarations
void displayTime();
void displayDate();
void displayTimeWithSeconds();
void displayWeather();
void handleSerialCommands();
void handleScheduledDisplay();

void setup() {
    Serial.begin(115200);
    Serial.println("\n\n=== VFD Clock Starting ===");
    
    // Initialize configuration manager first
    Serial.println("Loading configuration...");
    configManager.begin();
    
    // Initialize VFD display
    Serial.println("Initializing VFD display...");
    display.begin();
    display.setBrightness(configManager.getBrightness());
    display.clear();
    display.print("INIT...");
    
    // Connect to WiFi using saved credentials
    Serial.println("Connecting to WiFi...");
    display.clear();
    display.print("WiFi...");
    
    if (wifiManager.connect(configManager.getWifiSsid(), configManager.getWifiPassword())) {
        Serial.println("WiFi connected!");
        Serial.print("IP Address: ");
        Serial.println(WiFi.localIP());
        
        display.clear();
        display.print("SYNC...");
        
        // Initialize time from NTP with saved timezone
        timeManager.setTimezoneOffset(configManager.getTimezoneOffset());
        timeManager.begin();
        timeManager.sync();
        lastNtpSync = millis();
        
        // Initialize weather
        weatherManager.begin();
        
        // Start web configuration portal
        webPortal.begin();
        Serial.printf("Web portal: http://%s/\n", WiFi.localIP().toString().c_str());
    } else {
        Serial.println("WiFi failed - running offline");
        display.clear();
        display.print("OFFLINE");
        delay(1000);
    }
    
    Serial.println("Setup complete!");
}

void loop() {
    // Periodic NTP sync every 15 minutes
    if (millis() - lastNtpSync >= NTP_SYNC_INTERVAL) {
        Serial.println("Scheduled NTP sync...");
        timeManager.sync();
        lastNtpSync = millis();
    }
    
    // Update time periodically
    timeManager.update();
    
    // Handle scheduled display (weather every 5 mins)
    handleScheduledDisplay();
    
    // Update weather periodically
    weatherManager.update();
    
    // Handle web portal requests
    webPortal.handleClient();
    
    // Update display at configured interval
    if (millis() - lastDisplayUpdate >= DISPLAY_UPDATE_INTERVAL) {
        lastDisplayUpdate = millis();
        
        switch (currentMode) {
            case MODE_TIME:
                displayTime();
                break;
            case MODE_DATE:
                displayDate();
                break;
            case MODE_SECONDS:
                displayTimeWithSeconds();
                break;
            case MODE_WEATHER:
                displayWeather();
                break;
            case MODE_CUSTOM:
                // Custom mode - implement as needed
                break;
        }
    }
    
    // Handle serial commands for testing
    handleSerialCommands();
    
    // Small delay to prevent WDT reset
    yield();
}

void handleScheduledDisplay() {
    int minutes = timeManager.getMinutes();
    int seconds = timeManager.getSeconds();
    
    // Pre-fetch weather at minute 4 (1 min before display)
    // Also at 9, 14, 19, 24, 29, 34, 39, 44, 49, 54, 59
    if ((minutes % 5 == 4) && seconds == 0) {
        static int lastPrefetchMinute = -1;
        if (minutes != lastPrefetchMinute) {
            Serial.println("Pre-fetching weather for scheduled display...");
            weatherManager.fetch();
            lastPrefetchMinute = minutes;
        }
    }
    
    // Show weather at minutes 0, 5, 10, 15, 20, 25, 30, 35, 40, 45, 50, 55
    if ((minutes % 5 == 0) && seconds == 0 && !showingScheduledWeather) {
        Serial.println("Starting scheduled weather display");
        previousMode = currentMode;
        currentMode = MODE_WEATHER;
        showingScheduledWeather = true;
        scheduledWeatherStart = millis();
    }
    
    // Return to previous mode after 30 seconds
    if (showingScheduledWeather && (millis() - scheduledWeatherStart >= WEATHER_DISPLAY_DURATION)) {
        Serial.println("Ending scheduled weather display");
        currentMode = previousMode;
        showingScheduledWeather = false;
    }
}

void displayTime() {
    char buffer[16];
    int hours = timeManager.getHours();
    int minutes = timeManager.getMinutes();
    int seconds = timeManager.getSeconds();
    
    // Format: "12:34" or "12:34:56" with optional seconds
    bool colonOn = (millis() / 500) % 2;
    
    if (configManager.getShowSeconds()) {
        // Show HH:MM:ss with blinking secondary colon
        snprintf(buffer, sizeof(buffer), "%02d:%02d%c%02d", 
                 hours, minutes, colonOn ? ':' : ' ', seconds);
    } else {
        // Show HH:MM with blinking colon
        snprintf(buffer, sizeof(buffer), "%02d%c%02d", 
                 hours, colonOn ? ':' : ' ', minutes);
    }
    
    display.clear();
    display.print(buffer);
}

void displayTimeWithSeconds() {
    char buffer[16];
    snprintf(buffer, sizeof(buffer), "%02d:%02d:%02d",
             timeManager.getHours(),
             timeManager.getMinutes(),
             timeManager.getSeconds());
    
    display.clear();
    display.print(buffer);
}

void displayDate() {
    char buffer[16];
    snprintf(buffer, sizeof(buffer), "%02d-%02d-%02d",
             timeManager.getMonth(),
             timeManager.getDay(),
             timeManager.getYear() % 100);
    
    display.clear();
    display.print(buffer);
}

void displayWeather() {
    char buffer[16];
    
    if (weatherManager.isValid()) {
        // Format: "72F SUN" or "21C CLD"
        int temp = (int)round(weatherManager.getTemperature());
        const char* unit = strcmp(configManager.getWeatherUnits(), "imperial") == 0 ? "F" : "C";
        snprintf(buffer, sizeof(buffer), "%3d%s %s", 
                 temp, unit, weatherManager.getConditionShort());
    } else {
        snprintf(buffer, sizeof(buffer), "WEATHER?");
    }
    
    display.clear();
    display.print(buffer);
}

void handleSerialCommands() {
    if (Serial.available()) {
        char cmd = Serial.read();
        switch (cmd) {
            case 't': // Time mode
                currentMode = MODE_TIME;
                Serial.println("Mode: Time");
                break;
            case 'd': // Date mode
                currentMode = MODE_DATE;
                Serial.println("Mode: Date");
                break;
            case 's': // Seconds mode
                currentMode = MODE_SECONDS;
                Serial.println("Mode: Seconds");
                break;
            case 'w': // Weather mode
                currentMode = MODE_WEATHER;
                Serial.println("Mode: Weather");
                break;
            case '+': // Brightness up
                display.setBrightness(min(255, display.getBrightness() + 16));
                Serial.printf("Brightness: %d\n", display.getBrightness());
                break;
            case '-': // Brightness down
                display.setBrightness(max(0, display.getBrightness() - 16));
                Serial.printf("Brightness: %d\n", display.getBrightness());
                break;
            case 'r': // Resync time
                Serial.println("Resyncing time...");
                timeManager.sync();
                break;
        }
    }
}
