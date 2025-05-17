#include "BattleType.h"

namespace Domain
{

    const char *BattleType::getJapaneseName() const
    {
        switch (type)
        {
        case Type::REGULAR:
            return "レギュラーマッチ";
        case Type::X_MATCH:
            return "Xマッチ";
        case Type::BANKARA_CHALLENGE:
            return "バンカラマッチ チャレンジ";
        case Type::BANKARA_OPEN:
            return "バンカラマッチ オープン";
        default:
            return "Unknown";
        }
    }

    const char *BattleType::getEnglishName() const
    {
        switch (type)
        {
        case Type::REGULAR:
            return "Regular Battle";
        case Type::X_MATCH:
            return "X Battle";
        case Type::BANKARA_CHALLENGE:
            return "Anarchy Challenge";
        case Type::BANKARA_OPEN:
            return "Anarchy Open";
        default:
            return "Unknown";
        }
    }

    const char *BattleType::getRomajiName() const
    {
        switch (type)
        {
        case Type::REGULAR:
            return "Regular Match";
        case Type::X_MATCH:
            return "X Match";
        case Type::BANKARA_CHALLENGE:
            return "Bankara Challenge";
        case Type::BANKARA_OPEN:
            return "Bankara Open";
        default:
            return "Unknown";
        }
    }

    const char *BattleType::getDisplayName(bool useRomaji) const
    {
        return useRomaji ? getRomajiName() : getEnglishName();
    }

    uint16_t BattleType::getColor() const
    {
        switch (type)
        {
        case Type::REGULAR:
            return 0x7FE0; // Chartreuse (#7FFF00)
        case Type::X_MATCH:
            return 0x3E9C; // MediumTurquoise (#48D1CC)
        case Type::BANKARA_CHALLENGE:
        case Type::BANKARA_OPEN:
            return 0xFD00; // Orange (#FFA500)
        default:
            return 0xFFFF; // White
        }
    }

    const char *BattleType::getCurrentScheduleUrl() const
    {
        switch (type)
        {
        case Type::REGULAR:
            return "https://spla3.yuu26.com/api/regular/now";
        case Type::X_MATCH:
            return "https://spla3.yuu26.com/api/x/now";
        case Type::BANKARA_CHALLENGE:
            return "https://spla3.yuu26.com/api/bankara-challenge/now";
        case Type::BANKARA_OPEN:
            return "https://spla3.yuu26.com/api/bankara-open/now";
        default:
            return "";
        }
    }

    const char *BattleType::getNextScheduleUrl() const
    {
        switch (type)
        {
        case Type::REGULAR:
            return "https://spla3.yuu26.com/api/regular/next";
        case Type::X_MATCH:
            return "https://spla3.yuu26.com/api/x/next";
        case Type::BANKARA_CHALLENGE:
            return "https://spla3.yuu26.com/api/bankara-challenge/next";
        case Type::BANKARA_OPEN:
            return "https://spla3.yuu26.com/api/bankara-open/next";
        default:
            return "";
        }
    }

} // namespace Domain