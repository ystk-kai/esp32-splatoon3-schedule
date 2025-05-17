// ScheduleService.h
// Service for schedule data management

#ifndef SCHEDULE_SERVICE_H
#define SCHEDULE_SERVICE_H

#include "../domain/BattleSchedule.h"
#include "../domain/BattleType.h"
#include "ScheduleRepository.h"

namespace Application
{

    // Service for managing schedule data
    class ScheduleService
    {
    public:
        explicit ScheduleService(ScheduleRepository &repository)
            : repository(repository) {}

        // Get current schedule for regular battles
        Domain::BattleSchedule getCurrentRegularSchedule()
        {
            return repository.fetchCurrentSchedule(Domain::BattleType::regular());
        }

        // Get next schedule for regular battles
        Domain::BattleSchedule getNextRegularSchedule()
        {
            return repository.fetchNextSchedule(Domain::BattleType::regular());
        }

        // Get current schedule for X matches
        Domain::BattleSchedule getCurrentXMatchSchedule()
        {
            return repository.fetchCurrentSchedule(Domain::BattleType::xMatch());
        }

        // Get next schedule for X matches
        Domain::BattleSchedule getNextXMatchSchedule()
        {
            return repository.fetchNextSchedule(Domain::BattleType::xMatch());
        }

        // Get current schedule for Bankara Challenge
        Domain::BattleSchedule getCurrentBankaraChallengeSchedule()
        {
            return repository.fetchCurrentSchedule(Domain::BattleType::bankaraChallenge());
        }

        // Get next schedule for Bankara Challenge
        Domain::BattleSchedule getNextBankaraChallengeSchedule()
        {
            return repository.fetchNextSchedule(Domain::BattleType::bankaraChallenge());
        }

        // Get current schedule for Bankara Open
        Domain::BattleSchedule getCurrentBankaraOpenSchedule()
        {
            return repository.fetchCurrentSchedule(Domain::BattleType::bankaraOpen());
        }

        // Get next schedule for Bankara Open
        Domain::BattleSchedule getNextBankaraOpenSchedule()
        {
            return repository.fetchNextSchedule(Domain::BattleType::bankaraOpen());
        }

        // Update all schedules
        void updateAllSchedules()
        {
            repository.updateAllSchedules();
        }

    private:
        ScheduleRepository &repository;
    };

} // namespace Application

#endif // SCHEDULE_SERVICE_H