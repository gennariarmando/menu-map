#pragma once
#define MAP_SIZE (4000.0f)
#define MAP_ZOOM_MIN 2.1f
#define MAP_ZOOM_MAX 7.1f
#define RADAR_TILE_SIZE (500)
#define RADAR_NUM_TILES (MAP_SIZE / RADAR_TILE_SIZE)
#define RADAR_BLIPS_SCALE 14.0f

#include "plugin.h"
#include "CMenuManager.h"
#include "CRadar.h"

enum {
    MENUPAGE_MAP = MENUPAGE_NO_MEMORY_CARD,
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
};

extern std::shared_ptr<CMenuNew> MenuNew;
