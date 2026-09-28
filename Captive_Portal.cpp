#include "Captive_Portal.h"
#include "Variables.h"
#include "Storage.h"
#include "ArduinoJson-v7.4.3.h"
#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>

const byte DNS_PORT = 53;
IPAddress apIP(192, 168, 4, 1);
DNSServer dnsServer;
ESP8266WebServer webServer(80);

const char* html_page = R"=====(
<!DOCTYPE html>
<html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    body { font-family: Arial, sans-serif; margin: 40px auto; max-width: 400px; padding: 20px; background: #f4f4f9; }
    h2 { color: #333; }
    input[type=text], input[type=password] { width: 100%; padding: 10px; margin: 10px 0; border: 1px solid #ccc; border-radius: 4px; box-sizing: border-box; }
    input[type=submit] { width: 100%; background-color: #4CAF50; color: white; padding: 14px 20px; margin: 10px 0; border: none; border-radius: 4px; cursor: pointer; font-size: 16px; }
    input[type=submit]:hover { background-color: #45a049; }
  </style>
</head>
<body>
  <h2>Mira Setup</h2>
  <form action="/" method="POST">
    <label for="ssid">Wi-Fi Name (SSID)</label>
    <input type="text" id="ssid" name="wifi_ssid" placeholder="HomeNetwork" required>
    
    <label for="pass">Wi-Fi Password</label>
    <input type="password" id="pass" name="wifi_pass" placeholder="Password">
    
    <label for="url">Google Apps Script URL</label>
    <input type="text" id="url" name="web_app_url" placeholder="https://script.google.com/..." required>
    
    <input type="submit" value="Save & Reboot">
  </form>
</body>
</html>
)=====";

void handleRoot() {
    if (webServer.method() == HTTP_GET) {
        webServer.send(200, "text/html", html_page);
    } 
    else if (webServer.method() == HTTP_POST) {
        std::string new_ssid = webServer.arg("wifi_ssid").c_str();
        std::string new_pass = webServer.arg("wifi_pass").c_str();
        std::string new_url = webServer.arg("web_app_url").c_str();

        JsonDocument doc;
        // Keep existing config if any exists
        File inFile = Storage::open_file_read("/config.json");
        if (inFile) {
            deserializeJson(doc, inFile);
            Storage::close_file(inFile);
        }

        doc["wifi_ssid"] = new_ssid;
        doc["wifi_pass"] = new_pass;
        doc["web_app_url"] = new_url;

        File outFile = Storage::open_file_write("/config.json");
        if (outFile) {
            serializeJson(doc, outFile);
            Storage::close_file(outFile);
        }

        webServer.send(200, "text/html", "<h2>Settings Saved!</h2><p>Mira is rebooting. You can close this page.</p>");
        delay(2000);
        ESP.restart(); // Hard reboot to apply new credentials
    }
}

void Captive_Portal::run() {
    Serial.println("[Captive Portal] Starting AP Mode...");

    // Disconnect existing Wi-Fi and setup Access Point
    WiFi.disconnect();
    WiFi.mode(WIFI_AP);
    WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
    WiFi.softAP("Mira_Setup");

    // Start DNS server to hijack all requests and route to the ESP IP
    dnsServer.start(DNS_PORT, "*", apIP);

    webServer.on("/", handleRoot);
    webServer.onNotFound([]() {
        webServer.sendHeader("Location", "/", true);
        webServer.send(302, "text/plain", "");
    });
    webServer.begin();

    // Draw the instructions to the E-Paper display
    display.setFullWindow();
    display.firstPage();
    do {
        display.fillScreen(COLOR_WHITE);
        display.setTextColor(COLOR_BLACK);
        
        display.setTextSize(3);
        display.setCursor(20, 50);
        display.print("NETWORK SETUP");

        display.setTextSize(2);
        display.setCursor(20, 120);
        display.print("1. Connect your phone to Wi-Fi:");
        display.setCursor(20, 160);
        display.print("   Mira_Setup");

        display.setCursor(20, 220);
        display.print("2. A setup page will appear.");
        display.setCursor(20, 260);
        display.print("   (Or open http://192.168.4.1)");
        
    } while (display.nextPage());
    display.hibernate();

    // Infinite trap loop: handle web server traffic until the user hits save and triggers ESP.restart()
    while (true) {
        dnsServer.processNextRequest();
        webServer.handleClient();
        delay(10);
    }
}