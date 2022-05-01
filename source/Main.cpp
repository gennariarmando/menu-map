#include "plugin.h"
#include "CMenuManager.h"

#include "MenuNew.h"
#include "Utility.h"

using namespace plugin;

class MenuMap {
public:
    MenuMap() {
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

        ThiscallEvent <AddressList<0x47AB12, H_CALL>, PRIORITY_AFTER, ArgPickN<CMenuManager*, 0>, void(CMenuManager*)> onDrawingMenuManager;
        onDrawingMenuManager += [](CMenuManager* menuManager) {
            if (!MenuNew || !MenuNew->menuManager)
                return;

            switch (MenuNew->menuManager->m_nCurrentMenuScreen) {
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
#else
        const CMenuScreen mapMenuPage = {
            "FEH_MAP", MENUPAGE_34, 2,
            {

            }
        };

        plugin::patch::Set(0x6D8B70 + sizeof(CMenuScreen) * MENUPAGE_MAP, mapMenuPage);

        auto drawMap = [](CMenuManager*, int) {
            if (!MenuNew || !MenuNew->menuManager)
                return;

            switch (MenuNew->menuManager->m_nCurrentMenuScreen) {
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

        plugin::patch::RedirectCall(0x4A2CC2, (void(__fastcall*)(CMenuManager*, int))drawMap);
        plugin::patch::Nop(0x4A2CB7, 9);

        plugin::patch::RedirectCall(0x4A273A, (void(__fastcall*)(CMenuManager*, int))drawMap);
        plugin::patch::Nop(0x4A272F, 9);

        plugin::patch::Set<BYTE>(0x4973C9, 0xEB);
        plugin::patch::Set<BYTE>(0x496600 + 6, -1);
#endif

        plugin::Events::initRwEvent += [] {
            MenuNew = std::make_shared<CMenuNew>();
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

                DrawWayPoint(out.x, out.y, ScaleX(RADAR_BLIPS_SCALE), ScaleY(RADAR_BLIPS_SCALE));
            }
        };
    }
} menuMap;
