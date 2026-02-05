/**
 * Time Manager Implementation
 *
 * NTP-based time synchronization and tracking
 */

#include "time_manager.h"

TimeManager::TimeManager()
    : _ntpClient(nullptr), _timeValid(false), _lastSyncTime(0),
      _timezoneOffset(UTC_OFFSET_SECONDS), _lastTimeUpdate(0) {
  memset(&_timeInfo, 0, sizeof(_timeInfo));
}

void TimeManager::begin() {
  _ntpClient =
      new NTPClient(_udp, NTP_SERVER, _timezoneOffset, NTP_UPDATE_INTERVAL);
  _ntpClient->begin();

  Serial.println("TimeManager initialized");
  Serial.printf("NTP Server: %s\n", NTP_SERVER);
  Serial.printf("Timezone offset: %ld seconds\n", _timezoneOffset);
}

bool TimeManager::sync() {
  if (!_ntpClient) {
    Serial.println("TimeManager not initialized!");
    return false;
  }

  Serial.println("Syncing time with NTP server...");

  // Force update from NTP
  bool success = _ntpClient->forceUpdate();

  if (success) {
    _timeValid = true;
    _lastSyncTime = millis();

    Serial.printf("Time synced: %02d:%02d:%02d\n", getHours(), getMinutes(),
                  getSeconds());
    Serial.printf("Epoch time: %lu\n", getEpochTime());
  } else {
    Serial.println("NTP sync failed!");
  }

  return success;
}

void TimeManager::update() {
  if (_ntpClient) {
    _ntpClient->update();
  }

  // Re-sync periodically
  if (_timeValid && (millis() - _lastSyncTime > NTP_UPDATE_INTERVAL)) {
    sync();
  }
}

void TimeManager::updateTimeInfo() const {
  // Only update once per second
  unsigned long now = millis();
  if (now - _lastTimeUpdate < 100) {
    return;
  }
  _lastTimeUpdate = now;

  if (_ntpClient) {
    time_t epochTime = _ntpClient->getEpochTime();
    gmtime_r(&epochTime, &_timeInfo);
  }
}

int TimeManager::getHours() const {
  if (_ntpClient) {
    return _ntpClient->getHours();
  }
  return 0;
}

int TimeManager::getHours12() const {
  int h = getHours();
  if (h == 0)
    return 12;
  if (h > 12)
    return h - 12;
  return h;
}

bool TimeManager::isPM() const { return getHours() >= 12; }

int TimeManager::getMinutes() const {
  if (_ntpClient) {
    return _ntpClient->getMinutes();
  }
  return 0;
}

int TimeManager::getSeconds() const {
  if (_ntpClient) {
    return _ntpClient->getSeconds();
  }
  return 0;
}

int TimeManager::getYear() const {
  updateTimeInfo();
  return _timeInfo.tm_year + 1900;
}

int TimeManager::getMonth() const {
  updateTimeInfo();
  return _timeInfo.tm_mon + 1;
}

int TimeManager::getDay() const {
  updateTimeInfo();
  return _timeInfo.tm_mday;
}

int TimeManager::getDayOfWeek() const {
  updateTimeInfo();
  return _timeInfo.tm_wday;
}

bool TimeManager::isTimeValid() const { return _timeValid; }

unsigned long TimeManager::getEpochTime() const {
  if (_ntpClient) {
    return _ntpClient->getEpochTime();
  }
  return 0;
}

void TimeManager::setTimezoneOffset(long offset) {
  _timezoneOffset = offset;
  if (_ntpClient) {
    _ntpClient->setTimeOffset(offset);
  }
}
