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

static short MapLegendList[32] = {};
static short MapLegendCounter = 0;
CRGBA MapLegendBlipColor[32] = {};
#endif

#include "SkyUIAPI.h"

using namespace plugin;

std::unique_ptr<CMenuNew> MenuNew;
CSprite2d** RadarSpritesArray = pRadarSprites;

D3DVIEWPORT8 previousViewport = {};
D3DVIEWPORT8 newViewport = {};

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
}

CMenuNew::~CMenuNew() {
    menuManager = nullptr;
    
#ifdef LCSFICATION
    RadarSpritesArray = patch::Get<CSprite2d**>(0x4A6004 + 3);
#endif
}

uint32_t CMenuNew::GetAlpha(uint32_t a) {
    if (settings.skyUI)
        return min(menuManager->FadeIn(a), SkyUI::GetAlpha(a));

    return menuManager->FadeIn(a);
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
    if (settings.skyUI) {
        GetD3DDevice<IDirect3DDevice8>()->GetViewport(&previousViewport);

        newViewport.X = ScaleXKeepCentered(32.0f) + GetMenuOffsetX();
        newViewport.Y = ScaleY(42.0f);
        newViewport.Width = ScaleXKeepCentered(DEFAULT_SCREEN_WIDTH - 32.0f) - newViewport.X + GetMenuOffsetX();
        newViewport.Height = ScaleY(DEFAULT_SCREEN_HEIGHT - 102.0f) - newViewport.Y;
        GetD3DDevice<IDirect3DDevice8>()->SetViewport(&newViewport);
    }
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

    CSprite2d::DrawRect(CRect(-5.0f+ GetMenuOffsetX(), -5.0f, SCREEN_WIDTH + 5.0f + GetMenuOffsetX(), SCREEN_HEIGHT + 5.0f), CRGBA(0, 0, 0, GetAlpha()));
    CSprite2d::DrawRect(CRect(-5.0f+ GetMenuOffsetX(), -5.0f, SCREEN_WIDTH + 5.0f + GetMenuOffsetX(), SCREEN_HEIGHT + 5.0f), CRGBA(settings.backgroundColor.r, settings.backgroundColor.g, settings.backgroundColor.b, GetAlpha(settings.backgroundColor.a)));

    CRGBA col = { settings.radarMapColor.r, settings.radarMapColor.g, settings.radarMapColor.b,
#ifdef GTASA
        255
#else
        static_cast<unsigned char>(GetAlpha(settings.radarMapColor.a))
#endif
    };
    CRect rect;

    const float mapHalfSize = GetMenuMapWholeSize() / 2;
    float mapZoom = GetMenuMapTileSize() * m_fMapZoom;
    rect.left = m_vMapBase.x + GetMenuOffsetX();
    rect.top = m_vMapBase.y;
    rect.right = rect.left + mapZoom;
    rect.bottom = rect.top + mapZoom;

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

    DrawBlips();
    DrawCrosshair(m_vCrosshair.x, m_vCrosshair.y);
    DrawZone();

#ifdef GTAVC
    DrawLegend();
    menuManager->m_bDrawRadarOrMap = false;
    menuManager->DisplayHelperText("FEH_MPH");
#endif

#ifdef LCSFICATION
    if (settings.enableLegendBox)
        DrawLegend();

    MapLegendCounter = 0;
    memset(MapLegendList, 0, sizeof(MapLegendList));

#endif

#ifdef GTA3
    if (settings.skyUI)
        GetD3DDevice<IDirect3DDevice8>()->SetViewport(&previousViewport);
#endif
}

