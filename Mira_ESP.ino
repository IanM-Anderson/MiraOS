#include "System.h"

// This macro MUST remain in the main .ino file to correctly link the ESP8266 VCC reader
ADC_MODE(ADC_VCC); 

System sys;

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    // Pass your default fallback credentials here
    sys.init("YOUR_WIFI_SSID", "YOUR_WIFI_PASSWORD", "YOUR_GOOGLE_SCRIPT_URL");
}

void loop() {
    sys.run_loop();
}