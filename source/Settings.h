#pragma once
#include <array>

class Settings {
public:
    CRGBA radarMapColor;
    CRGBA backgroundColor;
    CRGBA crosshairColor;
    CRGBA zoneNameColor;
    bool forceBlipsOnMap;
    bool skyUI;
    bool enableLegendBox;
    bool readStringsFromThisFile;
    std::array<std::string, 128> gxt;

public:
    void Read();
};

