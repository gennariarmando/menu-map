#include "plugin.h"
#include "CMenuManager.h"

#include "MenuNew.h"

using namespace plugin;

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
			CMenuNew* newMenu = static_cast<CMenuNew*>(menuManager);

			switch (newMenu->m_nCurrentMenuScreen) {
			case MENUPAGE_MAP:
				newMenu->MapInput();
				newMenu->DrawMap();
				break;
			default:
				newMenu->ResetMap();
				newMenu->clearInput = true;
				break;
			}
		};
	}
} menuMap;