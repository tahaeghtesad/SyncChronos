/**
 * Web Portal Header
 * 
 * HTTP server for configuration interface
 */

#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include <Arduino.h>
#include <ESP8266WebServer.h>

class WebPortal {
public:
    WebPortal();
    
    /**
     * Start web server on port 80
     */
    void begin();
    
    /**
     * Handle incoming requests - call in loop()
     */
    void handleClient();
    
    /**
     * Get server port
     */
    uint16_t getPort() const { return 80; }

private:
    ESP8266WebServer _server;
    
    // Request handlers
    void handleRoot();
    void handleGetConfig();
    void handlePostConfig();
    void handleRestart();
    void handleNotFound();
    
    // HTML page generator
    String generateHtml();
};

// Global instance
extern WebPortal webPortal;

#endif // WEB_SERVER_H
