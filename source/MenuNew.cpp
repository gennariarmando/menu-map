#include "MenuNew.h"
#include "CPad.h"
#include "CSprite2d.h"
#include "CTxdStore.h"
#include "CStreaming.h"
#include "CWorld.h"
#include "Utility.h"
#include "CTheScripts.h"
#ifndef GTASA
#include "cDMAudio.h"
#include "d3d8.h"
#endif
#include "CTheZones.h"
#include "CFont.h"
#include "CText.h"

#ifdef LCSFICATION
#include "CStats.h"
#include "Other.h"

static short MapLegendList[MAX_LEGEND_ENTRIES] = {};
static short MapLegendCounter = 0;
CRGBA MapLegendBlipColor[MAX_LEGEND_ENTRIES] = {};
#endif

#include "SkyUIAPI.h"

std::unique_ptr<CMenuNew> MenuNew;
#ifdef GTASA
CSprite2d** RadarSpritesArray = &CRadar::RadarBlipSprites;
#else
CSprite2d** RadarSpritesArray = pRadarSprites;
#endif
tRadarTrace* RadarTraceArray = CRadar::ms_RadarTrace;
uint8_t RadarTraceArraySize = 32;

#ifdef WITH_VCS_MAP_OPTIONS
CSprite2d RadarPackageSprite = {};
CSprite2d RadarRampageSprite = {};
CSprite2d RadarStuntJumpSprite = {};
std::vector<Collectable> aPackages = {};
std::vector<Collectable> aRampages = {};
std::vector<Collectable> aUniqueStunts = {};

bool Debug_ShowAllHiddenPackages = false;
bool Debug_ShowAllRampages = false;
bool Debug_ShowAllUniqueStunts = false;
#endif

#ifndef GTASA
D3DVIEWPORT8 previousViewport = {};
D3DVIEWPORT8 newViewport = {};
#endif

plugin::SpriteLoader spriteLoader = {};

#ifdef WITH_VCS_MAP_OPTIONS
std::vector<MenuMapOptions> aMenuMapItems = {
    { MAP_OPTION_SHOW_DISCOVERED_EXTRA, "FE_DIXA" },

};

#endif

CMenuNew::CMenuNew() {
    menuManager = &FrontEndMenuManager;
    m_fMapZoom = MAP_ZOOM_MIN;
    m_vMapBase = {};
    m_vCrosshair = {};
    targetBlipIndex = 0;
    targetBlipWorldPos = {};
    clearInput = false;
    settings.Read();
#if defined(GTA3) && defined(LCSFICATION)
    m_bPrefsShowLegends = true;
#endif
    previousTimeInMilliseconds = 0;

    dontStreamRadarTiles = false;

#ifdef WITH_ANIMATION
    m_nAnimState = ANIM_NONE;
    m_nTimePassed = CTimer::m_snTimeInMillisecondsPauseMode;
#endif

#ifdef WITH_VCS_MAP_OPTIONS
    m_nMapOptionsKeyPressTime = 0;
    m_bPrefsShowMapOptions = false;
    m_nCurrMapItem = 0;
    m_nPrefsShowDiscoveredExtras = EXTRA_NONE;
#endif

    m_bRadarStreamed = false;
}

CMenuNew::~CMenuNew() {
    menuManager = nullptr;
}

void CMenuNew::SetViewport() {
    auto dev = (IDirect3DDevice8*)GetD3DDevice();

#ifndef GTASA
    if (settings.skyUI) {
        dev->GetViewport(&previousViewport);

        if (GetLcsfication()) {
            // if (!GetInputEnabled()) {
            newViewport.X = 0.0f;
            newViewport.Y = 0.0f;
            newViewport.Width = SCREEN_WIDTH - newViewport.X;
            newViewport.Height = ScaleY(DEFAULT_SCREEN_HEIGHT - 90.25f) - newViewport.Y;
            //newViewport.X = ScaleX(11.5f);
            //newViewport.Y = ScaleY(5.5f);
            //newViewport.Width = SCREEN_WIDTH - ScaleX(11.5f) - newViewport.X;
            //newViewport.Height = ScaleY(DEFAULT_SCREEN_HEIGHT - 97.0f) - newViewport.Y;
        //}
        //else {
        //    newViewport.X = 0.0f;
        //    newViewport.Y = 0.0f;
        //    newViewport.Width = SCREEN_WIDTH;
        //    newViewport.Height = SCREEN_HEIGHT;
        //}
        }
        else {
            newViewport.X = ScaleXKeepCentered(32.0f) + GetMenuOffsetX();
            newViewport.Y = ScaleY(42.0f);
            newViewport.Width = ScaleXKeepCentered(DEFAULT_SCREEN_WIDTH - 32.0f) - newViewport.X + GetMenuOffsetX();
            newViewport.Height = ScaleY(DEFAULT_SCREEN_HEIGHT - 102.0f) - newViewport.Y;
        }
        dev->SetViewport(&newViewport);
    }
#endif
}

void CMenuNew::RestoreViewport() {
    auto dev = (IDirect3DDevice8*)GetD3DDevice();

#ifndef GTASA 
    if (settings.skyUI)
        dev->SetViewport(&previousViewport);
#endif
}

uint32_t CMenuNew::GetAlpha(uint32_t a) {
#ifdef GTASA
    return a;
#else
    if (settings.skyUI)
        return std::min(menuManager->FadeIn(a), (int32_t)SkyUI::GetAlpha(a));

    return menuManager->FadeIn(a);
#endif
}

float CMenuNew::GetMenuOffsetX() {
    if (settings.skyUI)
        return ScaleX(SkyUI::GetMenuOffsetX());

    return 0.0f;
}

bool CMenuNew::GetInputEnabled() {
    if (settings.skyUI)
        return SkyUI::GetCurrentInput() == 1 && SkyUI::GetCheckHoverForStandardInput(menuManager);

    return true;
}

bool CMenuNew::GetLcsfication() {
    if (settings.skyUI)
        return SkyUI::GetGTA3LCS();

    return false;
}

int CMenuNew::GetTimeToWait() {
    if (settings.skyUI)
        return SkyUI::GetTimeToWaitBeforeStateChange();

    return 0;
}

