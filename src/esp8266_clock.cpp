/**
 * ESP8266 Software Clock Implementation
 * 
 * Tracks time using millis() counter
 * Extracted from original TimeManager implementation
 */

#include "esp8266_clock.h"

ESP8266Clock::ESP8266Clock()
    : _epochTime(0), _lastMillis(0), _valid(false) {
}

void ESP8266Clock::begin() {
    _lastMillis = millis();
    Serial.println("ESP8266Clock: Software clock initialized");
}

void ESP8266Clock::update() {
    if (!_valid) return;
    
    unsigned long now = millis();
    unsigned long elapsed = now - _lastMillis;
    
    if (elapsed >= 1000) {
        _epochTime += elapsed / 1000;
        _lastMillis = now - (elapsed % 1000);
    }
}

unsigned long ESP8266Clock::getEpochTime() const {
    return _epochTime;
}

void ESP8266Clock::setEpochTime(unsigned long epoch) {
    _epochTime = epoch;
    _lastMillis = millis();
    _valid = true;
}

bool ESP8266Clock::isValid() const {
    return _valid;
}
