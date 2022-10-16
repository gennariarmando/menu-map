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
#endif
#include "CTheZones.h"
#include "CFont.h"
#include "CText.h"

using namespace plugin;

std::unique_ptr<CMenuNew> MenuNew;

CMenuNew::CMenuNew() {
    menuManager = &FrontEndMenuManager;
    m_fMapZoom = MAP_ZOOM_MIN;
    m_vMapBase = {};
    m_vCrosshair = {};
    targetBlipIndex = 0;
    targetBlipWorldPos = {};
    clearInput = false;
    settings.Read();
}

CMenuNew::~CMenuNew() {
    menuManager = nullptr;
}

void CMenuNew::DrawMap() {
    if (!menuManager)
        return;

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

    CSprite2d::DrawRect(CRect(-5.0f, -5.0f, SCREEN_WIDTH + 5.0f, SCREEN_HEIGHT + 5.0f), CRGBA(0, 0, 0, 255));
    CSprite2d::DrawRect(CRect(-5.0f, -5.0f, SCREEN_WIDTH + 5.0f, SCREEN_HEIGHT + 5.0f), CRGBA(settings.backgroundColor.r, settings.backgroundColor.g, settings.backgroundColor.b, settings.backgroundColor.a));

    CRGBA col = { settings.radarMapColor.r, settings.radarMapColor.g, settings.radarMapColor.b,
#ifdef GTASA
        255
#else
        static_cast<unsigned char>(menuManager->FadeIn(settings.radarMapColor.a))
#endif
    };
    CRect rect;

    const float mapHalfSize = GetMenuMapWholeSize() / 2;
    float mapZoom = GetMenuMapTileSize() * m_fMapZoom;
    rect.left = m_vMapBase.x;
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
        rect.left = m_vMapBase.x;
        rect.right = rect.left + mapZoom;

        rect.top += mapZoom;
        rect.bottom = rect.top + mapZoom;
    }

    DrawBlips();
    DrawCrosshair(m_vCrosshair.x, m_vCrosshair.y);
    DrawZone();

#ifdef GTAVC
    DrawLegend();
    menuManager->m_bDrawRadarOrMap = false;
    menuManager->DisplayHelperText("FEH_MPH");
#endif

}

