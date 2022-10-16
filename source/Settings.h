#pragma once

class Settings {
public:
    CRGBA radarMapColor;
    CRGBA backgroundColor;
    CRGBA crosshairColor;
    CRGBA zoneNameColor;
    bool forceBlipsOnMap;

public:
    void Read();
};

