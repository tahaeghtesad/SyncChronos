#ifndef WEATHER_PARSER_H
#define WEATHER_PARSER_H

#include <Arduino.h>

struct WeatherData {
    float temp;
    int conditionCode;
    char conditionShort[4];
    bool valid;
};

class WeatherParser {
public:
    static WeatherData parse(const String& json);
    static void updateConditionShort(int code, char* buffer);
};

#endif // WEATHER_PARSER_H
