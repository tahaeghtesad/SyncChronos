/**
 * Web Portal Implementation
 * 
 * HTTP server with configuration UI
 */

#include "web_server.h"
#include "config_manager.h"

#include <ArduinoJson.h>

// Global instance
WebPortal webPortal;

WebPortal::WebPortal() : _server(80) {}

void WebPortal::begin() {
    // Setup routes
    _server.on("/", HTTP_GET, [this]() { handleRoot(); });
    _server.on("/api/config", HTTP_GET, [this]() { handleGetConfig(); });
    _server.on("/api/config", HTTP_POST, [this]() { handlePostConfig(); });
    _server.on("/api/restart", HTTP_POST, [this]() { handleRestart(); });
    _server.onNotFound([this]() { handleNotFound(); });
    
    _server.begin();
    Serial.println("WebPortal: Server started on port 80");
}

void WebPortal::handleClient() {
    _server.handleClient();
}

void WebPortal::handleRoot() {
    _server.send(200, "text/html", generateHtml());
}

void WebPortal::handleGetConfig() {
    JsonDocument doc;
    const ClockConfig& cfg = configManager.getConfig();
    
    doc["deviceName"] = cfg.deviceName;
    doc["wifiSsid"] = cfg.wifiSsid;
    // Don't send password for security
    doc["ntpServer"] = cfg.ntpServer;
    doc["timezoneOffset"] = cfg.timezoneOffset;
    doc["brightness"] = cfg.brightness;
    doc["showSeconds"] = cfg.showSeconds;
    doc["showActivityIndicators"] = cfg.showActivityIndicators;
    doc["weatherApiKey"] = cfg.weatherApiKey;
    doc["weatherLat"] = cfg.weatherLat;
    doc["weatherLon"] = cfg.weatherLon;
    doc["weatherUnits"] = cfg.weatherUnits;
    doc["weatherDisplayStartMin"] = cfg.weatherDisplayStartMin;
    doc["weatherDisplayStartMax"] = cfg.weatherDisplayStartMax;
    doc["weatherDurationMin"] = cfg.weatherDurationMin;
    doc["weatherDurationMax"] = cfg.weatherDurationMax;
    
    String response;
    serializeJson(doc, response);
    
    _server.send(200, "application/json", response);
}

void WebPortal::handlePostConfig() {
    if (!_server.hasArg("plain")) {
        _server.send(400, "application/json", "{\"error\":\"No body\"}");
        return;
    }
    
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, _server.arg("plain"));
    
    if (error) {
        _server.send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
        return;
    }
    
    ClockConfig& cfg = configManager.getConfig();
    
    // Update fields if present (using modern ArduinoJson API)
    if (doc["deviceName"].is<const char*>()) {
        strlcpy(cfg.deviceName, doc["deviceName"].as<const char*>(), sizeof(cfg.deviceName));
    }
    if (doc["wifiSsid"].is<const char*>()) {
        strlcpy(cfg.wifiSsid, doc["wifiSsid"].as<const char*>(), sizeof(cfg.wifiSsid));
    }
    if (doc["wifiPassword"].is<const char*>() && strlen(doc["wifiPassword"]) > 0) {
        strlcpy(cfg.wifiPassword, doc["wifiPassword"].as<const char*>(), sizeof(cfg.wifiPassword));
    }
    if (doc["ntpServer"].is<const char*>()) {
        strlcpy(cfg.ntpServer, doc["ntpServer"].as<const char*>(), sizeof(cfg.ntpServer));
    }
    if (doc["timezoneOffset"].is<long>()) {
        cfg.timezoneOffset = doc["timezoneOffset"].as<long>();
    }
    if (doc["brightness"].is<int>()) {
        cfg.brightness = doc["brightness"].as<uint8_t>();
    }
    if (doc["showSeconds"].is<bool>()) {
        cfg.showSeconds = doc["showSeconds"].as<bool>();
    }
    if (doc["showActivityIndicators"].is<bool>()) {
        cfg.showActivityIndicators = doc["showActivityIndicators"].as<bool>();
    }
    if (doc["weatherApiKey"].is<const char*>()) {
        strlcpy(cfg.weatherApiKey, doc["weatherApiKey"].as<const char*>(), sizeof(cfg.weatherApiKey));
    }
    if (doc["weatherLat"].is<float>()) {
        cfg.weatherLat = doc["weatherLat"].as<float>();
    }
    if (doc["weatherLon"].is<float>()) {
        cfg.weatherLon = doc["weatherLon"].as<float>();
    }
    if (doc["weatherUnits"].is<const char*>()) {
        strlcpy(cfg.weatherUnits, doc["weatherUnits"].as<const char*>(), sizeof(cfg.weatherUnits));
    }
    if (doc["weatherDisplayStartMin"].is<int>()) {
        cfg.weatherDisplayStartMin = doc["weatherDisplayStartMin"].as<uint8_t>();
    }
    if (doc["weatherDisplayStartMax"].is<int>()) {
        cfg.weatherDisplayStartMax = doc["weatherDisplayStartMax"].as<uint8_t>();
    }
    if (doc["weatherDurationMin"].is<int>()) {
        cfg.weatherDurationMin = doc["weatherDurationMin"].as<uint8_t>();
    }
    if (doc["weatherDurationMax"].is<int>()) {
        cfg.weatherDurationMax = doc["weatherDurationMax"].as<uint8_t>();
    }
    
    // Save to flash
    if (configManager.save()) {
        _server.send(200, "application/json", "{\"success\":true}");
    } else {
        _server.send(500, "application/json", "{\"error\":\"Save failed\"}");
    }
}

