// BattleType.h
// BattleType domain model - represents a type of battle in Splatoon3

#ifndef BATTLE_TYPE_H
#define BATTLE_TYPE_H

#include <Arduino.h>

namespace Domain
{

    // BattleType Value Object
    class BattleType
    {
    public:
        enum class Type
        {
            REGULAR,
            X_MATCH,
            BANKARA_CHALLENGE,
            BANKARA_OPEN
        };

        // デフォルトコンストラクタ - REGULARタイプで初期化
        BattleType() : type(Type::REGULAR) {}

        // Factory methods
        static BattleType regular() { return BattleType(Type::REGULAR); }
        static BattleType xMatch() { return BattleType(Type::X_MATCH); }
        static BattleType bankaraChallenge() { return BattleType(Type::BANKARA_CHALLENGE); }
        static BattleType bankaraOpen() { return BattleType(Type::BANKARA_OPEN); }

        // Value getters
        Type getType() const { return type; }

        const char *getJapaneseName() const;
        const char *getEnglishName() const;
        const char *getRomajiName() const;

        // Return appropriate display name based on display mode preference
        const char *getDisplayName(bool useRomaji = true) const;

        // Return color code for this battle type
        uint16_t getColor() const;

        // Return API URL for current schedule
        const char *getCurrentScheduleUrl() const;

        // Return API URL for next schedule
        const char *getNextScheduleUrl() const;

        // Equality operators
        bool operator==(const BattleType &other) const
        {
            return type == other.type;
        }

        bool operator!=(const BattleType &other) const
        {
            return !(*this == other);
        }

    private:
        Type type;

        // Private constructor to enforce creation via factory methods
        explicit BattleType(Type type) : type(type) {}
    };

} // namespace Domain

#endif // BATTLE_TYPE_H