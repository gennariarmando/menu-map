#include "plugin.h"
#include "Settings.h"

void Settings::Read() {
#ifdef GTA3
    plugin::config_file config(PLUGIN_PATH("MenuMapIII.ini"));
#else
    plugin::config_file config(PLUGIN_PATH("MenuMapVC.ini"));
#endif

    radarMapColor = config["RadarMapColor"].asRGBA(CRGBA(255, 255, 255, 255));
    backgroundColor = config["BackgroundColor"].asRGBA(CRGBA(0, 0, 0, 255));
    crosshairColor = config["CrosshairColor"].asRGBA(CRGBA(234, 171, 54, 255));
    zoneNameColor = config["ZoneNameColor"].asRGBA(CRGBA(255, 255, 255, 255));
    forceBlipsOnMap = config["ForceBlipsOnMap"].asBool(true);
    skyUI = config["SkyUI"].asBool(false);
    enableLegendBox = config["EnableLegendBox"].asBool(false);
    readStringsFromThisFile = config["ReadStringsFromThisFile"].asBool(true);

    int i = 0;
    for (auto& it : gxt) {
        char buff[8];
        sprintf(buff, "LG_%02d", i);
        it = config[buff].asString("");
        i++;
    }
}
