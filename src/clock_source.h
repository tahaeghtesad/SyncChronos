/**
 * Clock Source Interface
 * 
 * Abstract base class for time sources (ESP8266 internal, DS3231 RTC, etc.)
 * Allows runtime switching between hardware and software clocks
 */

#ifndef CLOCK_SOURCE_H
#define CLOCK_SOURCE_H

#include <Arduino.h>

/**
 * Abstract clock source interface
 * All clock implementations must inherit from this class
 */
class ClockSource {
public:
    virtual ~ClockSource() {}
    
    /**
     * Initialize the clock source
     */
    virtual void begin() = 0;
    
    /**
     * Update internal state (call from loop)
     * For software clocks, this updates time based on millis()
     * For hardware clocks, this may be a no-op
     */
    virtual void update() = 0;
    
    /**
     * Get current epoch time (seconds since 1970-01-01 00:00:00 UTC)
     * @return Unix timestamp with timezone applied
     */
    virtual unsigned long getEpochTime() const = 0;
    
    /**
     * Set epoch time (for NTP sync or manual adjustment)
     * @param epoch Unix timestamp
     */
    virtual void setEpochTime(unsigned long epoch) = 0;
    
    /**
     * Check if clock has been set/synchronized
     * @return true if time is valid
     */
    virtual bool isValid() const = 0;
    
    /**
     * Get human-readable name of this clock source
     * @return Name string (e.g., "ESP8266", "DS3231")
     */
    virtual const char* getName() const = 0;
};

#endif // CLOCK_SOURCE_H