void CMenuNew::DrawCrosshair(float x, float y) {
    float lineSize = ScaleY(2.0f);
    CRGBA lineCol = CRGBA(settings.crosshairColor.r, settings.crosshairColor.g, settings.crosshairColor.b, 
#ifdef GTASA
        255
#else
        menuManager->FadeIn(settings.crosshairColor.a)
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
    CSprite2d::DrawRect(CRect(-5.0f, SCREEN_HEIGHT + 5.0f, SCREEN_WIDTH + 5.0f, SCREEN_HEIGHT - ScaleY(42.0f)), CRGBA(10, 10, 10, menuManager->FadeIn(255)));
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
        CFont::SetColor(CRGBA(settings.zoneNameColor.r, settings.zoneNameColor.g, settings.zoneNameColor.b, 
#ifdef GTASA
            255
#else
            menuManager->FadeIn(settings.zoneNameColor.a)
#endif
        ));
#ifdef GTA3
        CFont::SetDropColor(CRGBA(0, 0, 0, 0));
        CFont::SetDropShadowPosition(0);
        CFont::SetRightJustifyOff();
        CFont::SetFontStyle(0);
        CFont::SetScale(ScaleX(0.54f), ScaleY(1.12f));
        CFont::PrintString(ScaleX(16.0f), SCREEN_HEIGHT - ScaleY(34.0f), str);
        CFont::SetRightJustifyOn();
#else
        CFont::SetDropColor(CRGBA(0, 0, 0, 255));
        CFont::SetDropShadowPosition(2);
        CFont::SetFontStyle(2);
        CFont::SetScale(ScaleX(0.64f), ScaleY(1.28f));
        CFont::PrintString(SCREEN_WIDTH - ScaleX(72.0f), SCREEN_HEIGHT - ScaleY(102.0f), (char*)str);
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
        unsigned short id = trace.m_nRadarSprite;
        int handle = trace.m_nEntityHandle;
        CSprite2d* sprite =
#ifdef GTASA
            & CRadar::RadarBlipSprites[id];
#else
            pRadarSprites[id];
#endif
        CEntity* e = NULL;

        if (trace.m_nBlipDisplay < BLIP_DISPLAY_BLIP_ONLY)
            continue;

        switch (trace.m_nBlipType) {
        case BLIP_COORD:
        case BLIP_CONTACTPOINT:
            if (!CTheScripts::IsPlayerOnAMission() || trace.m_nBlipType == BLIP_COORD) {
                if (id > RADAR_SPRITE_NONE && id < RADAR_SPRITE_COUNT) {
                    DrawSpriteWithRotation(sprite, pos.x, pos.y, ScaleX(RADAR_BLIPS_SCALE), ScaleY(RADAR_BLIPS_SCALE), 0.0f, CRGBA(255, 255, 255,
#ifdef GTASA
                        255
#else
                        menuManager->FadeIn(255)
#endif
                        ));
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
                    DrawLevel(pos.x, pos.y, ScaleX(RADAR_BLIPS_SCALE * 0.45f), ScaleY(RADAR_BLIPS_SCALE * 0.45f), level, CRGBA(col.r, col.g, col.b,
#ifdef GTASA
                        255
#else
                        menuManager->FadeIn(255)
#endif                    
                        ));
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
                    DrawSpriteWithRotation(sprite, pos.x, pos.y, ScaleX(RADAR_BLIPS_SCALE), ScaleY(RADAR_BLIPS_SCALE), 0.0f, CRGBA(255, 255, 255,
#ifdef GTASA
                        255
#else
                        menuManager->FadeIn(255)
#endif
                    ));
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
                    DrawLevel(pos.x, pos.y, ScaleX(RADAR_BLIPS_SCALE * 0.45f), ScaleY(RADAR_BLIPS_SCALE * 0.45f), level, CRGBA(col.r, col.g, col.b,
#ifdef GTASA
                        255
#else
                        menuManager->FadeIn(255)
#endif
                    ));
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
        };
        std::vector<fb> pos = {
            { 1071.2f, -400.0f, RADAR_SPRITE_WEAPON }, // ammu1
            { 345.5f, -713.5f, RADAR_SPRITE_WEAPON }, // ammu2
            { -1200.8f, -24.5f, RADAR_SPRITE_WEAPON }, // ammu3
            { 925.0f, -359.5f, RADAR_SPRITE_SPRAY }, // spray1
            { 379.0f, -493.7f, RADAR_SPRITE_SPRAY  }, // spray2
            { -1142.0f, 34.7f, RADAR_SPRITE_SPRAY }, // spray3
            { 1282.1f, -104.8f, RADAR_SPRITE_BOMB }, // bomb1
            { 380.0f, -576.6f, RADAR_SPRITE_BOMB }, // bomb2
            { -1082.5f, 55.2f, RADAR_SPRITE_BOMB }, // bomb3
        };

        for (auto& it : pos) {
            CVector2D pos = WorldToMap({ it.x, it.y, 0.0f });
            DrawSpriteWithRotation(pRadarSprites[it.sprite], pos.x, pos.y, ScaleX(RADAR_BLIPS_SCALE), ScaleY(RADAR_BLIPS_SCALE), 0.0f, CRGBA(255, 255, 255,
#ifdef GTASA
                255
#else
                menuManager->FadeIn(255)
#endif
            ));
        }
    }
#endif

    // Draw waypoint separately
    if (targetBlipIndex) {
        CVector2D pos = WorldToMap(targetBlipWorldPos);
        DrawWayPoint(pos.x, pos.y, ScaleX(RADAR_BLIPS_SCALE), ScaleY(RADAR_BLIPS_SCALE));
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
            &CRadar::RadarBlipSprites[RADAR_SPRITE_CENTRE];
#else 
            pRadarSprites[RADAR_SPRITE_CENTRE];
#endif

        if (sprite && flashItem(1000, 200))
            DrawSpriteWithRotation(sprite, pos.x, pos.y, ScaleX(RADAR_BLIPS_SCALE - 1.0f), ScaleY(RADAR_BLIPS_SCALE - 1.0f), angle, CRGBA(255, 255, 255,
#ifdef GTASA
                255
#else
                menuManager->FadeIn(255)
#endif
            ));
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
    bool leftBound = (m_vMapBase.x - halfMapSize < 0.0f);
    bool rightBound = (m_vMapBase.x + halfMapSize > SCREEN_WIDTH);
    bool topBound = (m_vMapBase.y - halfMapSize < 0.0f);
    bool bottomBound = (m_vMapBase.y + halfMapSize > SCREEN_HEIGHT);
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
        menuManager->m_bMapLegend = menuManager->m_bMapLegend == false;
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
        const int mult = 4.0f;
        float right = pad->NewKeyState.left ? -1.0f : pad->NewKeyState.right ? 1.0f : 0.0f;
        float up = pad->NewKeyState.up ? -1.0f : pad->NewKeyState.down ? 1.0f : 0.0f;

        right += pad->NewState.DPadLeft ? -1.0f : pad->NewState.DPadRight ? 1.0f : 0.0f;
        up += (pad->NewState.DPadUp || pad->NewState.LeftStickY < 0.0f) ? -1.0f : (pad->NewState.DPadDown || pad->NewState.LeftStickY > 0.0f) ? 1.0f : 0.0f;

        right += pad->NewState.LeftStickX / 128.0f;
        up += pad->NewState.LeftStickY / 128.0f;

        right = clamp(right, -1.0f, 1.0f);
        up = clamp(up, -1.0f, 1.0f);

        if (isNearlyEqualF(m_vCrosshair.x, SCREEN_WIDTH / 2, 32.0f)) {
            if ((right < 0.0f && leftBound || right > 0.0f && rightBound)) {
                m_vCrosshair.x = SCREEN_WIDTH / 2;
                m_vMapBase.x -= right * mult;
            }
        }

        if (isNearlyEqualF(m_vCrosshair.y, SCREEN_HEIGHT / 2, 32.0f)) {
            if ((up < 0.0f && topBound || up > 0.0f && bottomBound)) {
                m_vCrosshair.y = SCREEN_HEIGHT / 2;
                m_vMapBase.y -= up * mult;
            }
        }
        m_vCrosshair.x += right * mult;
        m_vCrosshair.y += up * mult;
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
#ifdef GTAVC
    if (!menuManager->m_bMapLegend)
        return;

    const float shift = 182.0f;
    float x = SCREEN_WIDTH / 2;
    float y = ScaleY(128.0f);

    CFont::SetPropOn();
    CFont::SetWrapx(SCREEN_WIDTH);
    CFont::SetBackGroundOnlyTextOff();
    CFont::SetCentreOff();
    CFont::SetRightJustifyOff();

    CFont::SetDropShadowPosition(1);
    CFont::SetDropColor(CRGBA(0, 0, 0, menuManager->FadeIn(255)));
    CFont::SetFontStyle(1);
    CFont::SetColor(CRGBA(225, 225, 225, menuManager->FadeIn(255)));
    CFont::SetScale(ScaleX(0.34f), ScaleY(0.48f));

    CSprite2d::DrawRect(CRect(x - ScaleX(shift * 1.1f), y - ScaleY(8.0f), x + ScaleX(shift * 1.1f), y + ScaleY(8.0f) + ScaleY(17.0f * (CRadar::MapLegendCounter - (1 * CRadar::MapLegendCounter / 2)))), CRGBA(0, 0, 0, menuManager->FadeIn(155)));

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
#endif
}
