/**
 * Time Manager Header
 *
 * Handles NTP synchronization and time tracking
 * Uses non-blocking UDP for NTP requests
 */

#ifndef TIME_MANAGER_H
#define TIME_MANAGER_H

#include "config.h"
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <time.h>

// NTP sync states for non-blocking operation
enum NtpSyncState {
    NTP_IDLE,
    NTP_SENDING,
    NTP_WAITING,
    NTP_RECEIVED,
    NTP_ERROR
};

class TimeManager {
public:
    TimeManager();

    /**
     * Initialize the time manager
     */
    void begin();

    /**
     * Update time and process NTP state machine
     * Call this regularly from loop()
     */
    void update();

    /**
     * Start a non-blocking NTP sync
     * Returns immediately, check isSyncing() for status
     */
    void startSync();

    /**
     * Force a blocking sync (legacy)
     */
    bool sync();

    /**
     * Check if an NTP sync is in progress
     */
    bool isSyncing() const;

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
    bool _timeValid;
    unsigned long _lastSyncTime;
    long _timezoneOffset;
    
    // Non-blocking NTP state
    NtpSyncState _syncState;
    unsigned long _syncStartTime;
    static const unsigned long NTP_TIMEOUT = 5000;  // 5 seconds
    static const int LOCAL_NTP_PACKET_SIZE = 48;
    uint8_t _ntpPacketBuffer[48];
    
    // Local time tracking (seconds since epoch + offset)
    unsigned long _epochTime;
    unsigned long _lastMillis;
    
    // Cached time components
    mutable struct tm _timeInfo;
    mutable unsigned long _lastTimeInfoUpdate;

    /**
     * Process the NTP state machine
     */
    void processNtpState();
    
    /**
     * Send NTP request packet
     */
    bool sendNtpPacket();
    
    /**
     * Parse NTP response and update time
     */
    bool parseNtpResponse();
    
    /**
     * Update cached time info struct
     */
    void updateTimeInfo() const;
};

#endif // TIME_MANAGER_H
