#include <Arduino.h>
#include <unity.h>
#include "config_manager.h"
#include "config.h"

void setUp(void) {
    configManager.reset();
}

void tearDown(void) {
}

void test_config_defaults(void) {
    TEST_ASSERT_EQUAL_STRING("VFD Clock", configManager.getDeviceName());
    TEST_ASSERT_EQUAL_STRING(WIFI_SSID, configManager.getWifiSsid());
    TEST_ASSERT_EQUAL_UINT8(VFD_DEFAULT_BRIGHTNESS, configManager.getBrightness());
    TEST_ASSERT_TRUE(configManager.getShowSeconds());
    TEST_ASSERT_TRUE(configManager.getShowActivityIndicators());
}

void test_config_setters_getters(void) {
    configManager.setDeviceName("Test Device");
    TEST_ASSERT_EQUAL_STRING("Test Device", configManager.getDeviceName());
    
    configManager.setBrightness(128);
    TEST_ASSERT_EQUAL_UINT8(128, configManager.getBrightness());
    
    configManager.setWifiCredentials("MySSID", "MyPass");
    TEST_ASSERT_EQUAL_STRING("MySSID", configManager.getWifiSsid());
    TEST_ASSERT_EQUAL_STRING("MyPass", configManager.getWifiPassword());
    
    configManager.setWeatherLocation(37.3688, -122.0363);
    TEST_ASSERT_EQUAL_FLOAT(37.3688, configManager.getWeatherLat());
    TEST_ASSERT_EQUAL_FLOAT(-122.0363, configManager.getWeatherLon());
}

void setup() {
    delay(2000); // Wait for serial
    UNITY_BEGIN();
    RUN_TEST(test_config_defaults);
    RUN_TEST(test_config_setters_getters);
    UNITY_END();
}

void loop() {}
