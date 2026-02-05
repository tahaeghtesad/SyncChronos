/**
 * WiFi Manager Header
 *
 * Handles WiFi connection and reconnection
 */

#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include "config.h"
#include <Arduino.h>
#include <ESP8266WiFi.h>

class WiFiManager {
public:
  WiFiManager();

  /**
   * Connect to WiFi network
   * @param ssid Network SSID
   * @param password Network password
   * @param timeout Connection timeout in ms
   * @return true if connected successfully
   */
  bool connect(const char *ssid, const char *password,
               unsigned long timeout = WIFI_CONNECT_TIMEOUT);

  /**
   * Disconnect from WiFi
   */
  void disconnect();

  /**
   * Check if connected to WiFi
   */
  bool isConnected() const;

  /**
   * Get current IP address
   */
  String getIP() const;

  /**
   * Get signal strength (RSSI)
   */
  int getSignalStrength() const;

  /**
   * Maintain connection (call in loop for auto-reconnect)
   */
  void maintain();

private:
  String _ssid;
  String _password;
  bool _autoReconnect;
};

#endif // WIFI_MANAGER_H
