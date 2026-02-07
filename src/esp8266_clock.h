/**
 * ESP8266 Software Clock
 * 
 * Time tracking using millis() - extracted from original TimeManager
 * Maintains time in memory, loses time on power cycle
 */

#ifndef ESP8266_CLOCK_H
#define ESP8266_CLOCK_H

#include "clock_source.h"

class ESP8266Clock : public ClockSource {
public:
    ESP8266Clock();
    
    void begin() override;
    void update() override;
    unsigned long getEpochTime() const override;
    void setEpochTime(unsigned long epoch) override;
    bool isValid() const override;
    const char* getName() const override { return "ESP8266"; }

private:
    unsigned long _epochTime;
    unsigned long _lastMillis;
    bool _valid;
};

#endif // ESP8266_CLOCK_H
