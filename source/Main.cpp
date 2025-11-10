#include "plugin.h"
#include "CMenuManager.h"
#include "CTxdStore.h"
#include "CTheScripts.h"

#include "MenuNew.h"
#include "Utility.h"

#include "ModuleList.hpp"

#include "debugmenu_public.h"

#include "ScmExtenderAPI.h"
#include "SaveExtenderAPI.h"

#include "SpriteLoader.h"

using namespace plugin;

DebugMenuAPI gDebugMenuAPI;

class MenuMap {
public:
#ifdef WITH_VCS_MAP_OPTIONS
    //static void Save(std::string const& filename) {
    //    std::ofstream file(filename, std::ios::binary | std::ios::trunc);
    //    if (file.is_open()) {
    //        size_t size = aPackages.size();
    //        file.write(reinterpret_cast<const char*>(&size), sizeof(size));
    //
    //        for (const auto& package : aPackages) {
    //            const Collectable& value = package;
    //            file.write(reinterpret_cast<const char*>(&value), sizeof(value));
    //        }
    //
    //        size = aRampages.size();
    //        file.write(reinterpret_cast<const char*>(&size), sizeof(size));
    //
    //        for (const auto& rampage : aRampages) {
    //            const Collectable& value = rampage;
    //            file.write(reinterpret_cast<const char*>(&value), sizeof(value));
    //        }
    //
    //        size = aUniqueStunts.size();
    //        file.write(reinterpret_cast<const char*>(&size), sizeof(size));
    //
    //        for (const auto& uniqueStunts : aUniqueStunts) {
    //            const Collectable& value = uniqueStunts;
    //            file.write(reinterpret_cast<const char*>(&value), sizeof(value));
    //        }
    //
    //        file.close();
    //    }
    //}
    //
    //static void Load(std::string const& filename) {
    //    std::ifstream file(filename, std::ios::binary);
    //    if (file.is_open()) {
    //        aPackages = {};
    //        size_t size = 0;
    //        file.read(reinterpret_cast<char*>(&size), sizeof(size));
    //
    //        for (size_t i = 0; i < size; i++) {
    //            Collectable value;
    //            file.read(reinterpret_cast<char*>(&value), sizeof(value));
    //
    //            aPackages.push_back(value);
    //        }
    //
    //        aRampages = {};
    //        size = 0;
    //        file.read(reinterpret_cast<char*>(&size), sizeof(size));
    //
    //        for (size_t i = 0; i < size; i++) {
    //            Collectable value;
    //            file.read(reinterpret_cast<char*>(&value), sizeof(value));
    //
    //            aRampages.push_back(value);
    //        }
    //
    //        aUniqueStunts = {};
    //        size = 0;
    //        file.read(reinterpret_cast<char*>(&size), sizeof(size));
    //
    //        for (size_t i = 0; i < size; i++) {
    //            Collectable value;
    //            file.read(reinterpret_cast<char*>(&value), sizeof(value));
    //
    //            aUniqueStunts.push_back(value);
    //        }
    //
    //        file.close();
    //    }
    //}
#endif

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
#ifdef WITH_ANIMATION
                if (!MenuNew->menuManager->m_bMenuActive)
#endif
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

#ifdef WITH_VCS_MAP_OPTIONS
        static CdeclEvent <AddressList<0x430652, H_CALL>, PRIORITY_AFTER, ArgPickN<int32_t, 0>, void(int32_t)> onNewPickup;
        onNewPickup += [](int32_t id) {
            auto const& pickup = CPickups::aPickUps[id];

            if (pickup.m_nPickupType == ePickupType::PICKUP_COLLECTABLE1) {
                Collectable col;
                col.pos = pickup.m_vecPos;
                col.removed = false;
                aPackages.push_back(col);
            }
            else if (pickup.m_nPickupType == ePickupType::PICKUP_ONCE && pickup.m_nModelIndex == MODEL_KILLFRENZY) {
                Collectable col;
                col.pos = pickup.m_vecPos;
                col.removed = false;
                aRampages.push_back(col);
            }
        };

        static CdeclEvent <AddressList<0x4312FA, H_CALL>, PRIORITY_AFTER, ArgPickN<CEntity*, 0>, void(CEntity*)> onRemoveCollectible;
        onRemoveCollectible += [](CEntity* e) {
            auto it = std::find_if(aPackages.begin(), aPackages.end(),
                [&](const Collectable& c) {
                return c.pos.x == e->GetPosition().x && c.pos.y == e->GetPosition().y && c.pos.z == e->GetPosition().z;
            });
            if (it != aPackages.end()) {
                it->removed = true;
                return;
            }

            it = std::find_if(aRampages.begin(), aRampages.end(),
                [&](const Collectable& c) {
                return c.pos.x == e->GetPosition().x && c.pos.y == e->GetPosition().y && c.pos.z == e->GetPosition().z;
            });
            if (it != aRampages.end()) {
                it->removed = true;
                return;
            }
        };