void CMenuNew::DrawMap() {
    if (!menuManager)
        return;

    if (GetTimeToWait() == -1)
        return;

#ifdef GTA3
    SetViewport();
#endif

#ifdef GTAVC
    menuManager->m_bDrawRadarOrMap = true;

    // Used to draw legend.
    CRadar::InitFrontEndMap();
    CRadar::DrawBlips();
#endif
    RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, reinterpret_cast<void*>(FALSE));
    RwRenderStateSet(rwRENDERSTATESRCBLEND, reinterpret_cast<void*>(rwBLENDSRCALPHA));
    RwRenderStateSet(rwRENDERSTATEDESTBLEND, reinterpret_cast<void*>(rwBLENDINVSRCALPHA));
    RwRenderStateSet(rwRENDERSTATEFOGENABLE, reinterpret_cast<void*>(FALSE));
    RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, reinterpret_cast<void*>(FALSE));
    RwRenderStateSet(rwRENDERSTATEZTESTENABLE, reinterpret_cast<void*>(FALSE));

    CSprite2d::DrawRect(CRect(-5.0f + GetMenuOffsetX(), -5.0f, SCREEN_WIDTH + 5.0f + GetMenuOffsetX(), SCREEN_HEIGHT + 5.0f), CRGBA(0, 0, 0, GetAlpha()));
    CSprite2d::DrawRect(CRect(-5.0f + GetMenuOffsetX(), -5.0f, SCREEN_WIDTH + 5.0f + GetMenuOffsetX(), SCREEN_HEIGHT + 5.0f), CRGBA(settings.backgroundColor.r, settings.backgroundColor.g, settings.backgroundColor.b, GetAlpha(settings.backgroundColor.a)));

    CRGBA col = { settings.radarMapColor.r, settings.radarMapColor.g, settings.radarMapColor.b,
#ifdef GTASA
        255
#else
        static_cast<unsigned char>(GetAlpha(settings.radarMapColor.a))
#endif
    };
    CRect rect;

    const float mapHalfSize = GetMenuMapHalfSize();
    float mapZoom = m_fMapZoom;
    rect.left = m_vMapBase.x + GetMenuOffsetX();
    rect.top = m_vMapBase.y;
    rect.right = rect.left + mapZoom;
    rect.bottom = rect.top + mapZoom;

    if (!dontStreamRadarTiles)
        StreamRadarSections();

    for (int y = 0; y < RADAR_NUM_TILES; y++) {
        for (int x = 0; x < RADAR_NUM_TILES; x++) {
            DrawRadarSectionMap(x, y, CRect((rect.left - mapHalfSize), (rect.top - mapHalfSize), (rect.right - mapHalfSize), (rect.bottom - mapHalfSize)), col);
            rect.left += mapZoom;
            rect.right = rect.left + mapZoom;
        }
        rect.left = m_vMapBase.x + GetMenuOffsetX();
        rect.right = rect.left + mapZoom;

        rect.top += mapZoom;
        rect.bottom = rect.top + mapZoom;
    }

#ifdef LCSFICATION
#ifdef GTA3
    CRect dark;

    dark.left = m_vMapBase.x + GetMenuOffsetX();
    dark.top = m_vMapBase.y;
    dark.right = m_vMapBase.x + mapZoom + GetMenuOffsetX();
    dark.bottom = m_vMapBase.y + mapZoom;

    dark.left -= mapHalfSize;
    dark.top -= mapHalfSize;
    dark.right -= mapHalfSize;

    CRGBA darkCol = settings.backgroundColor;
    darkCol.a = GetAlpha(200);

    // Right
    //CSprite2d::DrawRect(CRect(dark.right + (mapZoom * 4.4f), dark.top + (mapZoom * 3.4f), dark.right + (mapZoom * 8.0f), dark.bottom + mapHalfSize), darkCol);

    // Center
    if (!CStats::IndustrialPassed) {
        CSprite2d::DrawRect(CRect(dark.right + (mapZoom * 2.3f), dark.top + (mapZoom * 3.4f), dark.right + (mapZoom * 4.4f), dark.bottom + mapHalfSize), darkCol);
    }
    
    // Left
    if (!CStats::CommercialPassed) {
        CSprite2d::DrawRect(CRect(dark.left, dark.top + (mapZoom * 3.4f), dark.right + (mapZoom * 2.3f), dark.bottom + mapHalfSize), darkCol);
        CSprite2d::DrawRect(CRect(dark.left, dark.top, dark.right + (mapZoom * 8.0f), dark.top + (mapZoom * 3.4f)), darkCol);
    
        if (CStats::IndustrialPassed) {
            CSprite2d::DrawRect(CRect(
                dark.right + (mapZoom * 2.3f), dark.top + (mapZoom * 3.4f), dark.right + (mapZoom * 2.6f), dark.bottom - (mapZoom * 1.1f)), darkCol);
    
    
            CSprite2d::DrawRect(CRect(
                dark.right + (mapZoom * 2.6f), dark.top + (mapZoom * 3.4f), dark.right + (mapZoom * 2.7f), dark.bottom - (mapZoom * 1.3f)), darkCol);
        }
    }

#endif
#endif

#ifdef WITH_VCS_MAP_OPTIONS
    DrawDiscoveredExtrasBlips();
#endif

    DrawBlips();
    DrawCrosshair(m_vCrosshair.x, m_vCrosshair.y);

#ifdef LCSFICATION
    if (GetLcsfication())
        RestoreViewport();
#endif

    if (GetInputEnabled() || !GetLcsfication())
        DrawZone();

#ifdef LCSFICATION
    if (GetLcsfication())
        SetViewport();
#endif

    SetScaleMult(0.8f);
#ifdef GTAVC
    DrawLegend();
    menuManager->m_bDrawRadarOrMap = false;
    menuManager->DisplayHelperText("FEH_MPH");
#endif

#ifdef LCSFICATION
    if (settings.enableLegendBox)
        DrawLegend();

    SetScaleMult(1.0f);

    MapLegendCounter = 0;
    memset(MapLegendList, 0, sizeof(MapLegendList));
#endif

#ifdef GTA3
    RestoreViewport();
#endif
}

void CMenuNew::DrawCrosshair(float x, float y) {
    if (!GetInputEnabled())
        return;

    float lineSize = ScaleY(2.0f);
    CRGBA lineCol = CRGBA(settings.crosshairColor.r, settings.crosshairColor.g, settings.crosshairColor.b, 
#ifdef GTASA
        255
#else
        GetAlpha(settings.crosshairColor.a)
#endif
    );

    RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, reinterpret_cast<void*>(TRUE));

    // Ver
    CSprite2d::DrawRect(CRect(((x)-lineSize), 0.0f, ((x)+lineSize), SCREEN_HEIGHT), lineCol);

    // Hor
    CSprite2d::DrawRect(CRect(0.0f, ((y)-lineSize), SCREEN_WIDTH, ((y)+lineSize)), lineCol);
}

void CMenuNew::DrawZone() {
    RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, reinterpret_cast<void*>(FALSE));

#ifdef GTA3
    if (!GetLcsfication()) {
        if (settings.skyUI)
            CSprite2d::DrawRect(CRect(-5.0f + GetMenuOffsetX(), SCREEN_HEIGHT + 5.0f, SCREEN_WIDTH + 5.0f + GetMenuOffsetX(), ScaleY(DEFAULT_SCREEN_HEIGHT - 126.0f)), CRGBA(0, 0, 0, GetAlpha(255)));
        else
            CSprite2d::DrawRect(CRect(-5.0f, SCREEN_HEIGHT + 5.0f, SCREEN_WIDTH + 5.0f, SCREEN_HEIGHT - ScaleY(42.0f)), CRGBA(10, 10, 10, GetAlpha(255)));
    }

#endif
    CVector pos = MapToWorld(CVector2D(m_vCrosshair.x, m_vCrosshair.y));

