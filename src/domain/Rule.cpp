#include "Rule.h"
#include <string.h>

namespace Domain
{

    Rule Rule::fromJapaneseName(const char *japaneseName)
    {
        if (strcmp(japaneseName, "ナワバリバトル") == 0)
        {
            return turfWar();
        }
        else if (strcmp(japaneseName, "ガチエリア") == 0)
        {
            return splatZones();
        }
        else if (strcmp(japaneseName, "ガチヤグラ") == 0)
        {
            return towerControl();
        }
        else if (strcmp(japaneseName, "ガチホコバトル") == 0)
        {
            return rainmaker();
        }
        else if (strcmp(japaneseName, "ガチアサリ") == 0)
        {
            return clamBlitz();
        }
        else
        {
            return unknown();
        }
    }

    const char *Rule::getJapaneseName() const
    {
        switch (type)
        {
        case Type::TURF_WAR:
            return "ナワバリバトル";
        case Type::SPLAT_ZONES:
            return "ガチエリア";
        case Type::TOWER_CONTROL:
            return "ガチヤグラ";
        case Type::RAINMAKER:
            return "ガチホコバトル";
        case Type::CLAM_BLITZ:
            return "ガチアサリ";
        default:
            return "不明";
        }
    }

    const char *Rule::getEnglishName() const
    {
        switch (type)
        {
        case Type::TURF_WAR:
            return "Turf War";
        case Type::SPLAT_ZONES:
            return "Splat Zones";
        case Type::TOWER_CONTROL:
            return "Tower Control";
        case Type::RAINMAKER:
            return "Rainmaker";
        case Type::CLAM_BLITZ:
            return "Clam Blitz";
        default:
            return "Unknown";
        }
    }

    const char *Rule::getRomajiName() const
    {
        switch (type)
        {
        case Type::TURF_WAR:
            return "Nawabari";
        case Type::SPLAT_ZONES:
            return "Area";
        case Type::TOWER_CONTROL:
            return "Yagura";
        case Type::RAINMAKER:
            return "Hoko";
        case Type::CLAM_BLITZ:
            return "Asari";
        default:
            return "Unknown";
        }
    }

    const char *Rule::getSymbol() const
    {
        switch (type)
        {
        case Type::TURF_WAR:
            return "";
        case Type::SPLAT_ZONES:
            return "[-] ";
        case Type::TOWER_CONTROL:
            return "|^| ";
        case Type::RAINMAKER:
            return "{*} ";
        case Type::CLAM_BLITZ:
            return "(+) ";
        default:
            return "";
        }
    }

    const char *Rule::getDisplayName(bool useRomaji) const
    {
        return useRomaji ? getRomajiName() : getEnglishName();
    }

    uint16_t Rule::getSymbolColor() const
    {
        switch (type)
        {
        case Type::SPLAT_ZONES:
            return 0x07E0; // Green
        case Type::TOWER_CONTROL:
            return 0x15BD; // Blue
        case Type::RAINMAKER:
            return 0xFE60; // Gold/Yellow
        case Type::CLAM_BLITZ:
            return 0xF980; // Red-Orange
        default:
            return 0xFFFF; // White
        }
    }

} // namespace Domain