        //static char* ValidSaveName = (char*)0x8E2CBC;
        //static char* LoadFileName = (char*)0x9403C4;
        //
        //static CdeclEvent <AddressList<0x48C7CC, H_CALL>, PRIORITY_BEFORE, ArgPickNone, void()> onGenericLoad;
        //onGenericLoad += []() {
        //    std::string str = LoadFileName;
        //
        //    str = plugin::RemoveExtension(str);
        //    str += ".a";
        //    Load(str);
        //    std::cout << "" << std::endl;
        //};
        //
        //static CdeclEvent <AddressList<0x591F16, H_CALL>, PRIORITY_AFTER, ArgPickNone, void(int32_t)> onGenericSave;
        //onGenericSave += []() {
        //    std::string str = ValidSaveName;
        //
        //    str = plugin::RemoveExtension(str);
        //    str += ".a";
        //    Save(str);
        //};

        plugin::Events::shutdownRwEvent += []() {
            RadarPackageSprite.Delete();
            RadarRampageSprite.Delete();
            RadarStuntJumpSprite.Delete();
        };

        plugin::Events::restartGameEvent += []() {
            aPackages = {};
            aRampages = {};
            aUniqueStunts = {};
        };

        plugin::Events::initRwEvent += []() {
            SaveExtender::OnSavingEvent([]() {
                SaveExtender::Serialize("PACKAGES", aPackages, aPackages.size() * sizeof(Collectable));
                SaveExtender::Serialize("RAMPAGES", aRampages, aRampages.size() * sizeof(Collectable));
                SaveExtender::Serialize("UNIQUESTUNTS", aUniqueStunts, aUniqueStunts.size() * sizeof(Collectable));
            });

            SaveExtender::OnLoadingEvent([]() {
                if (aPackages.empty())
                    aPackages.resize(100);

                if (aRampages.empty())
                    aRampages.resize(100);

                if (aUniqueStunts.empty())
                    aUniqueStunts.resize(100);

                SaveExtender::Retrieve("PACKAGES", &aPackages, aPackages.size() * sizeof(Collectable));
                SaveExtender::Retrieve("RAMPAGES", &aRampages, aRampages.size() * sizeof(Collectable));
                SaveExtender::Retrieve("UNIQUESTUNTS", &aUniqueStunts, aUniqueStunts.size() * sizeof(Collectable));
            });
        };
#endif
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
        plugin::Events::initRwEvent += []() {
#ifdef WITH_VCS_MAP_OPTIONS
            if (DebugMenuLoad()) {
                DebugMenuAddVarBool8("MenuMap", "Show all Hidden Packages", (int8_t*)&Debug_ShowAllHiddenPackages, nullptr);
                DebugMenuAddVarBool8("MenuMap", "Show all Rampages", (int8_t*)&Debug_ShowAllRampages, nullptr);
                DebugMenuAddVarBool8("MenuMap", "Show all Unique Stunts", (int8_t*)&Debug_ShowAllUniqueStunts, nullptr);
            }

            enum {
                REGISTER_STUNT_JUMP = 4600,
                SET_STUNT_JUMP_COMPLETED,
            };
            ScmExtender::AddOneCommand(REGISTER_STUNT_JUMP, [](int32_t* params) -> int8_t {
                ScmExtender::CollectParams(4);

                tScriptParam* p = (tScriptParam*)params;
                Collectable col;
                col.index = p[0].iParam;
                col.pos = *(CVector*)&p[1].fParam;
                col.removed = false;
                aUniqueStunts.push_back(col);

                return 0;
            });

            ScmExtender::AddOneCommand(SET_STUNT_JUMP_COMPLETED, [](int32_t* params) -> int8_t {
                ScmExtender::CollectParams(1);

                tScriptParam* p = (tScriptParam*)params;

                auto it = std::find_if(aUniqueStunts.begin(), aUniqueStunts.end(),
                    [&](const Collectable& c) {
                    return c.index == p[0].iParam;
                });

                if (it != aUniqueStunts.end())
                    it->removed = true;

                return 0;
            });
#endif
        };

        plugin::Events::initGameEvent += [] {
            const HMODULE h = ModuleList().GetByPrefix(L"skyui");
            if (h) {
                MenuNew->settings.skyUI = true;
            }
            
#ifdef GTA3
            RadarTraceArray = plugin::patch::Get<tRadarTrace*>(0x4A55CA + 2);
            RadarSpritesArray = plugin::patch::Get<CSprite2d**>(0x4A6004 + 3);
            RadarTraceArraySize = *(uint8_t*)0x4A47FC;

#ifdef WITH_VCS_MAP_OPTIONS
                int32_t slot = CTxdStore::FindTxdSlot("hud");
                CTxdStore::SetCurrentTxd(slot);
                RadarPackageSprite.m_pTexture = RwTextureRead("radar_package", nullptr);
                RadarRampageSprite.m_pTexture = RwTextureRead("radar_rampage", nullptr);
                RadarStuntJumpSprite.m_pTexture = RwTextureRead("radar_stuntjump", nullptr);

                CTxdStore::PopCurrentTxd();     

                aPackages = {};
                aRampages = {};
                aUniqueStunts = {};
#endif
#endif

                spriteLoader.LoadSpriteFromFolder("models\\map.png");

                if (spriteLoader.GetTex("map"))
                    MenuNew->dontStreamRadarTiles = true;
        };

        plugin::Events::shutdownRwEvent += []() {
            spriteLoader.Clear();
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