#ifdef GTA3
    CZone* zoneType0 = CTheZones::FindSmallestZonePositionType(pos, 0);
    CZone* zoneType1 = CTheZones::FindSmallestZonePositionType(pos, 1);
#elif GTAVC
    CZone* zoneType0 = CTheZones::FindSmallestNavigationZoneForPosition(&pos, true, true);
#elif GTASA
    CZone* zoneType0 = CTheZones::FindSmallestZoneForPosition(pos, false);
#endif

#ifdef GTASA
    const char* str = TheText.Get("CITYZON");
#else
    const wchar_t* str = TheText.Get("CITYZON");
#endif

    if (GetInputEnabled()) {
        if (zoneType0)
            str = zoneType0->GetTranslatedName();

#ifdef GTA3
        if (zoneType1)
            str = zoneType1->GetTranslatedName();
#endif
    }

    if (str) {
#ifndef GTASA
        CFont::SetAlphaFade(255.0f);
        CFont::SetPropOn();
        CFont::SetBackgroundOff();
        CFont::SetCentreOff();
        CFont::SetRightJustifyOff();
#else
        CFont::SetProportional(true);
        CFont::SetBackground(false, false);
        CFont::SetOrientation(ALIGN_LEFT);
#endif
        CFont::SetWrapx(SCREEN_WIDTH);
        CFont::SetRightJustifyWrap(0.0f);
        CFont::SetColor(CRGBA(settings.zoneNameColor.r, settings.zoneNameColor.g, settings.zoneNameColor.b, 
#ifdef GTASA
            255
#else
            GetAlpha(settings.zoneNameColor.a)
#endif
        ));
#ifdef GTA3
        CFont::SetDropColor(CRGBA(0, 0, 0, GetAlpha(255)));
        CFont::SetDropShadowPosition(0);
        CFont::SetRightJustifyOff();
        CFont::SetFontStyle(0);
        CFont::SetScale(ScaleX(0.52f), ScaleY(1.10f));
        if (settings.skyUI) {
            float x = ScaleXKeepCentered(52.0f) + GetMenuOffsetX();
            float y = ScaleY(DEFAULT_SCREEN_HEIGHT - 126.0f);
            if (GetLcsfication()) {
                x = ScaleX(26.0f);
                y = ScaleY(DEFAULT_SCREEN_HEIGHT - 44.0f);
                str = UpperCase(str);
            }
            CFont::PrintString(x, y, str);
        }
        else
            CFont::PrintString(ScaleX(16.0f), SCREEN_HEIGHT - ScaleY(34.0f), str);
#elif GTAVC
        CFont::SetDropColor(CRGBA(0, 0, 0, 255));
        CFont::SetDropShadowPosition(2);
        CFont::SetFontStyle(2);
        CFont::SetScale(ScaleX(0.64f), ScaleY(1.28f));
        if (settings.skyUI) {
            CFont::SetRightJustifyOff();
            CFont::PrintString(ScaleXKeepCentered(78.0f) + GetMenuOffsetX(), ScaleY(DEFAULT_SCREEN_HEIGHT - 138.0f), str);
        }
        else {
            CFont::SetRightJustifyOn();
            CFont::PrintString(SCREEN_WIDTH - ScaleX(72.0f), SCREEN_HEIGHT - ScaleY(102.0f), str);
        }
#endif
    }

    CFont::DrawFonts();
}

