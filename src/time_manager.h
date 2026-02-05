/**
 * Time Manager Header
 *
 * Handles NTP synchronization and time tracking
 */

#ifndef TIME_MANAGER_H
#define TIME_MANAGER_H

#include "config.h"
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <time.h>

class TimeManager {
public:
  TimeManager();

  /**
   * Initialize the time manager
   */
  void begin();

  /**
   * Synchronize time with NTP server
   * @return true if sync successful
   */
  bool sync();

  /**
   * Update time (call in main loop)
   */
  void update();

  /**
   * Get current hour (0-23)
   */
  int getHours() const;

  /**
   * Get current hour in 12-hour format (1-12)
   */
  int getHours12() const;

  /**
   * Is it PM?
   */
  bool isPM() const;

  /**
   * Get current minute (0-59)
   */
  int getMinutes() const;

  /**
   * Get current second (0-59)
   */
  int getSeconds() const;

  /**
   * Get current year
   */
  int getYear() const;

  /**
   * Get current month (1-12)
   */
  int getMonth() const;

  /**
   * Get current day of month (1-31)
   */
  int getDay() const;

  /**
   * Get day of week (0=Sunday, 6=Saturday)
   */
  int getDayOfWeek() const;

  /**
   * Check if time is valid (has been synced)
   */
  bool isTimeValid() const;

  /**
   * Get epoch time
   */
  unsigned long getEpochTime() const;

  /**
   * Set timezone offset in seconds from UTC
   */
  void setTimezoneOffset(long offset);

private:
  WiFiUDP _udp;
  NTPClient *_ntpClient;
  bool _timeValid;
  unsigned long _lastSyncTime;
  long _timezoneOffset;

  // Cached time components
  mutable struct tm _timeInfo;
  mutable unsigned long _lastTimeUpdate;

  void updateTimeInfo() const;
};

#endif // TIME_MANAGER_H
