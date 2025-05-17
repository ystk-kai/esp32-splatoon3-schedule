#include "Stage.h"
#include <string.h>

namespace Domain
{

    Stage Stage::fromJapaneseName(const char *japaneseName)
    {
        if (strcmp(japaneseName, "ユノハナ大渓谷") == 0)
        {
            return Stage(Type::SCORCH_GORGE);
        }
        else if (strcmp(japaneseName, "ゴンズイ地区") == 0)
        {
            return Stage(Type::EELTAIL_ALLEY);
        }
        else if (strcmp(japaneseName, "ヤガラ市場") == 0)
        {
            return Stage(Type::HAGGLEFISH_MARKET);
        }
        else if (strcmp(japaneseName, "マテガイ放水路") == 0)
        {
            return Stage(Type::UNDERTOW_SPILLWAY);
        }
        else if (strcmp(japaneseName, "ナンプラー遺跡") == 0)
        {
            return Stage(Type::UMAMI_RUINS);
        }
        else if (strcmp(japaneseName, "ナメロウ金属") == 0)
        {
            return Stage(Type::MINCEMEAT_METALWORKS);
        }
        else if (strcmp(japaneseName, "クサヤ温泉") == 0)
        {
            return Stage(Type::BRINEWATER_SPRINGS);
        }
        else if (strcmp(japaneseName, "タラポートショッピングパーク") == 0)
        {
            return Stage(Type::BARNACLE_AND_DIME);
        }
        else if (strcmp(japaneseName, "ヒラメが丘団地") == 0)
        {
            return Stage(Type::FLOUNDER_HEIGHTS);
        }
        else if (strcmp(japaneseName, "マサバ海峡大橋") == 0)
        {
            return Stage(Type::HAMMERHEAD_BRIDGE);
        }
        else if (strcmp(japaneseName, "キンメダイ美術館") == 0)
        {
            return Stage(Type::MUSEUM_DALFONSINO);
        }
        else if (strcmp(japaneseName, "マヒマヒリゾート＆スパ") == 0)
        {
            return Stage(Type::MAHI_MAHI_RESORT);
        }
        else if (strcmp(japaneseName, "海女美術大学") == 0)
        {
            return Stage(Type::INKBLOT_ART_ACADEMY);
        }
        else if (strcmp(japaneseName, "チョウザメ造船") == 0)
        {
            return Stage(Type::STURGEON_SHIPYARD);
        }
        else if (strcmp(japaneseName, "ザトウマーケット") == 0)
        {
            return Stage(Type::MAKO_MART);
        }
        else if (strcmp(japaneseName, "スメーシーワールド") == 0)
        {
            return Stage(Type::WAHOO_WORLD);
        }
        else if (strcmp(japaneseName, "コンブトラック") == 0)
        {
            return Stage(Type::HUMPBACK_PUMP_TRACK);
        }
        else if (strcmp(japaneseName, "マンタマリア号") == 0)
        {
            return Stage(Type::MANTA_MARIA);
        }
        else if (strcmp(japaneseName, "タカアシ経済特区") == 0)
        {
            return Stage(Type::CRABLEG_CAPITAL);
        }
        else if (strcmp(japaneseName, "オヒョウ海運") == 0)
        {
            return Stage(Type::SHIPSHAPE_CARGO_CO);
        }
        else if (strcmp(japaneseName, "バイガイ亭") == 0)
        {
            return Stage(Type::BAYSIDE_BOWL);
        }
        else if (strcmp(japaneseName, "ネギトロ炭鉱") == 0)
        {
            return Stage(Type::BLUEFIN_DEPOT);
        }
        else if (strcmp(japaneseName, "カジキ空港") == 0)
        {
            return Stage(Type::MARLIN_AIRPORT);
        }
        else if (strcmp(japaneseName, "リュウグウターミナル") == 0)
        {
            return Stage(Type::DRAGON_PALACE_TERMINAL);
        }
        else
        {
            return Stage(Type::UNKNOWN);
        }
    }

    const char *Stage::getJapaneseName() const
    {
        switch (type)
        {
        case Type::SCORCH_GORGE:
            return "ユノハナ大渓谷";
        case Type::EELTAIL_ALLEY:
            return "ゴンズイ地区";
        case Type::HAGGLEFISH_MARKET:
            return "ヤガラ市場";
        case Type::UNDERTOW_SPILLWAY:
            return "マテガイ放水路";
        case Type::UMAMI_RUINS:
            return "ナンプラー遺跡";
        case Type::MINCEMEAT_METALWORKS:
            return "ナメロウ金属";
        case Type::BRINEWATER_SPRINGS:
            return "クサヤ温泉";
        case Type::BARNACLE_AND_DIME:
            return "タラポートショッピングパーク";
        case Type::FLOUNDER_HEIGHTS:
            return "ヒラメが丘団地";
        case Type::HAMMERHEAD_BRIDGE:
            return "マサバ海峡大橋";
        case Type::MUSEUM_DALFONSINO:
            return "キンメダイ美術館";
        case Type::MAHI_MAHI_RESORT:
            return "マヒマヒリゾート＆スパ";
        case Type::INKBLOT_ART_ACADEMY:
            return "海女美術大学";
        case Type::STURGEON_SHIPYARD:
            return "チョウザメ造船";
        case Type::MAKO_MART:
            return "ザトウマーケット";
        case Type::WAHOO_WORLD:
            return "スメーシーワールド";
        case Type::HUMPBACK_PUMP_TRACK:
            return "コンブトラック";
        case Type::MANTA_MARIA:
            return "マンタマリア号";
        case Type::CRABLEG_CAPITAL:
            return "タカアシ経済特区";
        case Type::SHIPSHAPE_CARGO_CO:
            return "オヒョウ海運";
        case Type::BAYSIDE_BOWL:
            return "バイガイ亭";
        case Type::BLUEFIN_DEPOT:
            return "ネギトロ炭鉱";
        case Type::MARLIN_AIRPORT:
            return "カジキ空港";
        case Type::DRAGON_PALACE_TERMINAL:
            return "リュウグウターミナル";
        default:
            return "不明";
        }
    }

