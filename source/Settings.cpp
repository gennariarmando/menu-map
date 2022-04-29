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

}
