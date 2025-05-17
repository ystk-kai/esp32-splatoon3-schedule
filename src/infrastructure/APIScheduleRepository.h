// APIScheduleRepository.h
// Repository implementation for fetching schedule data from API

#ifndef API_SCHEDULE_REPOSITORY_H
#define API_SCHEDULE_REPOSITORY_H

#include <ArduinoJson.h>
#include "../application/ScheduleRepository.h"
#include "../application/NetworkService.h"
#include "../domain/BattleSchedule.h"

namespace Infrastructure
{

    // Repository implementation for fetching schedule data from external API
    class APIScheduleRepository : public Application::ScheduleRepository
    {
    public:
        explicit APIScheduleRepository(Application::NetworkService &networkService)
            : networkService(networkService)
        {
            // 初期化時にスケジュールオブジェクトを生成
            initializeSchedules();
        }

        // デストラクタでメモリを解放
        ~APIScheduleRepository() override = default;

        // Fetch current schedule for a battle type
        Domain::BattleSchedule fetchCurrentSchedule(const Domain::BattleType &battleType) override;

        // Fetch next schedule for a battle type
        Domain::BattleSchedule fetchNextSchedule(const Domain::BattleType &battleType) override;

        // Update all schedules for all battle types
        void updateAllSchedules() override;

    private:
        Application::NetworkService &networkService;

        // スケジュールデータ（各バトルタイプごとに現在と次回のスケジュール）
        // メンバ変数の宣言のみ - 初期化はinitializeSchedules()で行う
        Domain::BattleSchedule regularSchedule;
        Domain::BattleSchedule xMatchSchedule;
        Domain::BattleSchedule bankaraChallengeSchedule;
        Domain::BattleSchedule bankaraOpenSchedule;
        Domain::BattleSchedule regularNextSchedule;
        Domain::BattleSchedule xMatchNextSchedule;
        Domain::BattleSchedule bankaraChallengeNextSchedule;
        Domain::BattleSchedule bankaraOpenNextSchedule;

        // 初期スケジュールを設定
        void initializeSchedules();

        // Update schedule data for a specific battle type
        void updateScheduleForBattleType(const Domain::BattleType &battleType);

        // Parse API response to create a BattleSchedule
        Domain::BattleSchedule parseScheduleFromJson(
            const String &jsonResponse,
            const Domain::BattleType &battleType,
            bool isCurrentSchedule);
    };

} // namespace Infrastructure

#endif // API_SCHEDULE_REPOSITORY_H