// BattleSchedule.h
// BattleSchedule domain model - represents a battle schedule time slot in Splatoon3

#ifndef BATTLE_SCHEDULE_H
#define BATTLE_SCHEDULE_H

#include <Arduino.h>
#include "Rule.h"
#include "Stage.h"
#include "BattleType.h"

namespace Domain
{

    // BattleSchedule Entity - represents a scheduled battle timeslot with stages and rule
    class BattleSchedule
    {
    public:
        // Create a battle schedule
        static BattleSchedule create(
            const BattleType &battleType,
            const Rule &rule,
            const Stage &stage1,
            const Stage &stage2,
            const char *startTime,
            const char *endTime)
        {
            return BattleSchedule(battleType, rule, stage1, stage2, startTime, endTime);
        }

        // Creates an empty/invalid schedule
        static BattleSchedule createEmpty(const BattleType &battleType)
        {
            return BattleSchedule(battleType);
        }

        // Default constructor - creates an empty/invalid schedule for Regular Battle
        BattleSchedule() : battleType(BattleType::regular()),
                           rule(Rule::unknown()),
                           stage1(Stage::fromJapaneseName("不明")),
                           stage2(Stage::fromJapaneseName("不明")),
                           valid(false)
        {
            strcpy(startTime, "--:--");
            strcpy(endTime, "--:--");
        }

        // Value getters
        const BattleType &getBattleType() const { return battleType; }
        const Rule &getRule() const { return rule; }
        const Stage &getStage1() const { return stage1; }
        const Stage &getStage2() const { return stage2; }

        // Get the start and end time strings
        const char *getStartTime() const { return startTime; }
        const char *getEndTime() const { return endTime; }

        // Check if this schedule is valid (has been properly populated)
        bool isValid() const { return valid; }

    private:
        BattleType battleType;
        Rule rule;
        Stage stage1;
        Stage stage2;
        char startTime[6]; // "HH:MM\0"
        char endTime[6];   // "HH:MM\0"
        bool valid;

        // Private constructor to enforce creation via factory methods
        BattleSchedule(
            const BattleType &battleType,
            const Rule &rule,
            const Stage &stage1,
            const Stage &stage2,
            const char *startTimeStr,
            const char *endTimeStr) : battleType(battleType),
                                      rule(rule),
                                      stage1(stage1),
                                      stage2(stage2),
                                      valid(true)
        {
            strncpy(startTime, startTimeStr, sizeof(startTime) - 1);
            startTime[sizeof(startTime) - 1] = '\0';

            strncpy(endTime, endTimeStr, sizeof(endTime) - 1);
            endTime[sizeof(endTime) - 1] = '\0';
        }

        // Private constructor for invalid/empty schedule
        explicit BattleSchedule(const BattleType &battleType) : battleType(battleType),
                                                                rule(Rule::unknown()),
                                                                stage1(Stage::fromJapaneseName("不明")),
                                                                stage2(Stage::fromJapaneseName("不明")),
                                                                valid(false)
        {
            strcpy(startTime, "--:--");
            strcpy(endTime, "--:--");
        }
    };

} // namespace Domain

#endif // BATTLE_SCHEDULE_H