#include "Storage.h"

File Storage::open_file_read(const std::string &path)
{
    File file = LittleFS.open(path.c_str(), "r");
    if (!file)
    {
        Serial.printf("[Storage ERROR] Could not open file for reading: %s\n", path.c_str());
    }
    return file;
}

File Storage::open_file_write(const std::string &path)
{
    File file = LittleFS.open(path.c_str(), "w");
    if (!file)
    {
        Serial.printf("[Storage ERROR] Could not create/open file for writing: %s\n", path.c_str());
    }
    return file;
}

void Storage::close_file(File &file)
{
    if (file)
    {
        file.close();
    }
}