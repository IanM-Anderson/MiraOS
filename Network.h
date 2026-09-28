#ifndef NETWORK_H
#define NETWORK_H

#include <string>
#include <Arduino.h>

class Network {
public:
    static void connect(const char* ssid, const char* password);
    static void disconnect();
    
    // Changed return type from std::string to bool
    static bool get_json(const std::string &url); 
    
    static int POST(const std::string &url, const std::string &payload);
};

#endif