void CMenuNew::DrawBlips() {
    int traceSize = 32;
   
    if (this->GetLcsfication())
        traceSize = RadarTraceArraySize;

#ifdef GTA3
    for (int i = 0; i < traceSize; i++) {
#else
    for (int i = 0; i < 75; i++) {
#endif
        tRadarTrace& trace = RadarTraceArray[i];

        if (!trace.m_bInUse)
            continue;

        CPlayerPed* playa = FindPlayerPed();
        CVector2D pos = WorldToMap(trace.m_vecPos);

        CRGBA col = CRadar::GetRadarTraceColour(trace.m_nColour, true
#ifdef GTASA
            , trace.m_bFriendly
#endif
        );
        col.a = GetAlpha(col.a);
        unsigned short id = trace.m_nRadarSprite;
        int handle = trace.m_nEntityHandle;
        CEntity* e = NULL;

        if (trace.m_nBlipDisplay < BLIP_DISPLAY_BLIP_ONLY)
            continue;

        switch (trace.m_nBlipType) {
        case BLIP_COORD:
        case BLIP_CONTACTPOINT:
            if (!CTheScripts::IsPlayerOnAMission() || trace.m_nBlipType == BLIP_COORD) {
                if (id != RADAR_SPRITE_NONE) {
                    CSprite2d* sprite =
#ifdef GTASA
                        & CRadar::RadarBlipSprites[id];
#else
                        RadarSpritesArray[id];
#endif
                    DrawSpriteWithRotation(sprite, pos.x + GetMenuOffsetX(), pos.y, ScaleX(RADAR_BLIPS_SCALE), ScaleY(RADAR_BLIPS_SCALE), 0.0f, CRGBA(255, 255, 255,
#ifdef GTASA
                        255
#else
                        GetAlpha(255)
#endif
                    ));

#if defined(GTA3) && defined(LCSFICATION)
                    AddBlipToToLegendList(id);
#endif
                }
                else {
                    int level = 0;
                    float diff = playa->GetPosition().z - trace.m_vecPos.z;
                    if (playa) {
                        if (diff > 1.0f)
                            level = 1;
                        else if (diff < -1.0)
                            level = 2;
                        else
                            level = 0;
                    }
                    DrawLevel(pos.x + GetMenuOffsetX(), pos.y, ScaleX(RADAR_BLIPS_SCALE * 0.5f), ScaleY(RADAR_BLIPS_SCALE * 0.5f), level, CRGBA(col.r, col.g, col.b,
#ifdef GTASA
                        255
#else
                        GetAlpha(255)
#endif                    
                    ));
#if defined(GTA3) && defined(LCSFICATION)
                    AddBlipToToLegendList(RADAR_DESTINATION, col);
#endif               
                }
            }
            break;
        case BLIP_CAR:
        case BLIP_CHAR:
        case BLIP_OBJECT:
            if (handle == -1)
                break;

            switch (trace.m_nBlipType) {
            case BLIP_CAR:
                e = CPools::GetVehicle(handle);
                break;
            case BLIP_CHAR:
                e = CPools::GetPed(handle);

                if (e) {
                    CPed* p = static_cast<CPed*>(e);
                    if (p &&
#ifdef GTASA
                        p->m_nPedFlags.bInVehicle
#else
                        p->m_bInVehicle
#endif
                        ) {
                        CVehicle* v = p->m_pVehicle;
                        if (v)
                            e = v;
                    }
                }
                break;
            case BLIP_OBJECT:
                e = CPools::GetObject(handle);
                break;
            }

            if (e) {
                pos = WorldToMap(e->GetPosition());

                if (id != RADAR_SPRITE_NONE) {
                    CSprite2d* sprite =
#ifdef GTASA
                        & CRadar::RadarBlipSprites[id];
#else
                        RadarSpritesArray[id];
#endif
                    DrawSpriteWithRotation(sprite, pos.x + GetMenuOffsetX(), pos.y, ScaleX(RADAR_BLIPS_SCALE), ScaleY(RADAR_BLIPS_SCALE), 0.0f, CRGBA(255, 255, 255,
#ifdef GTASA
                        255
#else
                        GetAlpha(255)
#endif
                    ));

#if defined(GTA3) && defined(LCSFICATION)
                    AddBlipToToLegendList(id, col);
#endif
                }
                else {
                    int level = 0;
                    float diff = playa->GetPosition().z - e->GetPosition().z;
                    if (playa) {
                        if (diff > 1.0f)
                            level = 1;
                        else if (diff < -1.0)
                            level = 2;
                        else
                            level = 0;
                    }
                    DrawLevel(pos.x + GetMenuOffsetX(), pos.y, ScaleX(RADAR_BLIPS_SCALE * 0.5f), ScaleY(RADAR_BLIPS_SCALE * 0.5f), level, CRGBA(col.r, col.g, col.b,
#ifdef GTASA
                        255
#else
                        GetAlpha(255)
#endif
                    ));
#if defined(GTA3) && defined(LCSFICATION)
                    AddBlipToToLegendList(RADAR_OBJECTIVE, col);
#endif
                }
            }
            break;
        }
    }

#ifdef GTA3
    // Force drawing of some blips, III only.
    if (settings.forceBlipsOnMap) {
        struct fb {
            float x;
            float y;
            unsigned short sprite;
            unsigned char island;
        };
        std::vector<fb> pos = {
            { 1071.2f, -400.0f, RADAR_SPRITE_WEAPON, 0 }, // ammu1
            { 345.5f, -713.5f, RADAR_SPRITE_WEAPON, 1 }, // ammu2
            { -1200.8f, -24.5f, RADAR_SPRITE_WEAPON, 2 }, // ammu3
            { 925.0f, -359.5f, RADAR_SPRITE_SPRAY, 0 }, // spray1
            { 379.0f, -493.7f, RADAR_SPRITE_SPRAY, 1  }, // spray2
            { -1142.0f, 34.7f, RADAR_SPRITE_SPRAY, 2 }, // spray3
            { 1282.1f, -104.8f, RADAR_SPRITE_BOMB, 0 }, // bomb1
            { 380.0f, -576.6f, RADAR_SPRITE_BOMB, 1 }, // bomb2
            { -1082.5f, 55.2f, RADAR_SPRITE_BOMB, 2 }, // bomb3
        };

        for (auto& it : pos) {
#if defined(GTA3) && defined(LCSFICATION)
            if ((it.island == 1 && !CStats::IndustrialPassed) || (it.island == 2 && !CStats::CommercialPassed))
                continue;
#endif

            CVector2D pos = WorldToMap({ it.x, it.y, 0.0f });
            DrawSpriteWithRotation(RadarSpritesArray[it.sprite], pos.x + GetMenuOffsetX(), pos.y, ScaleX(RADAR_BLIPS_SCALE), ScaleY(RADAR_BLIPS_SCALE), 0.0f, CRGBA(255, 255, 255,
#ifdef GTASA
                255
#else
                GetAlpha(255)
#endif
            ));

#if defined(GTA3) && defined(LCSFICATION)
            AddBlipToToLegendList(it.sprite);
#endif
        }
    }
#endif

    // Draw waypoint separately
    if (targetBlipIndex) {
        CVector2D pos = WorldToMap(targetBlipWorldPos);
        DrawWayPoint(pos.x + GetMenuOffsetX(), pos.y, ScaleX(RADAR_BLIPS_SCALE), ScaleY(RADAR_BLIPS_SCALE), CRGBA(255, 0, 0, GetAlpha(255)));

#if defined(GTA3) && defined(LCSFICATION)
        AddBlipToToLegendList(RADAR_WAYPOINT);
#endif
    }

    CPed* playa = FindPlayerPed();
    if (playa) {
        // Draw player
        CVector2D pos = WorldToMap(playa->GetPosition());
        float angle = FindPlayerHeading(
#ifdef GTASA 
            0
#endif
        );
        CSprite2d* sprite =
#ifdef GTASA
            & CRadar::RadarBlipSprites[RADAR_SPRITE_CENTRE];
#else 
            RadarSpritesArray[RADAR_SPRITE_CENTRE];
#endif

        if (sprite && flashItem(1000, 200))
            DrawSpriteWithRotation(sprite, pos.x + GetMenuOffsetX(), pos.y, ScaleX(RADAR_BLIPS_SCALE - 1.0f), ScaleY(RADAR_BLIPS_SCALE - 1.0f), angle, CRGBA(255, 255, 255,
#ifdef GTASA
                255
#else
                GetAlpha(255)
#endif
            ));

#if defined(GTA3) && defined(LCSFICATION)
        AddBlipToToLegendList(RADAR_SPRITE_CENTRE);
#endif
    }
}

CVector2D CMenuNew::WorldToMap(CVector in) {
    CVector2D out = { 0.0f, 0.0f };

    float x = (in.x + (MAP_SIZE / 2)) / MAP_SIZE;
    float y = ((MAP_SIZE / 2) - in.y) / MAP_SIZE;
    x *= GetMenuMapWholeSize();
    y *= GetMenuMapWholeSize();

    x += m_vMapBase.x - GetMenuMapHalfSize();
    y += m_vMapBase.y - GetMenuMapHalfSize();

    out.x = x;
    out.y = y;
    return out;
}

CVector CMenuNew::MapToWorld(CVector2D in) {
    CVector out = { 0.0f, 0.0f, 0.0f };

    in.x = (in.x + GetMenuMapHalfSize()) - m_vMapBase.x;
    in.y = m_vMapBase.y - (in.y + GetMenuMapHalfSize());

    in.x /= GetMenuMapWholeSize();
    in.y /= GetMenuMapWholeSize();

    float w = (MAP_SIZE / 2) / MAP_SIZE;
    out.x = (in.x - w) * MAP_SIZE;
    out.y = (w + in.y) * MAP_SIZE;
    out.z = 0.0f;
    return out;
}

void CMenuNew::StreamRadarSections() {
    //if (CStreaming::ms_disableStreaming)
    //    return;

    if (m_bRadarStreamed)
        return;

    for (int i = 0; i < RADAR_NUM_TILES; ++i) {
        for (int j = 0; j < RADAR_NUM_TILES; ++j) {
            int index = i + RADAR_NUM_TILES * j;
#ifdef GTASA
            int r = gRadarTextures[index];
#else
            int r = gRadarTxdIds[index];
#endif
#ifdef GTA3
            CStreaming::RequestModel(r + 5500, KEEP_IN_MEMORY | GAME_REQUIRED);
#elif GTAVC
            CStreaming::RequestModel(r + 6500, KEEP_IN_MEMORY | GAME_REQUIRED);
#elif GTASA
            CStreaming::RequestModel(r + 20000, KEEP_IN_MEMORY | GAME_REQUIRED);
            //CStreaming::LoadRequestedModels();
#endif    
        };
    }

#ifndef GTASA
    CStreaming::LoadAllRequestedModels(true);
#endif

    m_bRadarStreamed = true;
}

#ifdef WITH_ANIMATION
void CMenuNew::MoveMapToPosition(CVector2D target, CVector2D screenTarget, float t) {    
    float deltaX = screenTarget.x - target.x;
    float deltaY = screenTarget.y - target.y;
    
    m_vMapBase.x += (deltaX * t);
    m_vMapBase.y += (deltaY * t);
}

void CMenuNew::ProcessAnimStates() {
    CPlayerPed* playa = FindPlayerPed();
    CVector pos = playa->GetPosition();

    float deltaTime = (CTimer::m_snTimeInMillisecondsPauseMode - previousTimeInMilliseconds) / 1000.0f;
    float value = deltaTime * 3.0f;
    const float halfMapSize = GetMenuMapHalfSize();

    switch (m_nAnimState) {
        case ANIM_NONE:
            m_vPrevMapBase = m_vMapBase;
            m_nTimePassed = 2000 + CTimer::m_snTimeInMillisecondsPauseMode;
            m_nAnimState++;
            break;
        case ANIM_FRAME:
            m_vCrosshair.x = (SCREEN_WIDTH / 2);
            m_vCrosshair.y = (SCREEN_HEIGHT / 2);

            m_fMapZoom = (MAP_ZOOM_MAX * value) + ((1.0f - value) * m_fMapZoom);
            MoveMapToPosition(WorldToMap(pos), m_vCrosshair, value);

            if (m_nTimePassed < CTimer::m_snTimeInMillisecondsPauseMode) {
                m_nAnimState++;
            }
            break;
        case ANIM_DONE:
            break;
    }

    previousTimeInMilliseconds = CTimer::m_snTimeInMillisecondsPauseMode;
}
#endif

void CMenuNew::MapInput() {
    if (!menuManager)
        return;

    if (!GetInputEnabled()) {
        previousTimeInMilliseconds = CTimer::m_snTimeInMillisecondsPauseMode;
        return;
    }

#ifdef WITH_ANIMATION
    if (m_nAnimState != ANIM_DONE) {
        ProcessAnimStates();
        return;
    }
#endif

#ifdef GTASA
    menuManager->m_bStandardInput = true;
#endif

    CPad* pad = CPad::GetPad(0);

    if (clearInput) {
        pad->Clear(0
#ifdef GTASA
            , false
#endif
        );
        clearInput = false;
    }

    const float halfMapSize = GetMenuMapHalfSize();
    bool leftMousebutton = pad->NewMouseControllerState.lmb;
    bool rightMousebutton = pad->NewMouseControllerState.rmb && !pad->OldMouseControllerState.rmb;
    int32_t mapZoomOut = pad->NewMouseControllerState.wheelDown && !pad->OldMouseControllerState.wheelDown;
    int32_t mapZoomIn = pad->NewMouseControllerState.wheelUp && !pad->OldMouseControllerState.wheelUp;
    bool showLegend = pad->NewKeyState.standardKeys['L'] && !pad->OldKeyState.standardKeys['L'];

    mapZoomOut |= pad->NewKeyState.pgdn;
    mapZoomIn |= pad->NewKeyState.pgup;

    mapZoomOut |= pad->NewState.LeftShoulder2;
    mapZoomIn |= pad->NewState.RightShoulder2;

    mapZoomOut = std::clamp(mapZoomOut, -1, 1);
    mapZoomIn = std::clamp(mapZoomIn, -1, 1);

    rightMousebutton |= (pad->NewKeyState.standardKeys['T'] && !pad->OldKeyState.standardKeys['T']);
    rightMousebutton |= (pad->NewState.ButtonSquare && !pad->OldState.ButtonSquare);
    showLegend |= (pad->NewState.LeftShoulder1 && !pad->OldState.LeftShoulder1);

#ifdef GTAVC
    if (showLegend)
        menuManager->m_bPrefsShowLegends = menuManager->m_bPrefsShowLegends == false;
#elif defined(GTA3) && defined(LCSFICATION)
    if (settings.enableLegendBox) {
        if (showLegend)
            m_bPrefsShowLegends = m_bPrefsShowLegends == false;
    }
#endif

#ifdef WITH_VCS_MAP_OPTIONS
    bool showMapOptions = pad->NewKeyState.standardKeys['C'] || pad->NewState.ButtonCross;

    if (showMapOptions)
        m_nMapOptionsKeyPressTime += CTimer::m_snTimeInMillisecondsPauseMode - previousTimeInMilliseconds;
    else {
        m_bPrefsShowMapOptions = false;
        m_nMapOptionsKeyPressTime = 0;
    }

    if (m_nMapOptionsKeyPressTime > 500)
        m_bPrefsShowMapOptions = true;

    if (m_bPrefsShowMapOptions && m_bPrefsShowLegends) {
        m_nMapOptionsKeyPressTime = 0;
        bool up = pad->NewState.DPadUp && !pad->OldState.DPadUp;
        bool down = pad->NewState.DPadDown && !pad->OldState.DPadDown;
        bool left = pad->NewState.DPadLeft && !pad->OldState.DPadLeft;
        bool right = pad->NewState.DPadRight && !pad->OldState.DPadRight;

        left |= pad->NewState.LeftStickX < 0.0f && pad->OldState.LeftStickX >= 0.0f;
        right |= pad->NewState.LeftStickX > 0.0f && pad->OldState.LeftStickX <= 0.0f;

        up |= pad->NewState.LeftStickY < 0.0f && pad->OldState.LeftStickY >= 0.0f;
        down |= pad->NewState.LeftStickY > 0.0f && pad->NewState.LeftStickY >= 0.0f;

        // PC
        up |= pad->NewKeyState.up && !pad->OldKeyState.up;
        down |= pad->NewKeyState.down && !pad->OldKeyState.down;

        left |= pad->NewKeyState.left && !pad->OldKeyState.left;
        right |= pad->NewKeyState.right && !pad->OldKeyState.right;

        left |= pad->NewMouseControllerState.wheelDown && !pad->OldMouseControllerState.wheelDown;
        right |= pad->NewMouseControllerState.wheelUp && !pad->OldMouseControllerState.wheelUp;

        if (up)
            m_nCurrMapItem--;
        else if (down)
            m_nCurrMapItem++;

        if (m_nCurrMapItem > (int32_t)aMenuMapItems.size() - 1)
            m_nCurrMapItem = 0;

        if (m_nCurrMapItem < (int32_t)aMenuMapItems.size() - 1)
            m_nCurrMapItem = 0;

        for (auto& item : aMenuMapItems) {
            switch (item.item) {
                case MAP_OPTION_SHOW_DISCOVERED_EXTRA:
                    if (left)
                        m_nPrefsShowDiscoveredExtras--;
                    else if (right)
                        m_nPrefsShowDiscoveredExtras++;

                    if (m_nPrefsShowDiscoveredExtras > NUM_DISCOVERED_EXTRAS - 1)
                        m_nPrefsShowDiscoveredExtras = 0;

                    if (m_nPrefsShowDiscoveredExtras < 0)
                        m_nPrefsShowDiscoveredExtras = NUM_DISCOVERED_EXTRAS - 1;
                    break;
            }
        }

        previousTimeInMilliseconds = CTimer::m_snTimeInMillisecondsPauseMode;
        return;
    }
#endif

    if (menuManager->m_bShowMouse) {
        m_vCrosshair.x = static_cast<float>(menuManager->m_nMouseTempPosX);
        m_vCrosshair.y = static_cast<float>(menuManager->m_nMouseTempPosY);

        if (leftMousebutton) {
            m_vMapBase.x += static_cast<float>(menuManager->m_nMouseTempPosX - menuManager->m_nMouseOldPosX);
            m_vMapBase.y += static_cast<float>(menuManager->m_nMouseTempPosY - menuManager->m_nMouseOldPosY);
        }
    }
    const int mult = (CTimer::m_snTimeInMillisecondsPauseMode - previousTimeInMilliseconds) * 0.5f;
    float right = pad->NewKeyState.left ? -1.0f : pad->NewKeyState.right ? 1.0f : 0.0f;
    float up = pad->NewKeyState.up ? -1.0f : pad->NewKeyState.down ? 1.0f : 0.0f;

    right += pad->NewState.DPadLeft ? -1.0f : pad->NewState.DPadRight ? 1.0f : 0.0f;
    up += pad->NewState.DPadUp ? -1.0f : pad->NewState.DPadDown ? 1.0f : 0.0f;

    right += pad->NewState.LeftStickX / 128.0f;
    up += pad->NewState.LeftStickY / 128.0f;

    right = std::clamp(right, -1.0f, 1.0f);
    up = std::clamp(up, -1.0f, 1.0f);

    auto MoveMap = [&]() {
        m_vMapBase.x = m_vMapBase.x + (-right) * mult;
        m_vMapBase.y = m_vMapBase.y + (-up) * mult;

        float wholeMapSize = GetMenuMapWholeSize();

        float minX = SCREEN_WIDTH / 2.0f - wholeMapSize / 2.0f;
        float maxX = SCREEN_WIDTH / 2.0f + wholeMapSize / 2.0f;
        float minY = (SCREEN_HEIGHT / 2.0f) - wholeMapSize / 2.0f;
        float maxY = wholeMapSize / 2.0f;
       
        m_vMapBase.x = std::clamp(m_vMapBase.x, minX, maxX);
        m_vMapBase.y = std::clamp(m_vMapBase.y, minY, maxY);
    };



    auto MoveCrosshair = [&]() {

    };

    auto ZoomMap = [&](bool mapZoomIn, bool mapZoomOut) {
        float oldZoom = m_fMapZoom;

        if (mapZoomIn && m_fMapZoom < MAP_ZOOM_MAX) {
            m_fMapZoom += 1.0f * mult;
        }
        else if (mapZoomOut && m_fMapZoom > MAP_ZOOM_MIN) {
            m_fMapZoom -= 1.0f * mult;
        }

        float scale = m_fMapZoom / oldZoom;

        float deltaX = m_vCrosshair.x - m_vMapBase.x;
        float deltaY = m_vCrosshair.y - m_vMapBase.y;

        m_vMapBase.x += deltaX * (1.0f - scale);
        m_vMapBase.y += deltaY * (1.0f - scale);
    };

    ZoomMap(mapZoomIn, mapZoomOut);
    m_fMapZoom = std::clamp(m_fMapZoom, MAP_ZOOM_MIN, MAP_ZOOM_MAX);

    MoveMap();
    MoveCrosshair();

    if (rightMousebutton)
        SetWaypoint(m_vCrosshair.x, m_vCrosshair.y);

    previousTimeInMilliseconds = CTimer::m_snTimeInMillisecondsPauseMode;
}


void CMenuNew::ResetMap(bool resetCrosshair) {
    m_fMapZoom = MAP_ZOOM_MIN;

#ifdef GTA3
    m_vMapBase = { SCREEN_WIDTH / 2, GetMenuMapHalfSize()};
#else
    m_vMapBase = { (SCREEN_WIDTH / 2) + GetMenuMapHalfSize(), SCREEN_HEIGHT / 2};
#endif

    if (GetLcsfication()) {
#if defined(GTA3) && defined(LCSFICATION)
       //for (int i = 0; i < 4; i++)
       //    DoMapZoomInOut(false);

        m_vMapBase.x -= ScaleY(10.0f);
        m_vMapBase.y -= ScaleY(86.0f);
#endif
    }

    if (resetCrosshair) {
        m_vCrosshair.x = SCREEN_WIDTH / 2;
        m_vCrosshair.y = SCREEN_HEIGHT / 2;
    }

#ifdef WITH_ANIMATION
    m_nAnimState = ANIM_NONE;
    m_nTimePassed = CTimer::m_snTimeInMillisecondsPauseMode;
#endif

#ifdef WITH_VCS_MAP_OPTIONS
    m_nMapOptionsKeyPressTime = 0;
#endif

    m_bRadarStreamed = false;
}

void CMenuNew::DrawRadarSectionMap(int x, int y, CRect const& rect, CRGBA const& col) {
    RwRenderStateSet(rwRENDERSTATETEXTUREFILTER, (void*)rwFILTERMIPLINEAR);
    RwRenderStateSet(rwRENDERSTATESRCBLEND, (void*)rwBLENDSRCALPHA);
    RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void*)rwBLENDINVSRCALPHA);
    RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)TRUE);
    RwRenderStateSet(rwRENDERSTATETEXTUREPERSPECTIVE, (void*)FALSE);
    RwRenderStateSet(rwRENDERSTATETEXTUREADDRESS, (void*)rwTEXTUREADDRESSCLAMP);
    
    int index = std::clamp((int32_t)(x + RADAR_NUM_TILES * y), 0, (int32_t)(RADAR_NUM_TILES * RADAR_NUM_TILES) - 1);

    if (dontStreamRadarTiles) {
        auto texture = spriteLoader.GetTex("map");
        RwRenderStateSet(rwRENDERSTATETEXTURERASTER, RwTextureGetRaster(texture));

        float u1 = (float)(x) / (float)RADAR_NUM_TILES;
        float v1 = (float)(y) / (float)RADAR_NUM_TILES;
        float u2 = (float)(x + 1) / (float)RADAR_NUM_TILES;
        float v2 = (float)(y + 1) / (float)RADAR_NUM_TILES;

        CSprite2d::SetVertices(rect, col, col, col, col, u1, v1, u2, v1, u1, v2, u2, v2);
        RwIm2DRenderPrimitive(rwPRIMTYPETRIFAN, CSprite2d::maVertices, 4);
    }
    else {
        RwTexture* texture = NULL;
#ifdef GTASA
        int r = gRadarTextures[index];
#else
        int r = gRadarTxdIds[index];
#endif
        RwTexDictionary* txd = CTxdStore::ms_pTxdPool->GetAt(r)->m_pRwDictionary;

        if (txd)
            texture = GetFirstTexture(txd);

        if (texture) {
            RwRenderStateSet(rwRENDERSTATETEXTURERASTER, RwTextureGetRaster(texture));
            CSprite2d::SetVertices(rect, col, col, col, col
#ifdef GTA3
                                   , 0
#endif
            );
            RwIm2DRenderPrimitive(rwPRIMTYPETRIFAN, CSprite2d::maVertices, 4);
        }
    }
}

