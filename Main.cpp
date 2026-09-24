#include <plugin.h> // Plugin-SDK version 1005 from 2026-08-14 08:10:00
#include <CStats.h>
#include <mini/ini.h>
#include <CCheat.h>
#include <CHud.h>

using namespace plugin;

struct Main
{
    std::string cheat = "GUNWEIGHT";
    float velocity = 0.0;

    std::unordered_map<unsigned int, float> weaponMasses;
    std::unordered_map<unsigned int, float> ammoMasses;

    void readConfig() {
        weaponMasses.clear();
        ammoMasses.clear();
        mINI::INIFile file(PLUGIN_PATH("config.ini"));
        mINI::INIStructure ini;
        file.read(ini);
        for (const std::pair<const std::string, std::string>& weapon : ini["weapons"]) {
            weaponMasses[std::stoi(weapon.first)] = std::stof(weapon.second);
        }
        for (const std::pair<const std::string, std::string>& ammo : ini["ammo"]) {
            ammoMasses[std::stoi(ammo.first)] = std::stof(ammo.second);
        }
        cheat = ini["settings"]["cheat"];
        std::transform(cheat.begin(), cheat.end(), cheat.begin(), ::toupper);
        std::reverse(cheat.begin(), cheat.end());
        static char msg[255];
        sprintf_s(msg, "Reloaded config for GunWeight!");
        CHud::SetHelpMessage(msg, true, false, false);
    }

    Main()
    {
        // register event callbacks
        Events::gameProcessEvent += []{ gInstance.OnGameProcess(); };
        readConfig();
    }

    std::vector<unsigned int> anims = {0, 1, 2};

    unsigned int nextTime = 0;

    float getWeaponMass(eWeaponType weapType) {
        unsigned int intWeapType = (int)weapType;
        if (weaponMasses.contains(intWeapType)) {
            return weaponMasses[intWeapType];
        }
        else {
            return 0.0f;
        }
    }
    float getAmmoMass(eWeaponType weapType) {
        unsigned int intWeapType = (int)weapType;
        if (ammoMasses.contains(intWeapType)) {
            return ammoMasses[intWeapType];
        }
        else {
            return 0.0f;
        }
    }

    void OnGameProcess()
    {
        CPlayerPed* player = FindPlayerPed();
        if (!player || !player->m_pRwClump) return;

        if (strncmp(CCheat::m_CheatString, cheat.c_str(), cheat.length()) == 0) {
            CCheat::m_CheatString[0] = '\0';
            readConfig();
        }

        float playerMass = player->m_fMass * ((1000.0f + CStats::GetStatValue(STAT_FAT)) / 1000.0f) * ((1000.0f + CStats::GetStatValue(STAT_MUSCLE)) / 1000.0f);
        float weaponMass = 0.0;
        for (const CWeapon& weapon : player->m_aWeapons) {
            eWeaponType weapType = weapon.m_eWeaponType;
            float weapOnlyMass = getWeaponMass(weapType);
            float ammoMass = getAmmoMass(weapType);
            float weapMass = weapOnlyMass + ammoMass * weapon.m_nAmmoTotal;
            weaponMass += weapMass;
        }

        for (unsigned int anim : anims) {
            CAnimBlendAssociation* walkAnim = RpAnimBlendClumpGetAssociation(player->m_pRwClump, anim);
            if (walkAnim) {
                float massMult = playerMass / (playerMass + weaponMass);
                walkAnim->m_fSpeed = massMult;
            }
        }
    }
} gInstance;