    const char *Stage::getEnglishName() const
    {
        switch (type)
        {
        case Type::SCORCH_GORGE:
            return "Scorch Gorge";
        case Type::EELTAIL_ALLEY:
            return "Eeltail Alley";
        case Type::HAGGLEFISH_MARKET:
            return "Hagglefish Market";
        case Type::UNDERTOW_SPILLWAY:
            return "Undertow Spillway";
        case Type::UMAMI_RUINS:
            return "Um'ami Ruins";
        case Type::MINCEMEAT_METALWORKS:
            return "Mincemeat Metalworks";
        case Type::BRINEWATER_SPRINGS:
            return "Brinewater Springs";
        case Type::BARNACLE_AND_DIME:
            return "Barnacle & Dime";
        case Type::FLOUNDER_HEIGHTS:
            return "Flounder Heights";
        case Type::HAMMERHEAD_BRIDGE:
            return "Hammerhead Bridge";
        case Type::MUSEUM_DALFONSINO:
            return "Museum d'Alfonsino";
        case Type::MAHI_MAHI_RESORT:
            return "Mahi-Mahi Resort";
        case Type::INKBLOT_ART_ACADEMY:
            return "Inkblot Art Academy";
        case Type::STURGEON_SHIPYARD:
            return "Sturgeon Shipyard";
        case Type::MAKO_MART:
            return "Mako Mart";
        case Type::WAHOO_WORLD:
            return "Wahoo World";
        case Type::HUMPBACK_PUMP_TRACK:
            return "Humpback Pump Track";
        case Type::MANTA_MARIA:
            return "Manta Maria";
        case Type::CRABLEG_CAPITAL:
            return "Crableg Capital";
        case Type::SHIPSHAPE_CARGO_CO:
            return "Shipshape Cargo Co.";
        case Type::BAYSIDE_BOWL:
            return "Bayside Bowl";
        case Type::BLUEFIN_DEPOT:
            return "Bluefin Depot";
        case Type::MARLIN_AIRPORT:
            return "Marlin Airport";
        case Type::DRAGON_PALACE_TERMINAL:
            return "Dragon Palace Terminal";
        default:
            return "Unknown";
        }
    }

    const char *Stage::getRomajiName() const
    {
        switch (type)
        {
        case Type::SCORCH_GORGE:
            return "Yunohana";
        case Type::EELTAIL_ALLEY:
            return "Gonzui";
        case Type::HAGGLEFISH_MARKET:
            return "Yagara";
        case Type::UNDERTOW_SPILLWAY:
            return "Mategai";
        case Type::UMAMI_RUINS:
            return "Nampla";
        case Type::MINCEMEAT_METALWORKS:
            return "Namero";
        case Type::BRINEWATER_SPRINGS:
            return "Kusaya";
        case Type::BARNACLE_AND_DIME:
            return "Taraport";
        case Type::FLOUNDER_HEIGHTS:
            return "Hirame";
        case Type::HAMMERHEAD_BRIDGE:
            return "Masaba";
        case Type::MUSEUM_DALFONSINO:
            return "Kinmedai";
        case Type::MAHI_MAHI_RESORT:
            return "Mahimahi";
        case Type::INKBLOT_ART_ACADEMY:
            return "Amabi";
        case Type::STURGEON_SHIPYARD:
            return "Chouzame";
        case Type::MAKO_MART:
            return "Zatou";
        case Type::WAHOO_WORLD:
            return "Sume-shi";
        case Type::HUMPBACK_PUMP_TRACK:
            return "Kombu";
        case Type::MANTA_MARIA:
            return "Mantamaria";
        case Type::CRABLEG_CAPITAL:
            return "Takaashi";
        case Type::SHIPSHAPE_CARGO_CO:
            return "Ohyou";
        case Type::BAYSIDE_BOWL:
            return "Baigai";
        case Type::BLUEFIN_DEPOT:
            return "Negitoro";
        case Type::MARLIN_AIRPORT:
            return "Kajiki";
        case Type::DRAGON_PALACE_TERMINAL:
            return "Ryuuguu";
        default:
            return "Unknown";
        }
    }

    const char *Stage::getDisplayName(bool useRomaji) const
    {
        return useRomaji ? getRomajiName() : getEnglishName();
    }

} // namespace Domain