void CMenuNew::SetWaypoint(float x, float y) {
    if (targetBlipIndex) {
        targetBlipIndex = 0;
    }
    else {
        CVector pos = MapToWorld(CVector2D(x, y));
        pos.x = std::clamp(pos.x, -MAP_SIZE / 2, MAP_SIZE / 2);
        pos.y = std::clamp(pos.y, -MAP_SIZE / 2, MAP_SIZE / 2);

        targetBlipWorldPos = pos;
        targetBlipIndex = 1;
    }

#ifdef GTA3
    DMAudio.PlayFrontEndSound(152, 0);
#elif GTAVC
    DMAudio.PlayFrontEndSound(196, 0);
#endif
}

float CMenuNew::GetMenuMapTileSize() {
    const float tileSize = ScaleY(24.0f);
    return tileSize;
}

float CMenuNew::GetMenuMapWholeSize() {
    return RADAR_NUM_TILES * m_fMapZoom;
}

float CMenuNew::GetMenuMapHalfSize() {
    return (RADAR_NUM_TILES / 2) * m_fMapZoom;
}

void CMenuNew::DrawLegend() {
#if defined(GTAVC) || (defined(GTA3) && defined(LCSFICATION))
#ifdef GTAVC
    if (!menuManager->m_bPrefsShowLegends)
        return;
#elif GTA3
    if (!m_bPrefsShowLegends)
        return;
#endif

    const float boxHalfWidth = 304.0f; // LCS:296.0f;
    const float boxHorFromCenter = 162.0f;

    const float spacing = 28.0f;

    float offset = 0.0f;
#ifdef WITH_VCS_MAP_OPTIONS
    offset = ScaleY(spacing * 2);
#endif

#ifdef GTA3
    float boxHeight = (MapLegendCounter - (1 * MapLegendCounter / 2));
#else
    float boxHeight = (CRadar::MapLegendCounter - (1 * CRadar::MapLegendCounter / 2));
#endif

#ifdef WITH_VCS_MAP_OPTIONS
    if (m_bPrefsShowMapOptions) {
        boxHeight = aMenuMapItems.size();
        offset = 0.0f;
    }
#endif

    const float centerX = SCREEN_WIDTH / 2;
    const float centerY = SCREEN_HEIGHT / 2;

    const float startBoxX = centerX - ScaleX(boxHalfWidth);
    const float legendEntryStartX = startBoxX + ScaleX(36.0f);
    const float legendEntryStartX2 = centerX;
    const float endBoxX = startBoxX + ScaleX(boxHalfWidth * 2);

#define OFFSET_DOWN_SCALE (fScaleMult < 1.0f ? 76.0f * (1.0f + (1.0f - fScaleMult)) : 76.0f)

    const float startBoxY = ScaleY(OFFSET_DOWN_SCALE);
    const float legendEntryStartY = startBoxY + ScaleY(34.0f);
    const float endBoxY = legendEntryStartY + ScaleY(spacing * boxHeight) + offset;

    CSprite2d::DrawRect(CRect(startBoxX, startBoxY, endBoxX, endBoxY), CRGBA(25, 25, 25, GetAlpha(195)));

#ifdef WITH_VCS_MAP_OPTIONS
    if (m_bPrefsShowMapOptions) {
        float x = 0.0f;
        float y = legendEntryStartY - ScaleY(spacing * 0.5f);
        int32_t i = 0;
        for (auto& it : aMenuMapItems) {
            x = legendEntryStartX;
            CRGBA col = CRGBA(HUD_COLOUR_LCS_MENU, 225);

            if (i = m_nCurrMapItem)
                col = CRGBA(255, 255, 255, 225);

            DrawTextForLegendBox(x, y, TheText.Get(it.str.c_str()), col);

            std::wstring rightStr = {};
            switch (it.item) {
                case MAP_OPTION_SHOW_DISCOVERED_EXTRA:
                    switch (m_nPrefsShowDiscoveredExtras) {
                        case EXTRA_NONE:
                            rightStr = TheText.Get("FEM_OFF");
                            break;
                        case EXTRA_HIDDEN_PACKAGE:
                            rightStr = TheText.Get("FE_DIXB");
                            break;
                        case EXTRA_RAMPAGE:
                            rightStr = TheText.Get("FE_DIXC");
                            break;
                        case EXTRA_UNIQUE_STUNT:
                            rightStr = TheText.Get("FE_DIXD");
                            break;
                    }
                    break;
            }

            if (!rightStr.empty()) {
                x = legendEntryStartX2;
                DrawTextForLegendBox(x, y, rightStr.c_str(), col);
            }

            y += ScaleY(spacing);
            i++;
        }
        return;
    }
#endif

#ifdef GTAVC
    for (int i = 0; i < CRadar::MapLegendCounter; i += 2) {
        x = SCREEN_WIDTH / 2;
        x -= ScaleX(shift);
        CRadar::DrawLegend(x, y, CRadar::MapLegendList[i]);

        if (CRadar::MapLegendList[i + 1] != RADAR_SPRITE_NONE && i < 74) {
            x = SCREEN_WIDTH / 2;
            x += ScaleX(shift / 8);
            CRadar::DrawLegend(x, y, CRadar::MapLegendList[i + 1]);
        }

        y += ScaleY(spacing);
    }
#elif defined(GTA3) && defined(LCSFICATION)
    float x = 0.0f;
    float y = legendEntryStartY;

    for (int i = 0; i < MapLegendCounter; i += 2) {
        CRGBA col = MapLegendBlipColor[i];
        col.a = GetAlpha(col.a);
        x = legendEntryStartX;
        DrawLegendEntry(x + GetMenuOffsetX(), y, MapLegendList[i], &col);

        if (MapLegendList[i + 1] != RADAR_SPRITE_NONE) {
            x = legendEntryStartX2;
            DrawLegendEntry(x + GetMenuOffsetX(), y, MapLegendList[i + 1], &col);
        }

        y += ScaleY(spacing);
    }

#ifdef WITH_VCS_MAP_OPTIONS
    x = legendEntryStartX;
    y = endBoxY - ScaleY(spacing * 1.5f);
    DrawTextForLegendBox(x + GetMenuOffsetX(), y, TheText.Get("FE_HLPD"), CRGBA(HUD_COLOUR_LCS_MENU, GetAlpha(255)));
#endif
#endif
#endif
}

