// Stage.h
// Stage domain model - represents a battle stage in Splatoon3

#ifndef STAGE_H
#define STAGE_H

#include <Arduino.h>

namespace Domain
{

    // Stage Value Object
    class Stage
    {
    public:
        enum class Type
        {
            SCORCH_GORGE,
            EELTAIL_ALLEY,
            HAGGLEFISH_MARKET,
            UNDERTOW_SPILLWAY,
            UMAMI_RUINS,
            MINCEMEAT_METALWORKS,
            BRINEWATER_SPRINGS,
            BARNACLE_AND_DIME,
            FLOUNDER_HEIGHTS,
            HAMMERHEAD_BRIDGE,
            MUSEUM_DALFONSINO,
            MAHI_MAHI_RESORT,
            INKBLOT_ART_ACADEMY,
            STURGEON_SHIPYARD,
            MAKO_MART,
            WAHOO_WORLD,
            HUMPBACK_PUMP_TRACK,
            MANTA_MARIA,
            CRABLEG_CAPITAL,
            SHIPSHAPE_CARGO_CO,
            BAYSIDE_BOWL,
            BLUEFIN_DEPOT,
            MARLIN_AIRPORT,
            DRAGON_PALACE_TERMINAL,
            UNKNOWN
        };

        // デフォルトコンストラクタ - UNKNOWNタイプで初期化
        Stage() : type(Type::UNKNOWN) {}

        // Factory method from Japanese name
        static Stage fromJapaneseName(const char *japaneseName);

        // Value getters
        Type getType() const { return type; }

        const char *getJapaneseName() const;
        const char *getEnglishName() const;
        const char *getRomajiName() const;

        // Return appropriate display name based on display mode preference
        const char *getDisplayName(bool useRomaji = true) const;

        // Equality operators
        bool operator==(const Stage &other) const
        {
            return type == other.type;
        }

        bool operator!=(const Stage &other) const
        {
            return !(*this == other);
        }

    private:
        Type type;

        // Private constructor to enforce creation via factory methods
        explicit Stage(Type type) : type(type) {}
    };

} // namespace Domain

#endif // STAGE_H