// ScheduleRepository.h
// Repository interface for schedule data access

#ifndef SCHEDULE_REPOSITORY_H
#define SCHEDULE_REPOSITORY_H

#include "../domain/BattleSchedule.h"
#include "../domain/BattleType.h"

namespace Application
{

    // Repository interface for fetching schedule data
    class ScheduleRepository
    {
    public:
        virtual ~ScheduleRepository() = default;

        // Fetch current schedule for a battle type
        virtual Domain::BattleSchedule fetchCurrentSchedule(const Domain::BattleType &battleType) = 0;

        // Fetch next schedule for a battle type
        virtual Domain::BattleSchedule fetchNextSchedule(const Domain::BattleType &battleType) = 0;

        // Update all schedules for all battle types
        virtual void updateAllSchedules() = 0;
    };

} // namespace Application

#endif // SCHEDULE_REPOSITORY_H