#include <Arduino.h>
#include <unity.h>
#include "weather_manager.h"
#include "config_manager.h"

WeatherManager* wm;

void setUp(void) {
    wm = new WeatherManager();
    // Default config is metric
    configManager.setWeatherUnits("metric");
}

void tearDown(void) {
    delete wm;
}

void test_weather_parsing_success(void) {
    // Sample JSON response
    String json = "{\"coord\":{\"lon\":-122.08,\"lat\":37.39},\"weather\":[{\"id\":800,\"main\":\"Clear\",\"description\":\"clear sky\",\"icon\":\"01d\"}],\"base\":\"stations\",\"main\":{\"temp\":282.55,\"feels_like\":281.86,\"temp_min\":280.37,\"temp_max\":284.26,\"pressure\":1023,\"humidity\":100},\"visibility\":10000,\"wind\":{\"speed\":1.5,\"deg\":350},\"clouds\":{\"all\":1},\"dt\":1560350645,\"sys\":{\"type\":1,\"id\":5122,\"message\":0.0139,\"country\":\"US\",\"sunrise\":1560343627,\"sunset\":1560396563},\"timezone\":-25200,\"id\":420006353,\"name\":\"Mountain View\",\"cod\":200}";
    
    // Set metric units (though raw temp in JSON above is Kelvin, our parser assumes units are already handled by API call params
    // Wait, parser takes raw float value. If API returns 282.55, parser stores 282.55.
    // Our buildRequest adds &units=metric so API returns Celsius.
    // So let's simulate a Celsius response: temp: 20.5
    
    json = "{\"weather\":[{\"id\":800,\"main\":\"Clear\"}],\"main\":{\"temp\":20.5}}";
    
    TEST_ASSERT_TRUE(wm->parseWeatherJson(json));
    TEST_ASSERT_EQUAL_FLOAT(20.5f, wm->getTemperature());
    TEST_ASSERT_EQUAL_INT(800, wm->getConditionCode());
    TEST_ASSERT_EQUAL_STRING("SUN", wm->getConditionShort());
}

void test_weather_parsing_rain(void) {
    String json = "{\"weather\":[{\"id\":500,\"main\":\"Rain\"}],\"main\":{\"temp\":15.0}}";
    
    TEST_ASSERT_TRUE(wm->parseWeatherJson(json));
    TEST_ASSERT_EQUAL_INT(500, wm->getConditionCode());
    TEST_ASSERT_EQUAL_STRING("RAN", wm->getConditionShort());
}

void test_weather_parsing_invalid(void) {
    String json = "{\"invalid\":true}";
    
    TEST_ASSERT_FALSE(wm->parseWeatherJson(json));
}

void test_weather_broken_json(void) {
    String json = "{broken";
    TEST_ASSERT_FALSE(wm->parseWeatherJson(json));
}

void setup() {
    delay(2000);
    UNITY_BEGIN();
    RUN_TEST(test_weather_parsing_success);
    RUN_TEST(test_weather_parsing_rain);
    RUN_TEST(test_weather_parsing_invalid);
    RUN_TEST(test_weather_broken_json);
    UNITY_END();
}

void loop() {}
