// Rule.h
// Rule domain model - represents a battle rule in Splatoon3

#ifndef RULE_H
#define RULE_H

#include <Arduino.h>

namespace Domain
{

    // Rule Value Object
    class Rule
    {
    public:
        enum class Type
        {
            TURF_WAR,
            SPLAT_ZONES,
            TOWER_CONTROL,
            RAINMAKER,
            CLAM_BLITZ,
            UNKNOWN
        };

        // デフォルトコンストラクタ - UNKNOWNタイプで初期化
        Rule() : type(Type::UNKNOWN) {}

        // Factory methods
        static Rule turfWar() { return Rule(Type::TURF_WAR); }
        static Rule splatZones() { return Rule(Type::SPLAT_ZONES); }
        static Rule towerControl() { return Rule(Type::TOWER_CONTROL); }
        static Rule rainmaker() { return Rule(Type::RAINMAKER); }
        static Rule clamBlitz() { return Rule(Type::CLAM_BLITZ); }
        static Rule unknown() { return Rule(Type::UNKNOWN); }

        // Factory method from Japanese name
        static Rule fromJapaneseName(const char *japaneseName);

        // Value getters
        Type getType() const { return type; }

        const char *getJapaneseName() const;
        const char *getEnglishName() const;
        const char *getRomajiName() const;
        const char *getSymbol() const;

        // Return appropriate display name based on display mode preference
        const char *getDisplayName(bool useRomaji = true) const;

        // Get symbol color for this rule
        uint16_t getSymbolColor() const;

        // Equality operators
        bool operator==(const Rule &other) const
        {
            return type == other.type;
        }

        bool operator!=(const Rule &other) const
        {
            return !(*this == other);
        }

    private:
        Type type;

        // Private constructor to enforce creation via factory methods
        explicit Rule(Type type) : type(type) {}
    };

} // namespace Domain

#endif // RULE_H