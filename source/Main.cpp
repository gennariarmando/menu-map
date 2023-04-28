#include "plugin.h"
#include "CMenuManager.h"

#include "MenuNew.h"
#include "Utility.h"

#include "ModuleList.hpp"

using namespace plugin;

class MenuMap {
public:
    MenuMap() {
        MenuNew = std::make_unique<CMenuNew>();

#ifdef _DEBUG
        AllocConsole();
        freopen("conin$", "r", stdin);
        freopen("conout$", "w", stdout);
        freopen("conout$", "w", stderr);
        std::setvbuf(stdout, NULL, _IONBF, 0);
#endif
#ifdef GTA3
        const CMenuScreen pauseMenuPage = {
            "FET_PAU", 1, -1, -1, 0, 0,
            MENUACTION_RESUME, "FEM_RES", 0, MENUPAGE_NONE,
            MENUACTION_CHANGEMENU, "FEN_STA", 0, MENUPAGE_NEW_GAME,
            MENUACTION_CHANGEMENU, "FEG_MAP", 0, MENUPAGE_MAP,
            MENUACTION_CHANGEMENU, "FEP_STA", 0, MENUPAGE_STATS,
            MENUACTION_CHANGEMENU, "FEP_BRI", 0, MENUPAGE_BRIEFS,
            MENUACTION_CHANGEMENU, "FET_OPT", 0, MENUPAGE_OPTIONS,
            MENUACTION_CHANGEMENU, "FEM_QT", 0, MENUPAGE_EXIT,
        };

        plugin::patch::Set(0x611930 + sizeof(CMenuScreen) * MENUPAGE_PAUSE_MENU, pauseMenuPage);
        const CMenuScreen mapMenuPage = {
            "", 1, MENUPAGE_PAUSE_MENU, MENUPAGE_PAUSE_MENU, 2, 2,
        };

        plugin::patch::Set(0x611930 + sizeof(CMenuScreen) * MENUPAGE_MAP, mapMenuPage);

        ThiscallEvent <AddressList<0x47AB12, H_CALL>, PRIORITY_AFTER, ArgPickN<CMenuManager*, 0>, void(CMenuManager*)> onDrawStandardMenu;
        onDrawStandardMenu += [](CMenuManager* menuManager) {
            if (!MenuNew || !MenuNew->menuManager)
                return;

            switch (MenuNew->menuManager->m_nCurrentMenuPage) {
            case MENUPAGE_MAP:
                MenuNew->MapInput();
                MenuNew->DrawMap();
                break;
            default:
                MenuNew->ResetMap(true);
                MenuNew->clearInput = true;
                break;
            }
        };

        ThiscallEvent <AddressList<0x48E721, H_CALL, 0x48C8A4, H_CALL>, PRIORITY_AFTER, ArgPickN<CMenuManager*, 0>, void(CMenuManager*)> onProcess;
        onProcess += [](CMenuManager* menuManager) {
            if (!menuManager->m_bMenuActive) {
                MenuNew->ResetMap(true);
            }
        };

#else
        ThiscallEvent <AddressList<0x4A325E, H_CALL, 0x4A32AD, H_CALL>, PRIORITY_AFTER, ArgPickN<CMenuManager*, 0>, void(CMenuManager*, int)> onDrawStandardMenu;
        onDrawStandardMenu += [](CMenuManager* menuManager) {
            if (!MenuNew || !MenuNew->menuManager)
                return;

            switch (MenuNew->menuManager->m_nCurrentMenuPage) {
            case MENUPAGE_MAP:
                MenuNew->MapInput();
                break;
            default:
                MenuNew->ResetMap(true);
                MenuNew->clearInput = true;
                break;
            }
        };

#ifdef GTASA
        plugin::patch::RedirectCall(0x57BA28, (void(__fastcall*)(CMenuManager*, int))drawMap);
        plugin::patch::Nop(0x057BA08, 9);
#elif GTAVC
        auto drawMap = [](CMenuManager*, int) {
            if (!MenuNew || !MenuNew->menuManager)
                return;

            MenuNew->DrawMap();
        };
        plugin::patch::RedirectJump(0x49A5B7, (void(__fastcall*)(CMenuManager*, int))drawMap);

        // No map draw
        //plugin::patch::SetUChar(0x4A2CBE, 0xEB);
        //plugin::patch::SetUChar(0x4A2736, 0xEB);

        // No map input
        plugin::patch::Set<BYTE>(0x4973C9, 0xEB);
        plugin::patch::Set<BYTE>(0x496600 + 6, -1);

        ThiscallEvent <AddressList<0x4A4433, H_CALL, 0x4A5C88, H_CALL>, PRIORITY_AFTER, ArgPickN<CMenuManager*, 0>, void(CMenuManager*)> onProcess;
        onProcess += [](CMenuManager* menuManager) {
            if (!menuManager->m_bMenuActive) {
                MenuNew->ResetMap(true);
            }
        };
#endif
#endif

        plugin::Events::initRwEvent += [] {
            const HMODULE h = ModuleList().GetByPrefix(L"skyui");
            if (h) {
                MenuNew->settings.skyUI = true;
            }
        };

        plugin::Events::drawBlipsEvent += [] {
            if (!MenuNew || !MenuNew->menuManager)
                return;

            if (MenuNew->targetBlipIndex) {
                CVector2D in = CVector2D(0.0f, 0.0f);
                CVector2D out = CVector2D(0.0f, 0.0f);
                CRadar::TransformRealWorldPointToRadarSpace(in, CVector2D(MenuNew->targetBlipWorldPos.x, MenuNew->targetBlipWorldPos.y));
                CRadar::LimitRadarPoint(in);
                CRadar::TransformRadarPointToScreenSpace(out, in);

                DrawWayPoint(out.x, out.y, ScaleX(RADAR_BLIPS_SCALE), ScaleY(RADAR_BLIPS_SCALE), CRGBA(255, 0, 0, 255));
            }
        };
    }
} menuMap;
