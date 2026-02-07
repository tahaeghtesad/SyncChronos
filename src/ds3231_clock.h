/**
 * DS3231 RTC Clock
 * 
 * Hardware real-time clock with battery backup
 * Uses I2C interface (SDA/SCL)
 */

#ifndef DS3231_CLOCK_H
#define DS3231_CLOCK_H

#include "clock_source.h"
#include <RTClib.h>

class DS3231Clock : public ClockSource {
public:
    DS3231Clock();
    
    void begin() override;
    void update() override;
    unsigned long getEpochTime() const override;
    void setEpochTime(unsigned long epoch) override;
    bool isValid() const override;
    const char* getName() const override { return "DS3231"; }
    
    /**
     * Check if DS3231 is detected on I2C bus
     * @return true if RTC is present
     */
    bool isPresent() const { return _present; }
    
    /**
     * Get the underlying RTC object for direct access
     * @return Reference to RTC_DS3231
     */
    RTC_DS3231& getRTC() { return _rtc; }

private:
    RTC_DS3231 _rtc;
    bool _present;
    bool _valid;
    unsigned long _cachedEpoch;
    unsigned long _lastReadMillis;
    static const unsigned long READ_INTERVAL = 500;  // Read RTC every 500ms
};

#endif // DS3231_CLOCK_H
