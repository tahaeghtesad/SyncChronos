/**
 * DS3231 RTC Clock Implementation
 * 
 * Hardware RTC with battery backup for time persistence
 */

#include "ds3231_clock.h"
#include <Wire.h>

DS3231Clock::DS3231Clock()
    : _present(false), _valid(false), _cachedEpoch(0), _lastReadMillis(0) {
}

void DS3231Clock::begin() {
    Wire.begin();
    
    if (_rtc.begin()) {
        _present = true;
        _valid = !_rtc.lostPower();  // Valid if battery maintained time
        
        Serial.println("DS3231Clock: RTC found at 0x68");
        
        if (_rtc.lostPower()) {
            Serial.println("DS3231Clock: RTC lost power, needs sync");
        } else {
            // Read initial time
            DateTime now = _rtc.now();
            _cachedEpoch = now.unixtime();
            Serial.printf("DS3231Clock: RTC time is %02d:%02d:%02d\n",
                          now.hour(), now.minute(), now.second());
        }
    } else {
        _present = false;
        Serial.println("DS3231Clock: RTC not found!");
    }
}

void DS3231Clock::update() {
    if (!_present) return;
    
    // Periodically read from RTC to stay synchronized
    unsigned long now = millis();
    if (now - _lastReadMillis >= READ_INTERVAL) {
        _lastReadMillis = now;
        DateTime dt = _rtc.now();
        _cachedEpoch = dt.unixtime();
    }
}

unsigned long DS3231Clock::getEpochTime() const {
    return _cachedEpoch;
}

void DS3231Clock::setEpochTime(unsigned long epoch) {
    if (!_present) return;
    
    // Write to RTC hardware
    _rtc.adjust(DateTime(epoch));
    _cachedEpoch = epoch;
    _valid = true;
    
    DateTime dt = DateTime(epoch);
    Serial.printf("DS3231Clock: RTC set to %02d:%02d:%02d\n",
                  dt.hour(), dt.minute(), dt.second());
}

bool DS3231Clock::isValid() const {
    return _present && _valid;
}