void CMenuNew::DrawCrosshair(float x, float y) {
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
    if (settings.skyUI)
        CSprite2d::DrawRect(CRect(-5.0f + GetMenuOffsetX(), SCREEN_HEIGHT + 5.0f, SCREEN_WIDTH + 5.0f + GetMenuOffsetX(), ScaleY(DEFAULT_SCREEN_HEIGHT - 128.0f)), CRGBA(10, 10, 10, GetAlpha(255)));
    else
        CSprite2d::DrawRect(CRect(-5.0f, SCREEN_HEIGHT + 5.0f, SCREEN_WIDTH + 5.0f, SCREEN_HEIGHT - ScaleY(42.0f)), CRGBA(10, 10, 10, GetAlpha(255)));

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
    if (zoneType0)
        str = zoneType0->GetTranslatedName();

#ifdef GTA3
    if (zoneType1)
        str = zoneType1->GetTranslatedName();
#endif
    if (str) {
#ifndef GTASA
        CFont::SetPropOn();
        CFont::SetBackgroundOff();
        CFont::SetCentreOff();
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
        if (settings.skyUI)
            CFont::PrintString(ScaleXKeepCentered(52.0f) + GetMenuOffsetX(), ScaleY(DEFAULT_SCREEN_HEIGHT - 126.0f), str);
        else
            CFont::PrintString(ScaleX(16.0f), SCREEN_HEIGHT - ScaleY(34.0f), str);
#else
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
}

void CMenuNew::DrawBlips() {
#ifdef GTA3
    for (int i = 0; i < 32; i++) {
#else
    for (int i = 0; i < 75; i++) {
#endif
        tRadarTrace& trace = CRadar::ms_RadarTrace[i];

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
        CSprite2d* sprite =
#ifdef GTASA
            & CRadar::RadarBlipSprites[id];
#else
            RadarSpritesArray[id];
#endif
        CEntity* e = NULL;

        if (trace.m_nBlipDisplay < BLIP_DISPLAY_BLIP_ONLY)
            continue;

        switch (trace.m_nBlipType) {
        case BLIP_COORD:
        case BLIP_CONTACTPOINT:
            if (!CTheScripts::IsPlayerOnAMission() || trace.m_nBlipType == BLIP_COORD) {
                if (id > RADAR_SPRITE_NONE && id < RADAR_SPRITE_COUNT) {
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

                if (id > RADAR_SPRITE_NONE && id < RADAR_SPRITE_COUNT) {
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

    x += m_vMapBase.x - GetMenuMapWholeSize() / 2;
    y += m_vMapBase.y - GetMenuMapWholeSize() / 2;

    out.x = x;
    out.y = y;
    return out;
}

CVector CMenuNew::MapToWorld(CVector2D in) {
    CVector out = { 0.0f, 0.0f, 0.0f };

    in.x = (in.x + GetMenuMapWholeSize() / 2) - m_vMapBase.x;
    in.y = m_vMapBase.y - (in.y + GetMenuMapWholeSize() / 2);

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
            CStreaming::LoadRequestedModels();
#endif
        };
    }

#ifndef GTASA
    CStreaming::LoadAllRequestedModels(true);
#endif
}

void CMenuNew::MapInput() {
    if (!menuManager)
        return;

    if (!GetInputEnabled())
        return;

#ifdef GTASA
    menuManager->m_bMapOverview = true;
#endif

    CPad* pad = CPad::GetPad(0);

    if (clearInput) {
        pad->Clear(true
#ifdef GTASA
            , false
#endif
        );
        clearInput = false;
    }

    const float halfMapSize = GetMenuMapWholeSize() / 2;
    bool leftMousebutton = pad->NewMouseControllerState.lmb;
    bool rightMousebutton = pad->NewMouseControllerState.rmb && !pad->OldMouseControllerState.rmb;
    short mapZoomOut = pad->NewMouseControllerState.wheelDown && !pad->OldMouseControllerState.wheelDown;
    short mapZoomIn = pad->NewMouseControllerState.wheelUp && !pad->OldMouseControllerState.wheelUp;
    bool showLegend = pad->NewKeyState.standardKeys['L'] && !pad->OldKeyState.standardKeys['L'];

    mapZoomOut |= pad->NewKeyState.pgdn;
    mapZoomIn |= pad->NewKeyState.pgup;

    mapZoomOut |= pad->NewState.LeftShoulder2;
    mapZoomIn |= pad->NewState.RightShoulder2;

    mapZoomOut = clamp(mapZoomOut, -1.0f, 1.0f);
    mapZoomIn = clamp(mapZoomIn, -1.0f, 1.0f);

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

    if (menuManager->m_bShowMouse) {
        m_vCrosshair.x = static_cast<float>(menuManager->m_nMouseTempPosX);
        m_vCrosshair.y = static_cast<float>(menuManager->m_nMouseTempPosY);

        if (leftMousebutton) {
            m_vMapBase.x += static_cast<float>(menuManager->m_nMouseTempPosX - menuManager->m_nMouseOldPosX);
            m_vMapBase.y += static_cast<float>(menuManager->m_nMouseTempPosY - menuManager->m_nMouseOldPosY);
        }
    }
    else {
        const int mult = (CTimer::m_snTimeInMillisecondsPauseMode - previousTimeInMilliseconds) * 0.5f;
        float right = pad->NewKeyState.left ? -1.0f : pad->NewKeyState.right ? 1.0f : 0.0f;
        float up = pad->NewKeyState.up ? -1.0f : pad->NewKeyState.down ? 1.0f : 0.0f;

        right += pad->NewState.DPadLeft ? -1.0f : pad->NewState.DPadRight ? 1.0f : 0.0f;
        up += (pad->NewState.DPadUp || pad->NewState.LeftStickY < 0.0f) ? -1.0f : (pad->NewState.DPadDown || pad->NewState.LeftStickY > 0.0f) ? 1.0f : 0.0f;

        right += pad->NewState.LeftStickX / 128.0f;
        up += pad->NewState.LeftStickY / 128.0f;

        right = clamp(right, -1.0f, 1.0f);
        up = clamp(up, -1.0f, 1.0f);

        m_vMapBase.x -= right * mult;
        m_vMapBase.y -= up * mult;

#ifdef GTA3
        if (m_vMapBase.x <= (SCREEN_WIDTH / 2) - halfMapSize || m_vMapBase.x >= (SCREEN_WIDTH / 2) + halfMapSize) {
            m_vCrosshair.x += right * mult;
        }
        else
#endif
            m_vCrosshair.x = SCREEN_WIDTH / 2;

#ifdef GTA3
        if (m_vMapBase.y <= (SCREEN_HEIGHT / 2) - halfMapSize || m_vMapBase.y >= halfMapSize) {
            m_vCrosshair.y += up * mult;
        }
        else
#endif
            m_vCrosshair.y = SCREEN_HEIGHT / 2;
    }

    if (mapZoomIn)
        DoMapZoomInOut(false);
    else if (mapZoomOut)
        DoMapZoomInOut(true);

    if (rightMousebutton)
        SetWaypoint(m_vCrosshair.x, m_vCrosshair.y);

    m_vCrosshair.x = clamp(m_vCrosshair.x, m_vMapBase.x - halfMapSize, m_vMapBase.x + halfMapSize);
    m_vCrosshair.y = clamp(m_vCrosshair.y, m_vMapBase.y - halfMapSize, m_vMapBase.y + halfMapSize);

    m_vMapBase.x = clamp(m_vMapBase.x, (SCREEN_WIDTH / 2) - halfMapSize, (SCREEN_WIDTH / 2) + halfMapSize);
#ifdef GTA3
    m_vMapBase.y = clamp(m_vMapBase.y, (SCREEN_HEIGHT / 2) - halfMapSize, halfMapSize);
#else
    m_vMapBase.y = clamp(m_vMapBase.y, (SCREEN_HEIGHT / 2) - halfMapSize, (SCREEN_HEIGHT / 2) + halfMapSize);
#endif

    previousTimeInMilliseconds = CTimer::m_snTimeInMillisecondsPauseMode;
}

void CMenuNew::DoMapZoomInOut(bool out) {
    const float value = (!out ? 1.1f : 1.0f / 1.1f);

    if (out && m_fMapZoom > MAP_ZOOM_MIN || !out && m_fMapZoom < MAP_ZOOM_MAX) {
        m_fMapZoom *= value;

        const float halfMapSize = GetMenuMapWholeSize() / 2;
        m_vMapBase.x += ((m_vCrosshair.x) - m_vMapBase.x) * (1.0f - value);
        m_vMapBase.y += ((m_vCrosshair.y) - m_vMapBase.y) * (1.0f - value);

        m_vMapBase.x = clamp(m_vMapBase.x, (SCREEN_WIDTH / 2) - halfMapSize, (SCREEN_WIDTH / 2) + halfMapSize);
#ifdef GTA3
        m_vMapBase.y = clamp(m_vMapBase.y, (SCREEN_HEIGHT / 2) - halfMapSize, halfMapSize);
#else
        m_vMapBase.y = clamp(m_vMapBase.y, (SCREEN_HEIGHT / 2) - halfMapSize, (SCREEN_HEIGHT / 2) + halfMapSize);
#endif
    }
}

void CMenuNew::ResetMap(bool resetCrosshair) {
    m_fMapZoom = MAP_ZOOM_MIN;
#ifdef GTA3
    m_vMapBase = { SCREEN_WIDTH / 2, GetMenuMapWholeSize() / 2 };
#else
    m_vMapBase = { (SCREEN_WIDTH / 2) + GetMenuMapWholeSize() / 8, SCREEN_HEIGHT / 2 };
#endif

    if (resetCrosshair) {
        m_vCrosshair.x = SCREEN_WIDTH / 2;
        m_vCrosshair.y = SCREEN_HEIGHT / 2;
    }
}

void CMenuNew::DrawRadarSectionMap(int x, int y, CRect const& rect, CRGBA const& col) {
    int index = clamp(x + RADAR_NUM_TILES * y, 0, (RADAR_NUM_TILES * RADAR_NUM_TILES) - 1);
    RwTexture* texture = NULL;
#ifdef GTASA
    int r = gRadarTextures[index];
#else
    int r = gRadarTxdIds[index];
#endif
    RwTexDictionary* txd = CTxdStore::ms_pTxdPool->GetAt(r)->m_pRwDictionary;

    if (txd)
        texture = GetFirstTexture(txd);

    RwRenderStateSet(rwRENDERSTATETEXTUREFILTER, (void*)rwFILTERMIPLINEAR);
    RwRenderStateSet(rwRENDERSTATESRCBLEND, (void*)rwBLENDSRCALPHA);
    RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void*)rwBLENDINVSRCALPHA);
    RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)TRUE);
    RwRenderStateSet(rwRENDERSTATETEXTUREPERSPECTIVE, (void*)FALSE);
    RwRenderStateSet(rwRENDERSTATETEXTUREADDRESS, (void*)rwTEXTUREADDRESSCLAMP);

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

void CMenuNew::SetWaypoint(float x, float y) {
    if (targetBlipIndex) {
        targetBlipIndex = 0;
    }
    else {
        CVector pos = MapToWorld(CVector2D(x, y));
        pos.x = clamp(pos.x, -MAP_SIZE / 2, MAP_SIZE / 2);
        pos.y = clamp(pos.y, -MAP_SIZE / 2, MAP_SIZE / 2);

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
    const float mapWholeSize = GetMenuMapTileSize() * RADAR_NUM_TILES;
    return mapWholeSize * m_fMapZoom;
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

    const float shift = 182.0f;
    float x = SCREEN_WIDTH / 2;
    float y = ScaleY(128.0f);

    CFont::SetPropOn();
    CFont::SetWrapx(SCREEN_WIDTH * 2);
    CFont::SetBackGroundOnlyTextOff();
    CFont::SetCentreOff();
    CFont::SetRightJustifyOff();

    CFont::SetDropShadowPosition(1);
    CFont::SetDropColor(CRGBA(0, 0, 0, GetAlpha(255)));
    CFont::SetFontStyle(1);
    CFont::SetColor(CRGBA(225, 225, 225, GetAlpha(255)));
    CFont::SetScale(ScaleX(0.34f), ScaleY(0.48f));

#ifdef GTAVC
    CSprite2d::DrawRect(CRect(x - ScaleX(shift * 1.1f), y - ScaleY(8.0f), x + ScaleX(shift * 1.1f), y + ScaleY(8.0f) + ScaleY(17.0f * (CRadar::MapLegendCounter - (1 * CRadar::MapLegendCounter / 2)))), CRGBA(0, 0, 0, GetAlpha(155)));
#elif defined(GTA3) && defined(LCSFICATION)
    CSprite2d::DrawRect(CRect(x - ScaleX(shift * 1.1f) + GetMenuOffsetX(), y - ScaleY(16.0f), x + ScaleX(shift * 1.1f) + GetMenuOffsetX(), y + ScaleY(8.0f) + ScaleY(17.0f * (MapLegendCounter - (1 * MapLegendCounter / 2)))), CRGBA(0, 0, 0, GetAlpha(155)));
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

        y += ScaleY(18.0f);
    }
#elif defined(GTA3) && defined(LCSFICATION)
    for (int i = 0; i < MapLegendCounter; i += 2) {
        CRGBA col = MapLegendBlipColor[i];
        x = SCREEN_WIDTH / 2;
        x -= ScaleX(shift);
        DrawLegendEntry(x + GetMenuOffsetX(), y, MapLegendList[i], &col);

        if (MapLegendList[i + 1] != RADAR_SPRITE_NONE && i < 74) {
            x = SCREEN_WIDTH / 2;
            x += ScaleX(shift / 8);
            DrawLegendEntry(x + GetMenuOffsetX(), y, MapLegendList[i + 1], &col);
        }

        y += ScaleY(18.0f);
    }
#endif
#endif
}

#if defined(GTA3) && defined(LCSFICATION)
void CMenuNew::DrawLegendEntry(float x, float y, short id, CRGBA* col) {
    CFont::SetPropOn();
    CFont::SetWrapx(SCREEN_WIDTH * 2);
    CFont::SetBackGroundOnlyTextOff();
    CFont::SetCentreOff();
    CFont::SetRightJustifyOff();

    CFont::SetDropShadowPosition(1);
    CFont::SetDropColor(CRGBA(0, 0, 0, 255));
    CFont::SetFontStyle(0);
    CFont::SetColor(CRGBA(225, 225, 225, 255));
    CFont::SetScale(ScaleX(0.34f), ScaleY(0.48f));

    if (id == RADAR_WAYPOINT) {
        DrawWayPoint(x, y + ScaleY(4.0f), ScaleX(LEGEND_BLIP_SCALE), ScaleY(LEGEND_BLIP_SCALE), CRGBA(255, 0, 0, GetAlpha(255)));
    }
    else if (id < RADAR_DESTINATION) {
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

        DrawLevel(x, y + ScaleY(4.0f), ScaleX(LEGEND_BLIP_SCALE * 0.5f), ScaleY(LEGEND_BLIP_SCALE * 0.5f), level, CRGBA(col->r, col->g, col->b, GetAlpha(255)));
    }
    else {
        CSprite2d* sprite = RadarSpritesArray[id];

        if (sprite)
            DrawSpriteWithRotation(sprite, x, y + ScaleY(4.0f), ScaleX(LEGEND_BLIP_SCALE), ScaleY(LEGEND_BLIP_SCALE), 0.0f, CRGBA(255, 255, 255, GetAlpha(255)));
    }

    if (id < 0)
        id = abs(id) + 100;

    const float _x = x + ScaleX(LEGEND_BLIP_SCALE);
    const float _y = y;
    if (settings.readStringsFromThisFile) {
        CFont::PrintString(_x, _y, settings.gxt.at(id).c_str());
    }
    else {
        char buff[16];
        sprintf(buff, "LG_%02d", id);
        CFont::PrintString(_x, _y, TheText.Get(buff));
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
