#pragma once

class Settings {
public:
    CRGBA radarMapColor;
    CRGBA backgroundColor;
    CRGBA crosshairColor;
    CRGBA zoneNameColor;

public:
    void Read();
};

