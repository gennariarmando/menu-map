#pragma once
#include <string>
#include <vector>

struct ForcedBlip {
    float x;
    float y;
    int32_t sprite;
    int32_t island;

    ForcedBlip() {
        x = 0.0f;
        y = 0.0f;
        sprite = -1;
        island = 0;
    }
};

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
    std::vector<std::string> gxt;
    std::vector<ForcedBlip> forcedBlips;

public:
    void Read();
};

