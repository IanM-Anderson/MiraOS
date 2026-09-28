#ifndef STORAGE_H
#define STORAGE_H

#include <Arduino.h>
#include <LittleFS.h>
#include <string>

class Storage {
public:
    // Reading and writing using native ESP8266 LittleFS File objects
    static File open_file_read(const std::string &path);
    static File open_file_write(const std::string &path);
    static void close_file(File &file);
};

#endif