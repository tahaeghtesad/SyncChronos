#include <unity.h>
#include "weather_parser.h"

// No Arduino implementation needed as Native/ArduinoFake handles it?
// Actually ArduinoFake provides mocks. We might need to mock deserializeJson if we weren't linking ArduinoJson.
// But we link ArduinoJson which works natively. We just need String class which ArduinoFake provides.

void setUp(void) {
}

void tearDown(void) {
}

void test_native_parser_success(void) {
    String json = "{\"coord\":{\"lon\":-122.08,\"lat\":37.39},\"weather\":[{\"id\":800,\"main\":\"Clear\"}],\"main\":{\"temp\":20.5,\"humidity\":100}}";
    
    WeatherData data = WeatherParser::parse(json);
    
    TEST_ASSERT_TRUE(data.valid);
    TEST_ASSERT_EQUAL_FLOAT(20.5f, data.temp);
    TEST_ASSERT_EQUAL_INT(800, data.conditionCode);
    TEST_ASSERT_EQUAL_STRING("SUN", data.conditionShort);
}

void test_native_parser_rain(void) {
    String json = "{\"weather\":[{\"id\":500,\"main\":\"Rain\"}],\"main\":{\"temp\":15.0}}";
    
    WeatherData data = WeatherParser::parse(json);
    
    TEST_ASSERT_TRUE(data.valid);
    TEST_ASSERT_EQUAL_INT(500, data.conditionCode);
    TEST_ASSERT_EQUAL_STRING("RAN", data.conditionShort);
}

void test_native_parser_invalid(void) {
    String json = "{\"invalid\":true}";
    
    WeatherData data = WeatherParser::parse(json);
    
    TEST_ASSERT_FALSE(data.valid);
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_native_parser_success);
    RUN_TEST(test_native_parser_rain);
    RUN_TEST(test_native_parser_invalid);
    UNITY_END();
    return 0;
}