#if defined(GTA3) && defined(LCSFICATION)
template <typename T>
void CMenuNew::DrawTextForLegendBox(float x, float y, const T* str, CRGBA const& col) {
    CFont::SetPropOn();
    CFont::SetWrapx(SCREEN_WIDTH * 2);
    CFont::SetBackGroundOnlyTextOff();
    CFont::SetCentreOff();
    CFont::SetRightJustifyOff();

    CFont::SetDropShadowPosition(1);
    CFont::SetDropColor(CRGBA(0, 0, 0, GetAlpha(255)));
    CFont::SetFontStyle(0);
    CFont::SetColor(col);
    CFont::SetScale(ScaleX(0.5f), ScaleY(1.2f));

    CFont::PrintString(x, y, str);
}

void CMenuNew::DrawLegendEntry(float x, float y, short id, CRGBA* col) {
    const float blipX = x + ScaleX(LEGEND_BLIP_SCALE_X * 0.5f);
    const float blipY = y;

    if (id < 0) {
        switch (id) {
            case RADAR_OBJECTIVE:
            case RADAR_DESTINATION: {
                static int level = 0;
                static int levelTime = 0;
                CRGBA white = { 255, 255, 255, 255 };

                if (!col)
                    col = &white;

                if (levelTime < CTimer::m_snTimeInMillisecondsPauseMode) {
                    levelTime = CTimer::m_snTimeInMillisecondsPauseMode + 1000;
                    level++;

                    if (level > 2)
                        level = 0;
                }

                DrawLevel(blipX, blipY, ScaleX(LEGEND_BLIP_SCALE_X * 0.5f), ScaleY(LEGEND_BLIP_SCALE_Y * 0.5f), level, CRGBA(col->r, col->g, col->b, GetAlpha(255)));
            }
            break;
            case RADAR_WAYPOINT:
                DrawWayPoint(blipX, blipY, ScaleX(LEGEND_BLIP_SCALE_X), ScaleY(LEGEND_BLIP_SCALE_Y), CRGBA(255, 0, 0, GetAlpha(255)));
                break;
#ifdef WITH_VCS_MAP_OPTIONS
            case RADAR_PACKAGE:
                DrawSpriteWithRotation(&RadarPackageSprite, blipX, blipY, ScaleX(LEGEND_BLIP_SCALE_X), ScaleY(LEGEND_BLIP_SCALE_Y), 0.0f, CRGBA(255, 255, 255, GetAlpha(255)));
                break;
            case RADAR_RAMPAGE:
                DrawSpriteWithRotation(&RadarRampageSprite, blipX, blipY, ScaleX(LEGEND_BLIP_SCALE_X), ScaleY(LEGEND_BLIP_SCALE_Y), 0.0f, CRGBA(255, 255, 255, GetAlpha(255)));
                break;
            case RADAR_UNIQUE_STUNT:
                DrawSpriteWithRotation(&RadarStuntJumpSprite, blipX, blipY, ScaleX(LEGEND_BLIP_SCALE_X), ScaleY(LEGEND_BLIP_SCALE_Y), 0.0f, CRGBA(255, 255, 255, GetAlpha(255)));
                break;
#endif
        }
    }
    else {
        DrawSpriteWithRotation(RadarSpritesArray[id], blipX, blipY, ScaleX(LEGEND_BLIP_SCALE_X), ScaleY(LEGEND_BLIP_SCALE_Y), 0.0f, CRGBA(255, 255, 255, GetAlpha(255)));
    }

    if (id < 0)
        id = std::abs(id) + RADAR_SHIFTED_NEGATIVE;

    CRGBA c = CRGBA(HUD_COLOUR_LCS_MENU, GetAlpha(255));

    x = blipX + ScaleX(LEGEND_BLIP_SCALE_X * 0.8f);
    y = blipY - ScaleY(LEGEND_BLIP_SCALE_Y * 0.5f);
    if (settings.readStringsFromThisFile) {
        DrawTextForLegendBox(x, y, settings.gxt.at(id).c_str(), c);  
    }
    else {
        char buff[16];
        sprintf(buff, "LG_%02d", id);
        DrawTextForLegendBox(x, y, TheText.Get(buff), c);
    }
}

