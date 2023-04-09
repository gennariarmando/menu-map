#pragma once
#ifdef GTASA
#define MAP_SIZE (6000.0f)
#else
#define MAP_SIZE (4000.0f)
#endif
#define MAP_ZOOM_MIN 2.1f
#define MAP_ZOOM_MAX 7.1f
#define RADAR_TILE_SIZE (500)
#define RADAR_NUM_TILES (MAP_SIZE / RADAR_TILE_SIZE)
#define RADAR_BLIPS_SCALE 14.0f

#define LCSFICATION 1
#define MAX_LEGEND_ENTRIES 32
#define LEGEND_BLIP_SCALE 14.0f

#include "plugin.h"
#include "CMenuManager.h"
#include "CRadar.h"
#include "Settings.h"

enum {
#ifdef GTA3
    MENUPAGE_MAP = MENUPAGE_NO_MEMORY_CARD,
#endif
};

class CMenuNew {
public:
    CMenuManager* menuManager;
    float m_fMapZoom;
    CVector2D m_vMapBase;
    CVector2D m_vCrosshair;
    int targetBlipIndex;
    CVector targetBlipWorldPos;
    bool clearInput;
    Settings settings;

#ifdef LCSFICATION
    bool m_bMapLegend;
#endif

public:
    CMenuNew();
    ~CMenuNew();
    void DrawMap();
    void DrawCrosshair(float x, float y);
    void DrawZone();
    void DrawBlips();
    CVector2D WorldToMap(CVector pos);
    CVector MapToWorld(CVector2D in);
    void StreamRadarSections();
    void MapInput();
    void DoMapZoomInOut(bool out);
    void ResetMap(bool resetCrosshair = false);
    void DrawRadarSectionMap(int x, int y, CRect const& rect, CRGBA const& col);
    void SetWaypoint(float x, float y);
    float GetMenuMapTileSize();
    float GetMenuMapWholeSize();
    void DrawLegend();

#ifdef LCSFICATION
    void DrawLegendEntry(float x, float y, short id, CRGBA* col);
    void AddBlipToToLegendList(short id, CRGBA const& col = 0);
#endif

};

extern std::unique_ptr<CMenuNew> MenuNew;
extern CSprite2d** RadarSpritesArray;
