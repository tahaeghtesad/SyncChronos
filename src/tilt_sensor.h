/**
 * Tilt Sensor Header
 * 
 * Simple digital tilt switch reader for display rotation
 * Reads GPIO state to detect orientation
 */

#ifndef TILT_SENSOR_H
#define TILT_SENSOR_H

#include <Arduino.h>

class TiltSensor {
public:
    TiltSensor();
    
    /**
     * Initialize tilt sensor on specified GPIO pin
     * @param pin GPIO pin connected to tilt switch
     * @param invertLogic If true, HIGH = normal, LOW = flipped
     */
    void begin(uint8_t pin, bool invertLogic = false);
    
    /**
     * Update sensor state (call from loop)
     * Includes debounce logic
     */
    void update();
    
    /**
     * Check if display should be flipped
     * @return true if device is upside down
     */
    bool isFlipped() const { return _flipped; }
    
    /**
     * Check if orientation changed since last check
     * Clears the flag after reading
     * @return true if orientation changed
     */
    bool hasChanged();
    
    /**
     * Check if sensor is enabled
     * @return true if pin is configured
     */
    bool isEnabled() const { return _enabled; }
    
    /**
     * Disable the sensor
     */
    void disable();

private:
    uint8_t _pin;
    bool _enabled;
    bool _invertLogic;
    bool _flipped;
    bool _changed;
    
    // Debounce
    bool _lastRawState;
    unsigned long _lastDebounceTime;
    static const unsigned long DEBOUNCE_DELAY = 50;  // 50ms debounce
};

#endif // TILT_SENSOR_H
