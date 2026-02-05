#include <Arduino.h>
#include <unity.h>
#include "time_manager.h"

TimeManager* tm_test;

void setUp(void) {
    tm_test = new TimeManager();
    // Default config has UTC_OFFSET_SECONDS (-28800 for PST)
    // We should ensure timezone is set to 0 for easier testing, or account for it
    tm_test->setTimezoneOffset(0);
}

void tearDown(void) {
    delete tm_test;
}

void test_time_components(void) {
    // 2023-01-01 12:00:00 UTC = 1672574400
    unsigned long epoch = 1672574400;
    tm_test->setTime(epoch);
    
    TEST_ASSERT_TRUE(tm_test->isTimeValid());
    TEST_ASSERT_EQUAL_UINT32(epoch, tm_test->getEpochTime());
    
    TEST_ASSERT_EQUAL_INT(12, tm_test->getHours());
    TEST_ASSERT_EQUAL_INT(0, tm_test->getMinutes());
    TEST_ASSERT_EQUAL_INT(0, tm_test->getSeconds());
    
    // Check 12-hour format
    TEST_ASSERT_EQUAL_INT(12, tm_test->getHours12());
    TEST_ASSERT_TRUE(tm_test->isPM());
}

void test_time_math(void) {
    // 2023-01-01 23:59:59 UTC = 1672617599
    unsigned long epoch = 1672617599;
    tm_test->setTime(epoch);
    
    TEST_ASSERT_EQUAL_INT(23, tm_test->getHours());
    TEST_ASSERT_EQUAL_INT(59, tm_test->getMinutes());
    TEST_ASSERT_EQUAL_INT(59, tm_test->getSeconds());
}

void test_timezone_offset(void) {
    // 12:00 UTC
    unsigned long epoch = 1672574400;
    tm_test->setTime(epoch);
    
    // Set PST (-8 hours = -28800)
    tm_test->setTimezoneOffset(-28800);
    
    // Should be 04:00
    TEST_ASSERT_EQUAL_INT(4, tm_test->getHours());
    
    // Set JST (+9 hours = 32400)
    tm_test->setTimezoneOffset(32400);
    
    // Should be 21:00 (12 + 9)
    TEST_ASSERT_EQUAL_INT(21, tm_test->getHours());
}

void setup() {
    delay(2000);
    UNITY_BEGIN();
    RUN_TEST(test_time_components);
    RUN_TEST(test_time_math);
    RUN_TEST(test_timezone_offset);
    UNITY_END();
}

void loop() {}
