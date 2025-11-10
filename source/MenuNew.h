#pragma once
#ifdef GTASA
#define MAP_SIZE (6000.0f)
#else
#define MAP_SIZE (4000.0f)
#endif
#define MAP_ZOOM_MIN DEFAULT_SCREEN_HEIGHT / 2
#define MAP_ZOOM_MAX DEFAULT_SCREEN_HEIGHT
#define RADAR_TILE_SIZE (500)
#define RADAR_NUM_TILES (MAP_SIZE / RADAR_TILE_SIZE)
#define RADAR_BLIPS_SCALE 14.0f

#ifdef GTA3
#define LCSFICATION
//#define WITH_VCS_MAP_OPTIONS
#endif

#define WITH_ANIMATION

#define MAX_LEGEND_ENTRIES 64
#define LEGEND_BLIP_SCALE_X 20.0f
#define LEGEND_BLIP_SCALE_Y 20.0f

#define HUD_COLOUR_LCS_MENU 111, 165, 208

#include "plugin.h"
#include "CMenuManager.h"
#include "CRadar.h"
#include "CPickups.h"
#include "Settings.h"

#include "SpriteLoader.h"

enum {
#ifdef GTA3
    MENUPAGE_MAP = MENUPAGE_NO_MEMORY_CARD,
#endif
};

enum {
    RADAR_SHIFTED_NEGATIVE = 100,
    RADAR_DESTINATION = -1,
    RADAR_OBJECTIVE = - 2,
    RADAR_WAYPOINT = -8,

#ifdef WITH_VCS_MAP_OPTIONS
    RADAR_PACKAGE = -12,
    RADAR_RAMPAGE = -13,
    RADAR_UNIQUE_STUNT = -14,
#endif
};

enum {
    MODEL_KILLFRENZY = 1392,
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
    int previousTimeInMilliseconds;
    bool dontStreamRadarTiles;

#if defined(GTA3) && defined(LCSFICATION)
    bool m_bPrefsShowLegends;
#endif

#ifdef WITH_ANIMATION
    enum eAnimStates {
        ANIM_NONE,
        ANIM_FRAME,
        ANIM_DONE,
    };
    uint8_t m_nAnimState;
    int32_t m_nTimePassed;
    CVector2D m_vPrevMapBase;
#endif

#ifdef WITH_VCS_MAP_OPTIONS
    int32_t m_nMapOptionsKeyPressTime;
    bool m_bPrefsShowMapOptions;
    int32_t m_nCurrMapItem;

    enum eDiscoveredExtras {
        EXTRA_NONE,
        EXTRA_HIDDEN_PACKAGE,
        EXTRA_RAMPAGE,
        EXTRA_UNIQUE_STUNT,
        NUM_DISCOVERED_EXTRAS
    };
    int8_t m_nPrefsShowDiscoveredExtras;
#endif

    bool m_bRadarStreamed;

public:
    CMenuNew();
    ~CMenuNew();

    void SetViewport();
    void RestoreViewport();

    uint32_t GetAlpha(uint32_t a = 255);
    float GetMenuOffsetX();
    bool GetInputEnabled();
    bool GetLcsfication();
    int GetTimeToWait();

    void DrawMap();
    void DrawCrosshair(float x, float y);
    void DrawZone();
    void DrawBlips();
    CVector2D WorldToMap(CVector pos);
    CVector MapToWorld(CVector2D in);
    void StreamRadarSections();
#ifdef WITH_ANIMATION
    void MoveMapToPosition(CVector2D target, CVector2D screenTarget, float t);
    void ProcessAnimStates();
#endif
    void MapInput();
    void ResetMap(bool resetCrosshair = false);
    void DrawRadarSectionMap(int x, int y, CRect const& rect, CRGBA const& col);
    void SetWaypoint(float x, float y);
    float GetMenuMapTileSize();
    float GetMenuMapWholeSize();
    float GetMenuMapHalfSize();
    void DrawLegend();

#if defined(GTA3) && defined(LCSFICATION)
    template<typename T>
    void DrawTextForLegendBox(float x, float y, const T* str, CRGBA const& col);
    void DrawLegendEntry(float x, float y, short id, CRGBA* col);
    void AddBlipToToLegendList(short id, CRGBA const& col = 0);
#endif

#ifdef WITH_VCS_MAP_OPTIONS
    void DrawDiscoveredExtrasBlips();
#endif

};

extern std::unique_ptr<CMenuNew> MenuNew;
extern CSprite2d** RadarSpritesArray;
extern tRadarTrace* RadarTraceArray;
extern uint8_t RadarTraceArraySize;

extern plugin::SpriteLoader spriteLoader;

#ifdef WITH_VCS_MAP_OPTIONS
extern CSprite2d RadarPackageSprite;
extern CSprite2d RadarRampageSprite;
extern CSprite2d RadarStuntJumpSprite;

struct Collectable {
    int32_t index;
    CVector pos;
    bool removed;

    Collectable() {
        index = -1;
        pos = { 0.0f, 0.0f, 0.0f };
        removed = false;
    }
};

enum eMenuMapOptions {
    MAP_OPTION_SHOW_DISCOVERED_EXTRA,
};

struct MenuMapOptions {
    uint8_t item;
    std::string str;
};

extern std::vector<Collectable> aPackages;
extern std::vector<Collectable> aRampages;
extern std::vector<Collectable> aUniqueStunts;

extern bool Debug_ShowAllHiddenPackages;
extern bool Debug_ShowAllRampages;
extern bool Debug_ShowAllUniqueStunts;

#endif
