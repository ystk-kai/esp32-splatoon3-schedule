// DisplaySettings.h
// DisplaySettings domain model - represents display preferences for the application

#ifndef DISPLAY_SETTINGS_H
#define DISPLAY_SETTINGS_H

namespace Domain
{

    // DisplaySettings Value Object - immutable configuration for display preferences
    class DisplaySettings
    {
    public:
        // Factory method with default settings
        static DisplaySettings createDefault()
        {
            return DisplaySettings(true, true, true);
        }

        // Factory method with custom settings
        static DisplaySettings create(
            bool useRomajiForBattleType,
            bool useRomajiForRule,
            bool useRomajiForStage)
        {
            return DisplaySettings(useRomajiForBattleType, useRomajiForRule, useRomajiForStage);
        }

        // Getters
        bool isUseRomajiForBattleType() const { return useRomajiForBattleType; }
        bool isUseRomajiForRule() const { return useRomajiForRule; }
        bool isUseRomajiForStage() const { return useRomajiForStage; }

        // Create a copy with modified settings
        DisplaySettings withRomajiForBattleType(bool useRomaji) const
        {
            return DisplaySettings(useRomaji, useRomajiForRule, useRomajiForStage);
        }

        DisplaySettings withRomajiForRule(bool useRomaji) const
        {
            return DisplaySettings(useRomajiForBattleType, useRomaji, useRomajiForStage);
        }

        DisplaySettings withRomajiForStage(bool useRomaji) const
        {
            return DisplaySettings(useRomajiForBattleType, useRomajiForRule, useRomaji);
        }

        // Equality operators
        bool operator==(const DisplaySettings &other) const
        {
            return useRomajiForBattleType == other.useRomajiForBattleType &&
                   useRomajiForRule == other.useRomajiForRule &&
                   useRomajiForStage == other.useRomajiForStage;
        }

        bool operator!=(const DisplaySettings &other) const
        {
            return !(*this == other);
        }

    private:
        bool useRomajiForBattleType;
        bool useRomajiForRule;
        bool useRomajiForStage;

        // Private constructor to enforce creation via factory methods
        DisplaySettings(
            bool useRomajiForBattleType,
            bool useRomajiForRule,
            bool useRomajiForStage) : useRomajiForBattleType(useRomajiForBattleType),
                                      useRomajiForRule(useRomajiForRule),
                                      useRomajiForStage(useRomajiForStage)
        {
        }
    };

} // namespace Domain

#endif // DISPLAY_SETTINGS_H