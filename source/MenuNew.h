#pragma once
#define RADAR_START (-4000.0f / 2)
#define RADAR_END (4000.0f / 2)
#define RADAR_SIZE (RADAR_END - RADAR_START)

#define RADAR_TILE_SIZE (500)
#define RADAR_NUM_TILES (RADAR_SIZE / RADAR_TILE_SIZE)

#include "plugin.h"
#include "CMenuManager.h"

enum {
	MENUPAGE_MAP = MENUPAGE_NO_MEMORY_CARD,
};

class CMenuNew : public CMenuManager {
public:
	float m_fMapZoom;
	CVector2D m_vMapBase;
	CVector2D m_vCrosshair;

public:
	void DrawMap();
	void DrawCrosshair(float x, float y);
	void StreamRadarSections();
	void MapInput();
	void DoMapZoomInOut(bool out);
	void ResetMap();
	void DrawRadarSectionMap(int x, int y, CRect const& rect, CRGBA const& col);

	float GetMenuMapTileSize();
	float GetMenuMapWholeSize();
};
