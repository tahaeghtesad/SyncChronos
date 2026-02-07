/**
 * Tilt Sensor Implementation
 * 
 * Reads digital tilt switch with debouncing
 */

#include "tilt_sensor.h"

TiltSensor::TiltSensor()
    : _pin(0), _enabled(false), _invertLogic(false),
      _flipped(false), _changed(false),
      _lastRawState(false), _lastDebounceTime(0) {
}

void TiltSensor::begin(uint8_t pin, bool invertLogic) {
    if (pin == 0) {
        _enabled = false;
        return;
    }
    
    _pin = pin;
    _invertLogic = invertLogic;
    _enabled = true;
    
    pinMode(_pin, INPUT_PULLUP);
    
    // Read initial state
    bool rawState = digitalRead(_pin);
    _flipped = _invertLogic ? rawState : !rawState;
    _lastRawState = rawState;
    _lastDebounceTime = millis();
    
    Serial.printf("TiltSensor: Initialized on GPIO%d (inverted=%d)\n", 
                  _pin, _invertLogic);
}

void TiltSensor::update() {
    if (!_enabled) return;
    
    bool rawState = digitalRead(_pin);
    
    // Check if state changed
    if (rawState != _lastRawState) {
        _lastDebounceTime = millis();
        _lastRawState = rawState;
    }
    
    // Apply change after debounce period
    if ((millis() - _lastDebounceTime) > DEBOUNCE_DELAY) {
        bool newFlipped = _invertLogic ? rawState : !rawState;
        
        if (newFlipped != _flipped) {
            _flipped = newFlipped;
            _changed = true;
            Serial.printf("TiltSensor: Orientation changed to %s\n",
                          _flipped ? "FLIPPED" : "NORMAL");
        }
    }
}

bool TiltSensor::hasChanged() {
    bool result = _changed;
    _changed = false;
    return result;
}

void TiltSensor::disable() {
    _enabled = false;
    _flipped = false;
}
