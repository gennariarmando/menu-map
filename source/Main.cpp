#include "plugin.h"
#include "CMenuManager.h"

#include "MenuNew.h"
#include "Utility.h"

using namespace plugin;

int targetBlipIndex = 0;
CVector targetBlipWorldPos;

class MenuMap {
public:
	MenuMap() {
		const CMenuScreen pauseMenuPage = {
			"FET_PAU", 1, -1, -1, 0, 0,
			MENUACTION_RESUME,		"FEM_RES",	0, MENUPAGE_NONE,
			MENUACTION_CHANGEMENU,	"FEN_STA",	0, MENUPAGE_NEW_GAME,
			MENUACTION_CHANGEMENU,	"FEG_MAP",	0, MENUPAGE_MAP,
			MENUACTION_CHANGEMENU,	"FEP_STA",	0, MENUPAGE_STATS,
			MENUACTION_CHANGEMENU,	"FEP_BRI",	0, MENUPAGE_BRIEFS,
			MENUACTION_CHANGEMENU,	"FET_OPT",	0, MENUPAGE_OPTIONS,
			MENUACTION_CHANGEMENU,	"FEM_QT",	0, MENUPAGE_EXIT,
		};

		plugin::patch::Set(0x611930 + sizeof(CMenuScreen) * MENUPAGE_PAUSE_MENU, pauseMenuPage);

		const CMenuScreen mapMenuPage = {
			"", 1, MENUPAGE_PAUSE_MENU, MENUPAGE_PAUSE_MENU, 2, 2,
		};

		plugin::patch::Set(0x611930 + sizeof(CMenuScreen) * MENUPAGE_MAP, mapMenuPage);

		ThiscallEvent <AddressList<0x47AB12, H_CALL>, PRIORITY_AFTER, ArgPickN<CMenuManager*, 0>, void(CMenuManager*)> onDrawingMenuManager;
		onDrawingMenuManager += [](CMenuManager* menuManager) {
			CMenuNew* menuNew = static_cast<CMenuNew*>(menuManager);

			switch (menuNew->m_nCurrentMenuScreen) {
			case MENUPAGE_MAP:
				menuNew->MapInput();
				menuNew->DrawMap();
				targetBlipIndex = menuNew->targetBlipIndex;
				targetBlipWorldPos = menuNew->targetBlipWorldPos;
				break;
			default:
				menuNew->ResetMap();
				menuNew->clearInput = true;
				break;
			}
		};

		plugin::Events::drawBlipsEvent += [] {
			if (targetBlipIndex) {
				CVector2D in = CVector2D(0.0f, 0.0f);
				CVector2D out = CVector2D(0.0f, 0.0f);			
				CRadar::TransformRealWorldPointToRadarSpace(in, CVector2D(targetBlipWorldPos.x, targetBlipWorldPos.y));
				CRadar::LimitRadarPoint(in);
				CRadar::TransformRadarPointToScreenSpace(out, in);

				DrawWayPoint(out.x, out.y, Scale(22.0f), CRGBA(0, 0, 0, 255));
				DrawWayPoint(out.x, out.y, Scale(20.0f), CRGBA(255, 0, 0, 255));
				DrawWayPoint(out.x, out.y, Scale(19.0f), CRGBA(0, 0, 0, 255));
			}
		};
	}
} menuMap;