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
#include "clock_source.h"
#include "esp8266_clock.h"
#include "ds3231_clock.h"
#include "tilt_sensor.h"

// Conditional display driver selection
#ifdef USE_MAX7219_DISPLAY
    #include "max7219_driver.h"
    MAX7219Driver display;
#else
    #include "vfd_driver.h"
    VFDDriver display;
#endif

// Clock sources
ESP8266Clock esp8266Clock;
DS3231Clock ds3231Clock;

// Tilt sensor for display rotation
TiltSensor tiltSensor;

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
int lastDisplayedSecond = -1;  // Track for second-accurate updates
int lastDisplayedMinute = -1;  // Track for smart updates
unsigned long lastFastUpdate = 0; // Track for activity indicator blinking

// Scheduled display timing
bool showingScheduledWeather = false;
unsigned long scheduledWeatherStart = 0;
unsigned long currentWeatherDuration = 20000;  // Will be randomized

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
    
    // Initialize clock source based on config
    const ClockConfig& cfg = configManager.getConfig();
    if (cfg.clockSource == 1) {
        Serial.println("Using DS3231 RTC clock source");
        ds3231Clock.begin();
        timeManager.setClockSource(&ds3231Clock);
    } else {
        Serial.println("Using ESP8266 software clock source");
        esp8266Clock.begin();
        timeManager.setClockSource(&esp8266Clock);
    }
    
    // Initialize tilt sensor if configured
    if (cfg.tiltSensorPin > 0 && cfg.autoRotate) {
        Serial.printf("Initializing tilt sensor on GPIO%d\n", cfg.tiltSensorPin);
        tiltSensor.begin(cfg.tiltSensorPin);
    }
    
    // Initialize VFD display
    Serial.println("Initializing display...");
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
    // Periodic NTP sync every 15 minutes (non-blocking)
    if (millis() - lastNtpSync >= NTP_SYNC_INTERVAL) {
        Serial.println("Starting scheduled NTP sync...");
        timeManager.startSync();  // Non-blocking!
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
    
    // Update tilt sensor and handle display rotation
    if (tiltSensor.isEnabled()) {
        tiltSensor.update();
        if (tiltSensor.hasChanged()) {
            display.setRotation(tiltSensor.isFlipped());
        }
    }
    
    // Determine if we need fast updates for activity indicators
    bool showActivity = configManager.getShowActivityIndicators() && 
                        (timeManager.isSyncing() || weatherManager.isFetching());
    bool updateDisplay = false;

    if (showActivity) {
        // Fast update (10Hz) for smooth blinking
        if (millis() - lastFastUpdate >= 100) {
            lastFastUpdate = millis();
            updateDisplay = true;
        }
    } else {
        // Normal update exactly when the second changes
        int currentSecond = timeManager.getSeconds();
        if (currentSecond != lastDisplayedSecond) {
            lastDisplayedSecond = currentSecond;
            updateDisplay = true;
        }
    }

    if (updateDisplay) {
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
                // Only update weather display once per minute (static content)
                if (timeManager.getMinutes() != lastDisplayedMinute) {
                    displayWeather();
                    lastDisplayedMinute = timeManager.getMinutes();
                }
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
    
    // Random trigger second for weather display (configurable range)
    static int nextWeatherTriggerSecond = -1;
    static int lastTriggerMinute = -1;
    
    // Pre-fetch weather at minute 4 (1 min before display)
    // Also at 9, 14, 19, 24, 29, 34, 39, 44, 49, 54, 59
    if ((minutes % 5 == 4) && seconds == 0) {
        static int lastPrefetchMinute = -1;
        if (minutes != lastPrefetchMinute) {
            Serial.println("Starting non-blocking weather prefetch...");
            weatherManager.startFetch();  // Non-blocking!
            lastPrefetchMinute = minutes;
            
            // Generate random trigger second from config range
            int startMin = configManager.getWeatherDisplayStartMin();
            int startMax = configManager.getWeatherDisplayStartMax();
            nextWeatherTriggerSecond = random(startMin, startMax + 1);
            Serial.printf("Next weather display at second %d\n", nextWeatherTriggerSecond);
        }
    }
    
    // Show weather at minutes 0, 5, 10, 15... at random second
    if ((minutes % 5 == 0) && minutes != lastTriggerMinute && !showingScheduledWeather) {
        if (nextWeatherTriggerSecond >= 0 && seconds == nextWeatherTriggerSecond) {
            // Generate random duration from config range
            int durMin = configManager.getWeatherDurationMin();
            int durMax = configManager.getWeatherDurationMax();
            currentWeatherDuration = random(durMin, durMax + 1) * 1000UL;
            
            Serial.printf("Starting weather display (:%02d) for %lu sec\n", 
                          seconds, currentWeatherDuration / 1000);
            previousMode = currentMode;
            currentMode = MODE_WEATHER;
            showingScheduledWeather = true;
            scheduledWeatherStart = millis();
            lastTriggerMinute = minutes;
        }
    }
    
    // Reset trigger tracking when minute changes
    if (minutes % 5 != 0) {
        lastTriggerMinute = -1;
    }
    
    // Return to previous mode after random duration
    if (showingScheduledWeather && (millis() - scheduledWeatherStart >= currentWeatherDuration)) {
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
    // Format: "12:34" or "12:34:56" with optional seconds
    bool showActivity = configManager.getShowActivityIndicators() && 
                        (timeManager.isSyncing() || weatherManager.isFetching());
    
    // Normal blink (1Hz) or Fast blink (5Hz)
    bool colonOn;
    if (showActivity) {
        colonOn = (millis() / 100) % 2;
    } else {
        colonOn = (millis() / 500) % 2;
    }
    
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
    
    // Determine activity state for blink rate
    bool showActivity = configManager.getShowActivityIndicators() && 
                        (timeManager.isSyncing() || weatherManager.isFetching());
    
    // Normal blink (1Hz) or Fast blink (5Hz)
    // Note: displayTimeWithSeconds usually shows all colons fixed, but we can blink them for activity
    // or just blink the last one? Let's blink both colons if active.
    
    bool colonsOn = true;
    if (showActivity) {
        colonsOn = (millis() / 100) % 2;
        
        snprintf(buffer, sizeof(buffer), "%02d%c%02d%c%02d",
                 timeManager.getHours(),
                 colonsOn ? ':' : ' ',
                 timeManager.getMinutes(),
                 colonsOn ? ':' : ' ',
                 timeManager.getSeconds());
    } else {
        // Standard display: fixed colons
        snprintf(buffer, sizeof(buffer), "%02d:%02d:%02d",
                 timeManager.getHours(),
                 timeManager.getMinutes(),
                 timeManager.getSeconds());
    }
    
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