void WebPortal::handleRestart() {
    _server.send(200, "application/json", "{\"success\":true,\"message\":\"Restarting...\"}");
    delay(500);
    ESP.restart();
}

void WebPortal::handleNotFound() {
    _server.send(404, "text/plain", "Not Found");
}

String WebPortal::generateHtml() {
    const ClockConfig& cfg = configManager.getConfig();
    
    String html = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>VFD Clock Settings</title>
    <style>
        * { box-sizing: border-box; margin: 0; padding: 0; }
        body {
            font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif;
            background: linear-gradient(135deg, #1a1a2e 0%, #16213e 100%);
            color: #e4e4e4;
            min-height: 100vh;
            padding: 20px;
        }
        .container { max-width: 500px; margin: 0 auto; }
        h1 {
            text-align: center;
            margin-bottom: 30px;
            color: #00d9ff;
            text-shadow: 0 0 20px rgba(0,217,255,0.5);
        }
        .card {
            background: rgba(255,255,255,0.05);
            border-radius: 12px;
            padding: 20px;
            margin-bottom: 20px;
            border: 1px solid rgba(255,255,255,0.1);
            backdrop-filter: blur(10px);
        }
        .card h2 {
            font-size: 14px;
            text-transform: uppercase;
            color: #888;
            margin-bottom: 15px;
            letter-spacing: 1px;
        }
        .field { margin-bottom: 15px; }
        .field:last-child { margin-bottom: 0; }
        label {
            display: block;
            font-size: 13px;
            color: #aaa;
            margin-bottom: 5px;
        }
        input, select {
            width: 100%;
            padding: 12px;
            border: 1px solid rgba(255,255,255,0.2);
            border-radius: 8px;
            background: rgba(0,0,0,0.3);
            color: #fff;
            font-size: 16px;
            transition: border-color 0.2s;
        }
        input:focus, select:focus {
            outline: none;
            border-color: #00d9ff;
        }
        input[type="range"] {
            padding: 0;
            height: 8px;
            -webkit-appearance: none;
            background: rgba(255,255,255,0.2);
            border-radius: 4px;
        }
        input[type="range"]::-webkit-slider-thumb {
            -webkit-appearance: none;
            width: 20px;
            height: 20px;
            background: #00d9ff;
            border-radius: 50%;
            cursor: pointer;
        }
        .range-value {
            text-align: center;
            margin-top: 5px;
            color: #00d9ff;
        }
        .row { display: flex; gap: 10px; }
        .row .field { flex: 1; }
        .buttons {
            display: flex;
            gap: 10px;
            margin-top: 20px;
        }
        button {
            flex: 1;
            padding: 14px;
            border: none;
            border-radius: 8px;
            font-size: 16px;
            font-weight: 600;
            cursor: pointer;
            transition: transform 0.1s, box-shadow 0.2s;
        }
        button:active { transform: scale(0.98); }
        .btn-save {
            background: linear-gradient(135deg, #00d9ff, #0099cc);
            color: #000;
        }
        .btn-restart {
            background: rgba(255,255,255,0.1);
            color: #fff;
            border: 1px solid rgba(255,255,255,0.2);
        }
        .status {
            text-align: center;
            padding: 10px;
            border-radius: 8px;
            margin-top: 15px;
            display: none;
        }
        .status.success { display: block; background: rgba(0,200,100,0.2); color: #00c864; }
        .status.error { display: block; background: rgba(255,50,50,0.2); color: #ff5050; }
        .search-row { display: flex; gap: 8px; }
        .search-row input { flex: 1; }
        .btn-search {
            flex: 0 0 48px;
            padding: 12px;
            background: rgba(255,255,255,0.1);
            border: 1px solid rgba(255,255,255,0.2);
            border-radius: 8px;
            color: #fff;
            font-size: 18px;
            cursor: pointer;
        }
        .btn-search:hover { background: rgba(255,255,255,0.15); }
        .search-results {
            margin-top: 8px;
            max-height: 200px;
            overflow-y: auto;
        }
        .search-result {
            padding: 10px 12px;
            background: rgba(0,0,0,0.3);
            border: 1px solid rgba(255,255,255,0.1);
            border-radius: 6px;
            margin-bottom: 4px;
            cursor: pointer;
            font-size: 14px;
            transition: background 0.2s;
        }
        .search-result:hover { background: rgba(0,217,255,0.2); }
        .location-display {
            margin-top: 8px;
            padding: 8px 12px;
            background: rgba(0,217,255,0.1);
            border-radius: 6px;
            font-size: 13px;
            color: #00d9ff;
        }
        .toggle-label {
            display: flex;
            align-items: center;
            cursor: pointer;
            gap: 10px;
        }
        .toggle-label input[type="checkbox"] {
            width: 20px;
            height: 20px;
            accent-color: #00d9ff;
        }
        .toggle-text { color: #fff; font-size: 14px; }
    </style>
</head>
<body>
    <div class="container">
        <h1>⏰ VFD Clock</h1>
        
        <div class="card">
            <h2>Device</h2>
            <div class="field">
                <label>Device Name</label>
                <input type="text" id="deviceName" value=")rawliteral";
    html += cfg.deviceName;
    html += R"rawliteral(">
            </div>
        </div>
        
        <div class="card">
            <h2>WiFi</h2>
            <div class="field">
                <label>SSID</label>
                <input type="text" id="wifiSsid" value=")rawliteral";
    html += cfg.wifiSsid;
    html += R"rawliteral(">
            </div>
            <div class="field">
                <label>Password (leave blank to keep current)</label>
                <input type="password" id="wifiPassword" placeholder="••••••••">
            </div>
        </div>
        
        <div class="card">
            <h2>Time</h2>
            <div class="field">
                <label>NTP Server</label>
                <input type="text" id="ntpServer" value=")rawliteral";
    html += cfg.ntpServer;
    html += R"rawliteral(">
            </div>
            <div class="field">
                <label>UTC Offset (seconds)</label>
                <input type="number" id="timezoneOffset" value=")rawliteral";
    html += String(cfg.timezoneOffset);
    html += R"rawliteral(">
            </div>
        </div>
        
        <div class="card">
            <h2>Display</h2>
            <div class="field">
                <label>Brightness</label>
                <input type="range" id="brightness" min="0" max="255" value=")rawliteral";
    html += String(cfg.brightness);
    html += R"rawliteral(">
                <div class="range-value" id="brightnessValue">)rawliteral";
    html += String(cfg.brightness);
    html += R"rawliteral(</div>
            </div>
            <div class="field">
                <label class="toggle-label">
                    <input type="checkbox" id="showSeconds")rawliteral";
    if (cfg.showSeconds) html += " checked";
    html += R"rawliteral(>
                    <span class="toggle-text">Show seconds on clock (HH:MM:ss)</span>
                </label>
                <div style="margin-top: 10px;"></div>
                <label class="toggle-label">
                    <input type="checkbox" id="showActivityIndicators")rawliteral";
    if (cfg.showActivityIndicators) html += " checked";
    html += R"rawliteral(>
                    <span class="toggle-text">Show network activity (blinking colons)</span>
                </label>
            </div>
        </div>
        
        <div class="card">
            <h2>Weather</h2>
            <div class="field">
                <label>OpenWeatherMap API Key</label>
                <input type="text" id="weatherApiKey" value=")rawliteral";
    html += cfg.weatherApiKey;
    html += R"rawliteral(">
            </div>
            <div class="field">
                <label>Location</label>
                <div class="search-row">
                    <input type="text" id="citySearch" placeholder="Search city (e.g. Sunnyvale, CA)">
                    <button type="button" class="btn-search" onclick="searchCity()">🔍</button>
                </div>
                <div id="searchResults" class="search-results"></div>
                <div class="location-display" id="locationDisplay">)rawliteral";
    html += String(cfg.weatherLat, 4) + ", " + String(cfg.weatherLon, 4);
    html += R"rawliteral(</div>
                <input type="hidden" id="weatherLat" value=")rawliteral";
    html += String(cfg.weatherLat, 4);
    html += R"rawliteral(">
                <input type="hidden" id="weatherLon" value=")rawliteral";
    html += String(cfg.weatherLon, 4);
    html += R"rawliteral(">
            </div>
            <div class="field">
                <label>Units</label>
                <select id="weatherUnits">
                    <option value="imperial")rawliteral";
    if (strcmp(cfg.weatherUnits, "imperial") == 0) html += " selected";
    html += R"rawliteral(>Fahrenheit (°F)</option>
                    <option value="metric")rawliteral";
    if (strcmp(cfg.weatherUnits, "metric") == 0) html += " selected";
    html += R"rawliteral(>Celsius (°C)</option>
                </select>
            </div>
            <div class="field">
                <label>Display Timing (seconds)</label>
                <div style="display:grid; grid-template-columns:1fr 1fr; gap:10px;">
                    <div>
                        <small>Start range min</small>
                        <input type="number" id="weatherDisplayStartMin" min="0" max="59" value=")rawliteral";
    html += String(cfg.weatherDisplayStartMin);
    html += R"rawliteral(">
                    </div>
                    <div>
                        <small>Start range max</small>
                        <input type="number" id="weatherDisplayStartMax" min="0" max="59" value=")rawliteral";
    html += String(cfg.weatherDisplayStartMax);
    html += R"rawliteral(">
                    </div>
                    <div>
                        <small>Duration min</small>
                        <input type="number" id="weatherDurationMin" min="5" max="60" value=")rawliteral";
    html += String(cfg.weatherDurationMin);
    html += R"rawliteral(">
                    </div>
                    <div>
                        <small>Duration max</small>
                        <input type="number" id="weatherDurationMax" min="5" max="60" value=")rawliteral";
    html += String(cfg.weatherDurationMax);
    html += R"rawliteral(">
                    </div>
                </div>
            </div>
        </div>
        
        <div class="buttons">
            <button class="btn-save" onclick="saveConfig()">Save</button>
            <button class="btn-restart" onclick="restart()">Restart</button>
        </div>
        
        <div class="status" id="status"></div>
    </div>
    
    <script>
        document.getElementById('brightness').addEventListener('input', function() {
            document.getElementById('brightnessValue').textContent = this.value;
        });
        
        function showStatus(msg, isError) {
            const el = document.getElementById('status');
            el.textContent = msg;
            el.className = 'status ' + (isError ? 'error' : 'success');
            setTimeout(() => el.className = 'status', 3000);
        }
        
        async function saveConfig() {
            const cfg = {
                deviceName: document.getElementById('deviceName').value,
                wifiSsid: document.getElementById('wifiSsid').value,
                wifiPassword: document.getElementById('wifiPassword').value,
                ntpServer: document.getElementById('ntpServer').value,
                timezoneOffset: parseInt(document.getElementById('timezoneOffset').value),
                brightness: parseInt(document.getElementById('brightness').value),
                showSeconds: document.getElementById('showSeconds').checked,
                showActivityIndicators: document.getElementById('showActivityIndicators').checked,
                weatherApiKey: document.getElementById('weatherApiKey').value,
                weatherLat: parseFloat(document.getElementById('weatherLat').value),
                weatherLon: parseFloat(document.getElementById('weatherLon').value),
                weatherUnits: document.getElementById('weatherUnits').value,
                weatherDisplayStartMin: parseInt(document.getElementById('weatherDisplayStartMin').value),
                weatherDisplayStartMax: parseInt(document.getElementById('weatherDisplayStartMax').value),
                weatherDurationMin: parseInt(document.getElementById('weatherDurationMin').value),
                weatherDurationMax: parseInt(document.getElementById('weatherDurationMax').value)
            };
            
            try {
                const res = await fetch('/api/config', {
                    method: 'POST',
                    headers: {'Content-Type': 'application/json'},
                    body: JSON.stringify(cfg)
                });
                if (res.ok) {
                    showStatus('Settings saved!', false);
                } else {
                    showStatus('Save failed', true);
                }
            } catch(e) {
                showStatus('Connection error', true);
            }
        }
        
        async function restart() {
            if (confirm('Restart the device?')) {
                try {
                    await fetch('/api/restart', {method: 'POST'});
                    showStatus('Restarting...', false);
                } catch(e) {}
            }
        }
        
        async function searchCity() {
            const query = document.getElementById('citySearch').value.trim();
            if (!query) return;
            
            const resultsDiv = document.getElementById('searchResults');
            resultsDiv.innerHTML = '<div style="color:#888;padding:10px;">Searching...</div>';
            
            try {
                const res = await fetch(
                    `https://nominatim.openstreetmap.org/search?format=json&q=${encodeURIComponent(query)}&limit=5`,
                    { headers: { 'Accept': 'application/json' } }
                );
                const data = await res.json();
                
                if (data.length === 0) {
                    resultsDiv.innerHTML = '<div style="color:#ff5050;padding:10px;">No results found</div>';
                    return;
                }
                
                resultsDiv.innerHTML = data.map(place => 
                    `<div class="search-result" onclick="selectLocation(${place.lat}, ${place.lon}, '${place.display_name.replace(/'/g, "\\'")}')">` +
                    `${place.display_name}</div>`
                ).join('');
            } catch(e) {
                resultsDiv.innerHTML = '<div style="color:#ff5050;padding:10px;">Search failed</div>';
            }
        }
        
        async function selectLocation(lat, lon, name) {
            document.getElementById('weatherLat').value = lat.toFixed(4);
            document.getElementById('weatherLon').value = lon.toFixed(4);
            document.getElementById('locationDisplay').textContent = name;
            document.getElementById('searchResults').innerHTML = '';
            document.getElementById('citySearch').value = '';
            showStatus('Location set. Fetching timezone...', false);
            
            // Auto-detect timezone
            try {
                const res = await fetch(`https://api.open-meteo.com/v1/forecast?latitude=${lat}&longitude=${lon}&current=weather_code&timezone=auto`);
                const data = await res.json();
                
                if (data.utc_offset_seconds !== undefined) {
                    const offset = data.utc_offset_seconds;
                    document.getElementById('timezoneOffset').value = offset;
                    showStatus(`Location set. Timezone updated to UTC${offset >= 0 ? '+' : ''}${offset/3600}h`, false);
                } else {
                    showStatus('Location set. Could not detect timezone.', true);
                }
            } catch(e) {
                showStatus('Location set. Timezone fetch failed.', true);
            }
        }
        
        document.getElementById('citySearch').addEventListener('keypress', function(e) {
            if (e.key === 'Enter') { e.preventDefault(); searchCity(); }
        });
    </script>
</body>
</html>
)rawliteral";
    
    return html;
}
