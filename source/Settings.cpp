#include "plugin.h"
#include "Settings.h"

#include <cctype>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

std::string Trim(std::string value) {
    size_t start = 0;
    while (start < value.size() && std::isspace(static_cast<unsigned char>(value[start])))
        start++;

    size_t end = value.size();
    while (end > start && std::isspace(static_cast<unsigned char>(value[end - 1])))
        end--;

    return value.substr(start, end - start);
}

bool TryParseFloatToken(const std::string& token, float& out) {
    std::string t = Trim(token);
    if (t.empty())
        return false;

    char* endPtr = nullptr;
    out = strtof(t.c_str(), &endPtr);
    if (endPtr == t.c_str())
        return false;

    if (*endPtr == 'f' || *endPtr == 'F')
        endPtr++;

    while (*endPtr && std::isspace(static_cast<unsigned char>(*endPtr)))
        endPtr++;

    return *endPtr == '\0';
}

bool TryParseIntToken(const std::string& token, int32_t& out) {
    std::string t = Trim(token);
    if (t.empty())
        return false;

    char* endPtr = nullptr;
    auto value = strtol(t.c_str(), &endPtr, 10);
    if (endPtr == t.c_str())
        return false;

    while (*endPtr && std::isspace(static_cast<unsigned char>(*endPtr)))
        endPtr++;

    if (*endPtr != '\0')
        return false;

    out = static_cast<int32_t>(value);
    return true;
}

bool TryParseForcedBlip(const std::string& value, ForcedBlip& out) {
    std::stringstream stream(value);
    std::string part;
    std::vector<std::string> parts;

    while (std::getline(stream, part, ','))
        parts.push_back(part);

    if (parts.size() < 4)
        return false;

    if (!TryParseFloatToken(parts[0], out.x))
        return false;
    if (!TryParseFloatToken(parts[1], out.y))
        return false;

    if (!TryParseIntToken(parts[2], out.sprite))
        return false;

    if (!TryParseIntToken(parts[3], out.island))
        return false;

    return true;
}

void ReadForcedBlipsFromSection(const char* filePath, std::vector<ForcedBlip>& forcedBlips) {
    forcedBlips.clear();
    forcedBlips.reserve(64);

    std::ifstream file(filePath);
    if (!file.is_open())
        return;

    bool insideBlipsSection = false;
    std::string line;

    while (std::getline(file, line)) {
        std::string s = Trim(line);

        if (s.empty())
            continue;

        if (s.front() == '[' && s.back() == ']') {
            insideBlipsSection = (s == "[BLIPS]");
            continue;
        }

        if (!insideBlipsSection)
            continue;

        size_t commentPos = s.find(';');
        if (commentPos != std::string::npos)
            s = Trim(s.substr(0, commentPos));

        if (s.empty())
            continue;

        ForcedBlip parsed;
        if (TryParseForcedBlip(s, parsed))
            forcedBlips.push_back(parsed);
    }
}

void Settings::Read() {
#ifdef GTA3
    const char* configPath = PLUGIN_PATH("MenuMapIII.ini");
#else
    const char* configPath = PLUGIN_PATH("MenuMapVC.ini");
#endif

    plugin::config_file config(configPath);

    radarMapColor = config["RadarMapColor"].asRGBA(CRGBA(255, 255, 255, 255));
    backgroundColor = config["BackgroundColor"].asRGBA(CRGBA(0, 0, 0, 255));
    crosshairColor = config["CrosshairColor"].asRGBA(CRGBA(234, 171, 54, 255));
    zoneNameColor = config["ZoneNameColor"].asRGBA(CRGBA(255, 255, 255, 255));
    forceBlipsOnMap = config["ForceBlipsOnMap"].asBool(true);
    skyUI = config["SkyUI"].asBool(false);
    enableLegendBox = config["EnableLegendBox"].asBool(false);
    readStringsFromThisFile = config["ReadStringsFromThisFile"].asBool(true);

    gxt.assign(128, "");

    int i = 0;
    for (auto& it : gxt) {
        char buff[8];
        sprintf(buff, "LG_%02d", i);
        it = config[buff].asString("");
        i++;
    }

    ReadForcedBlipsFromSection(configPath, forcedBlips);
}