void CMenuNew::AddBlipToToLegendList(short id, CRGBA const& col) {
    bool found = false;

    for (int i = 0; i < MAX_LEGEND_ENTRIES; i++) {
        if (MapLegendList[i] == id) {
            found = true;
        }
    }

    if (!found) {
        MapLegendBlipColor[MapLegendCounter] = col;
        MapLegendList[MapLegendCounter] = id;
        MapLegendCounter++;
    }
}

#endif

#ifdef WITH_VCS_MAP_OPTIONS
void CMenuNew::DrawDiscoveredExtrasBlips() {
    if (m_nPrefsShowDiscoveredExtras == EXTRA_HIDDEN_PACKAGE || Debug_ShowAllHiddenPackages) {
        for (auto& it : aPackages) {
            int32_t a = 255;
            if (!it.removed)
                if (!Debug_ShowAllHiddenPackages)
                    continue;
                else
                    a = 75;

            CVector2D pos = WorldToMap(it.pos);
            DrawSpriteWithRotation(&RadarPackageSprite, pos.x + GetMenuOffsetX(), pos.y, ScaleX(RADAR_BLIPS_SCALE), ScaleY(RADAR_BLIPS_SCALE), 0.0f, CRGBA(255, 255, 255, GetAlpha(a)));
            AddBlipToToLegendList(RADAR_PACKAGE);
        }
    }

    if (m_nPrefsShowDiscoveredExtras == EXTRA_RAMPAGE || Debug_ShowAllRampages) {
        for (auto& it : aRampages) {
            int32_t a = 255;
            if (!it.removed)
                if (!Debug_ShowAllRampages)
                    continue;
                else
                    a = 75;

            CVector2D pos = WorldToMap(it.pos);
            DrawSpriteWithRotation(&RadarRampageSprite, pos.x + GetMenuOffsetX(), pos.y, ScaleX(RADAR_BLIPS_SCALE), ScaleY(RADAR_BLIPS_SCALE), 0.0f, CRGBA(255, 255, 255, GetAlpha(a)));
            AddBlipToToLegendList(RADAR_RAMPAGE);
        }
    }

    if (m_nPrefsShowDiscoveredExtras == EXTRA_UNIQUE_STUNT || Debug_ShowAllUniqueStunts) {
        for (auto& it : aUniqueStunts) {
            int32_t a = 255;
            if (!it.removed)
                if (!Debug_ShowAllUniqueStunts)
                    continue;
                else
                    a = 75;

            CVector2D pos = WorldToMap(it.pos);
            DrawSpriteWithRotation(&RadarStuntJumpSprite, pos.x + GetMenuOffsetX(), pos.y, ScaleX(RADAR_BLIPS_SCALE), ScaleY(RADAR_BLIPS_SCALE), 0.0f, CRGBA(255, 255, 255, GetAlpha(a)));
            AddBlipToToLegendList(RADAR_UNIQUE_STUNT);
        }
    }
}
#endif

