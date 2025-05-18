// ScheduleApplicationService.h
// Main application service for coordinating schedule display

#ifndef SCHEDULE_APPLICATION_SERVICE_H
#define SCHEDULE_APPLICATION_SERVICE_H

#include "ScheduleService.h"
#include "DisplayService.h"
#include "NetworkService.h"
#include "WiFiService.h"
#include "../domain/DisplaySettings.h"

namespace Application
{

    // Main application service that coordinates the schedule display logic
    class ScheduleApplicationService
    {
    public:
        ScheduleApplicationService(
            ScheduleService &scheduleService,
            DisplayService &displayService,
            NetworkService &networkService) : scheduleService(scheduleService),
                                              displayService(displayService),
                                              networkService(networkService),
                                              displaySettings(Domain::DisplaySettings::createDefault()) {}

        // Initialize the application
        bool initialize(const char *ssid, const char *password)
        {
            // Show startup screen
            displayService.initialize();
            displayService.showStartupScreen();

            // Connect to WiFi
            bool connected = networkService.connect(ssid, password);

            // Show connection status
            displayService.showConnectionStatus(
                connected,
                connected ? "Connected!" : "Failed!");

            if (connected)
            {
                // Configure time service
                networkService.configureTimeService();

                // Initial data fetch - 初回データ取得なのでフルスクリーン表示
                displayService.showLoadingMessage("Fetching data...", false);
                scheduleService.updateAllSchedules();

                // Update display
                updateDisplay();
            }

            return connected;
        }

        // Initialize the application with WiFiService
        bool initialize(WiFiService &wifiService)
        {
            // Show startup screen
            displayService.initialize();
            displayService.showStartupScreen();

            // Wait for WiFi connection
            while (!wifiService.isConnected())
            {
                delay(100);
                // If captive portal is active, show portal info
                if (wifiService.isCaptivePortalActive())
                {
                    displayService.showConnectionStatus(
                        false,
                        "WiFi Portal Active");
                    return false;
                }
            }

            // Show connection status
            displayService.showConnectionStatus(
                true,
                "Connected!");

            // Configure time service
            networkService.configureTimeService();

            // Initial data fetch - 初回データ取得なのでフルスクリーン表示
            displayService.showLoadingMessage("Fetching data...", false);
            scheduleService.updateAllSchedules();

            // Update display
            updateDisplay();

            return true;
        }

        // Update all schedule data and refresh display
        void updateAllData()
        {
            // アプリケーションが初期化済みの場合はバックグラウンド更新を使用
            displayService.showLoadingMessage("Updating data...", true);

            // Update all schedule data
            scheduleService.updateAllSchedules();

            // Update display
            updateDisplay();
        }

        // Update only the time display (for more efficient updates)
        void updateTimeDisplay()
        {
            displayService.updateTimeDisplay();
        }

        // Update display with current schedule data
        void updateDisplay()
        {
            // Get current schedules
            Domain::BattleSchedule regularSchedule = scheduleService.getCurrentRegularSchedule();
            Domain::BattleSchedule xSchedule = scheduleService.getCurrentXMatchSchedule();
            Domain::BattleSchedule anarchyChallengeSchedule = scheduleService.getCurrentBankaraChallengeSchedule();
            Domain::BattleSchedule anarchyOpenSchedule = scheduleService.getCurrentBankaraOpenSchedule();

            // Get next schedules
            Domain::BattleSchedule regularNextSchedule = scheduleService.getNextRegularSchedule();
            Domain::BattleSchedule xNextSchedule = scheduleService.getNextXMatchSchedule();
            Domain::BattleSchedule anarchyChallengeNextSchedule = scheduleService.getNextBankaraChallengeSchedule();
            Domain::BattleSchedule anarchyOpenNextSchedule = scheduleService.getNextBankaraOpenSchedule();

            // Update display with schedules
            displayService.updateDisplay(
                regularSchedule,
                xSchedule,
                anarchyChallengeSchedule,
                anarchyOpenSchedule,
                regularNextSchedule,
                xNextSchedule,
                anarchyChallengeNextSchedule,
                anarchyOpenNextSchedule,
                displaySettings);
        }

        // Set display settings
        void setDisplaySettings(const Domain::DisplaySettings &settings)
        {
            displaySettings = settings;
        }

    private:
        ScheduleService &scheduleService;
        DisplayService &displayService;
        NetworkService &networkService;
        Domain::DisplaySettings displaySettings;
    };

} // namespace Application

#endif // SCHEDULE_APPLICATION_SERVICE_H