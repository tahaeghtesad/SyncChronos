/**
 * Time Manager Implementation
 *
 * Non-blocking NTP synchronization using raw UDP
 * State machine allows display updates during sync
 */

#include "time_manager.h"
#include "config_manager.h"

// NTP server port
static const int NTP_PORT = 123;

TimeManager::TimeManager()
    : _timeValid(false), _lastSyncTime(0),
      _timezoneOffset(UTC_OFFSET_SECONDS), _syncState(NTP_IDLE),
      _syncStartTime(0), _epochTime(0), _lastMillis(0),
      _lastTimeInfoUpdate(0) {
    memset(&_timeInfo, 0, sizeof(_timeInfo));
    memset(_ntpPacketBuffer, 0, LOCAL_NTP_PACKET_SIZE);
}

void TimeManager::begin() {
    _udp.begin(NTP_PORT);
    Serial.println("TimeManager initialized (non-blocking)");
    Serial.printf("Timezone offset: %ld seconds\n", _timezoneOffset);
    
    // Start initial sync
    startSync();
}

void TimeManager::update() {
    // Process NTP state machine
    if (_syncState != NTP_IDLE) {
        processNtpState();
    }
    
    // Update local time based on millis elapsed
    if (_timeValid) {
        unsigned long now = millis();
        unsigned long elapsed = now - _lastMillis;
        if (elapsed >= 1000) {
            _epochTime += elapsed / 1000;
            _lastMillis = now - (elapsed % 1000);
        }
    }
}

void TimeManager::startSync() {
    if (_syncState != NTP_IDLE) {
        return;  // Already syncing
    }
    
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("NTP: WiFi not connected");
        return;
    }
    
    Serial.println("NTP: Starting non-blocking sync...");
    _syncState = NTP_SENDING;
    _syncStartTime = millis();
}

void TimeManager::setTime(unsigned long epoch) {
    _epochTime = epoch;
    _timeValid = true;
    _lastMillis = millis();
    updateTimeInfo();
}

bool TimeManager::isSyncing() const {
    return _syncState != NTP_IDLE;
}

void TimeManager::processNtpState() {
    // Check for timeout
    if (millis() - _syncStartTime > NTP_TIMEOUT) {
        Serial.println("NTP: Sync timeout");
        _syncState = NTP_IDLE;
        return;
    }
    
    switch (_syncState) {
        case NTP_SENDING:
            if (sendNtpPacket()) {
                _syncState = NTP_WAITING;
            } else {
                Serial.println("NTP: Failed to send packet");
                _syncState = NTP_IDLE;
            }
            break;
            
        case NTP_WAITING:
            if (_udp.parsePacket() >= LOCAL_NTP_PACKET_SIZE) {
                _syncState = NTP_RECEIVED;
            }
            // Stay in waiting state until packet arrives or timeout
            break;
            
        case NTP_RECEIVED:
            if (parseNtpResponse()) {
                _timeValid = true;
                _lastSyncTime = millis();
                _lastMillis = millis();
                Serial.printf("NTP: Synced - %02d:%02d:%02d\n", 
                              getHours(), getMinutes(), getSeconds());
            } else {
                Serial.println("NTP: Failed to parse response");
            }
            _syncState = NTP_IDLE;
            break;
            
        case NTP_ERROR:
        case NTP_IDLE:
        default:
            _syncState = NTP_IDLE;
            break;
    }
}

bool TimeManager::sendNtpPacket() {
    // Initialize NTP packet
    memset(_ntpPacketBuffer, 0, LOCAL_NTP_PACKET_SIZE);
    
    // Set NTP header values
    _ntpPacketBuffer[0] = 0b11100011;  // LI=3, Version=4, Mode=3 (client)
    _ntpPacketBuffer[1] = 0;           // Stratum
    _ntpPacketBuffer[2] = 6;           // Polling interval
    _ntpPacketBuffer[3] = 0xEC;        // Precision
    // 8 bytes of zero (Root Delay, Root Dispersion)
    _ntpPacketBuffer[12] = 49;         // Reference ID "1"
    _ntpPacketBuffer[13] = 0x4E;       // "N"
    _ntpPacketBuffer[14] = 49;         // "1"
    _ntpPacketBuffer[15] = 52;         // "4"
    
    // Get NTP server from config
    const char* server = configManager.getNtpServer();
    
    // Send packet
    if (_udp.beginPacket(server, NTP_PORT) == 0) {
        return false;
    }
    
    _udp.write(_ntpPacketBuffer, LOCAL_NTP_PACKET_SIZE);
    
    return _udp.endPacket() == 1;
}

bool TimeManager::parseNtpResponse() {
    // Read packet into buffer
    _udp.read(_ntpPacketBuffer, LOCAL_NTP_PACKET_SIZE);
    
    // Extract transmit timestamp (bytes 40-43)
    // NTP timestamp is seconds since 1900, we need Unix epoch (since 1970)
    unsigned long highWord = word(_ntpPacketBuffer[40], _ntpPacketBuffer[41]);
    unsigned long lowWord = word(_ntpPacketBuffer[42], _ntpPacketBuffer[43]);
    
    // Combine into seconds since 1900
    unsigned long secsSince1900 = (highWord << 16) | lowWord;
    
    // Unix epoch starts 1970-01-01, NTP starts 1900-01-01
    // Difference is 70 years = 2208988800 seconds
    const unsigned long seventyYears = 2208988800UL;
    
    if (secsSince1900 < seventyYears) {
        return false;  // Invalid response
    }
    
    // Convert to Unix epoch and apply timezone
    _epochTime = secsSince1900 - seventyYears + _timezoneOffset;
    
    return true;
}

void TimeManager::updateTimeInfo() const {
    unsigned long now = millis();
    if (now - _lastTimeInfoUpdate < 100) {
        return;  // Don't update too frequently
    }
    _lastTimeInfoUpdate = now;
    
    time_t t = _epochTime;
    gmtime_r(&t, &_timeInfo);
}

int TimeManager::getHours() const {
    if (!_timeValid) return 0;
    return (_epochTime % 86400) / 3600;
}

int TimeManager::getHours12() const {
    int h = getHours();
    if (h == 0) return 12;
    if (h > 12) return h - 12;
    return h;
}

bool TimeManager::isPM() const {
    return getHours() >= 12;
}

int TimeManager::getMinutes() const {
    if (!_timeValid) return 0;
    return (_epochTime % 3600) / 60;
}

int TimeManager::getSeconds() const {
    if (!_timeValid) return 0;
    return _epochTime % 60;
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

bool TimeManager::isTimeValid() const {
    return _timeValid;
}

unsigned long TimeManager::getEpochTime() const {
    return _epochTime;
}

void TimeManager::setTimezoneOffset(long offset) {
    // Adjust epoch time for the offset change
    if (_timeValid) {
        _epochTime = _epochTime - _timezoneOffset + offset;
    }
    _timezoneOffset = offset;
}

// Legacy blocking sync
bool TimeManager::sync() {
    startSync();
    
    unsigned long start = millis();
    while (_syncState != NTP_IDLE && millis() - start < NTP_TIMEOUT) {
        processNtpState();
        yield();
    }
    
    return _timeValid && (millis() - _lastSyncTime < 5000);
}
