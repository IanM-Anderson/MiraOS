#include "Network.h"
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>
#include <LittleFS.h>
#include <time.h>

void Network::connect(const char* ssid, const char* password) {
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    
    Serial.print("\n[Network] Syncing NTP Time...");
    configTime("CST6CDT,M3.2.0,M11.1.0", "pool.ntp.org", "time.nist.gov");
    
    time_t now = time(nullptr);
    while (now < 1000000000) { 
        delay(100);
        Serial.print(".");
        now = time(nullptr);
    }
    Serial.println("\n[Network] Time Synced!");
}

void Network::disconnect() {
    WiFi.disconnect(true);
    delay(1);
    WiFi.mode(WIFI_OFF);
    WiFi.forceSleepBegin();
    delay(1);
}



int Network::POST(const std::string &url, const std::string &payload) {
    WiFiClientSecure client;
    client.setInsecure();
    client.setBufferSizes(4096, 512); 
    
    HTTPClient http;
    if (!http.begin(client, url.c_str())) {
        Serial.println("[Network] POST HTTP begin failed (Malformed URL?)");
        return -1;
    }
    
    http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
    http.addHeader("Content-Type", "application/json");
    
    int httpCode = http.POST(payload.c_str());
    http.end();
    
    return httpCode;
}

bool Network::get_json(const std::string &url) {
    WiFiClientSecure client;
    client.setInsecure();
    client.setBufferSizes(4096, 512); 
    
    HTTPClient http;
    if (!http.begin(client, url.c_str())) {
        Serial.println("[Network] GET HTTP begin failed (Malformed URL?)");
        return false;
    }
    
    http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
    int httpCode = http.GET();

    if (httpCode == 200 || httpCode == 201) {
        File cache = LittleFS.open("/cache.json", "w");
        if (cache) {
            // Memory Fix: Stream directly from Wi-Fi to Flash Storage
            http.writeToStream(&cache); 
            cache.close();
            Serial.println("[Network] Live fetch successful. Layout cached.");
        }
    } else {
        Serial.printf("[Network] HTTP GET failed (Error: %d). Falling back...\n", httpCode);
        
        File cache = LittleFS.open("/cache.json", "r");
        if (cache && cache.size() > 0) {
            cache.close();
            Serial.println("[Network] Preserved existing layout in LittleFS cache.");
        } else {
            Serial.println("[Network] Cache empty. Generating emergency backup screen JSON.");
            File fallback = LittleFS.open("/cache.json", "w");
            if (fallback) {
                fallback.print("{\"screens\":[{\"name\":\"Home\",\"elements\":[{\"type\":\"text\",\"name\":\"backup_msg\",\"x\":\"50\",\"y\":\"200\",\"width\":\"700\",\"height\":\"100\",\"visible\":\"1\",\"focusable\":\"0\",\"color\":\"1\",\"text_color\":\"0\",\"text_size\":\"3\",\"text_align\":\"center\",\"text\":\"Mira, network offline. I love you!\"}]}],\"variables\":[]}");
                fallback.close();
            }
        }
    }
    
    http.end();
    return true; // Always return true because /cache.json is guaranteed to exist now
}