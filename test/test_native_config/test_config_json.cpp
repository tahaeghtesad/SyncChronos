#include <unity.h>
#include <ArduinoJson.h>
#include <ArduinoFake.h>
#include "config_manager.h"
#include "config.h"

using namespace fakeit;

// We need to define the global instance for the test
// But ConfigManager constructor calls setDefaults which calls strlcpy etc.
// ArduinoFake should handle strlcpy if we include Arduino.h (which is mocked).

void setUp(void) {
    configManager.reset();
}

void tearDown(void) {
}

void test_serialize_config(void) {
    printf("test_serialize_config: Start\n");
    configManager.setDeviceName("Native Test Device");
    configManager.setBrightness(100);
    
    JsonDocument doc;
    printf("test_serialize_config: Serializing\n");
    configManager.serializeConfig(doc);
    
    TEST_ASSERT_EQUAL_STRING("Native Test Device", doc["deviceName"]);
    TEST_ASSERT_EQUAL_INT(100, doc["brightness"]);
    TEST_ASSERT_EQUAL_STRING(WIFI_SSID, doc["wifiSsid"]);
    printf("test_serialize_config: Done\n");
}

void test_deserialize_config(void) {
    JsonDocument doc;
    doc["deviceName"] = "Imported Device";
    doc["brightness"] = 50;
    doc["showSeconds"] = false;
    
    configManager.deserializeConfig(doc);
    
    TEST_ASSERT_EQUAL_STRING("Imported Device", configManager.getDeviceName());
    TEST_ASSERT_EQUAL_UINT8(50, configManager.getBrightness());
    TEST_ASSERT_FALSE(configManager.getShowSeconds());
    
    // Check missing fields preserved defaults
    TEST_ASSERT_EQUAL_STRING(WIFI_SSID, configManager.getWifiSsid());
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_serialize_config);
    RUN_TEST(test_deserialize_config);
    UNITY_END();
    return 0;
}
