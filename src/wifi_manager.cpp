/**
 * WiFi Manager Implementation
 *
 * ESP8266 WiFi connection management
 */

#include "wifi_manager.h"

WiFiManager::WiFiManager() : _autoReconnect(true) {}

bool WiFiManager::connect(const char *ssid, const char *password,
                          unsigned long timeout) {
  _ssid = ssid;
  _password = password;

  Serial.printf("Connecting to WiFi: %s\n", ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  unsigned long startTime = millis();

  while (WiFi.status() != WL_CONNECTED) {
    if (millis() - startTime > timeout) {
      Serial.println("\nWiFi connection timeout!");
      return false;
    }
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi connected!");
  Serial.printf("IP address: %s\n", WiFi.localIP().toString().c_str());
  Serial.printf("Signal strength: %d dBm\n", WiFi.RSSI());

  return true;
}

void WiFiManager::disconnect() {
  WiFi.disconnect();
  Serial.println("WiFi disconnected");
}

bool WiFiManager::isConnected() const { return WiFi.status() == WL_CONNECTED; }

String WiFiManager::getIP() const { return WiFi.localIP().toString(); }

int WiFiManager::getSignalStrength() const { return WiFi.RSSI(); }

void WiFiManager::maintain() {
  if (_autoReconnect && !isConnected()) {
    Serial.println("WiFi connection lost, reconnecting...");
    connect(_ssid.c_str(), _password.c_str());
  }
}
