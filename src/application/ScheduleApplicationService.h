// ScheduleApplicationService.h
// Main application service for coordinating schedule display

#ifndef SCHEDULE_APPLICATION_SERVICE_H
#define SCHEDULE_APPLICATION_SERVICE_H

#include "ScheduleService.h"
#include "DisplayService.h"
#include "NetworkService.h"
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

                // Initial data fetch
                displayService.showLoadingMessage("Fetching data...");
                scheduleService.updateAllSchedules();

                // Update display
                updateDisplay();
            }

            return connected;
        }

        // Update all schedule data and refresh display
        void updateAllData()
        {
            scheduleService.updateAllSchedules();
            updateDisplay();
        }

        // Update just the current time display (bottom info bar)
        void updateTimeDisplay()
        {
            char currentDateTime[17];
            char lastUpdateTime[16];

            if (networkService.getCurrentDateTime(currentDateTime, sizeof(currentDateTime)) &&
                networkService.getLastUpdateTime(lastUpdateTime, sizeof(lastUpdateTime)))
            {
                displayService.updateBottomInfo(currentDateTime, lastUpdateTime);
            }
        }

        // Update display with current schedule data
        void updateDisplay()
        {
            // Get all schedule data
            auto regularSchedule = scheduleService.getCurrentRegularSchedule();
            auto regularNextSchedule = scheduleService.getNextRegularSchedule();
            auto xMatchSchedule = scheduleService.getCurrentXMatchSchedule();
            auto xMatchNextSchedule = scheduleService.getNextXMatchSchedule();
            auto bankaraChallengeSchedule = scheduleService.getCurrentBankaraChallengeSchedule();
            auto bankaraChallengeNextSchedule = scheduleService.getNextBankaraChallengeSchedule();
            auto bankaraOpenSchedule = scheduleService.getCurrentBankaraOpenSchedule();
            auto bankaraOpenNextSchedule = scheduleService.getNextBankaraOpenSchedule();

            // Get current time info
            char currentDateTime[17];
            char lastUpdateTime[16];

            networkService.getCurrentDateTime(currentDateTime, sizeof(currentDateTime));
            networkService.getLastUpdateTime(lastUpdateTime, sizeof(lastUpdateTime));

            // Update display
            displayService.updateScreen(
                regularSchedule,
                regularNextSchedule,
                xMatchSchedule,
                xMatchNextSchedule,
                bankaraChallengeSchedule,
                bankaraChallengeNextSchedule,
                bankaraOpenSchedule,
                bankaraOpenNextSchedule,
                currentDateTime,
                lastUpdateTime,
                displaySettings);
        }

        // Change display settings
        void setDisplaySettings(const Domain::DisplaySettings &settings)
        {
            displaySettings = settings;
            updateDisplay();
        }

        // Get current display settings
        const Domain::DisplaySettings &getDisplaySettings() const
        {
            return displaySettings;
        }

    private:
        ScheduleService &scheduleService;
        DisplayService &displayService;
        NetworkService &networkService;
        Domain::DisplaySettings displaySettings;
    };

} // namespace Application

#endif // SCHEDULE_APPLICATION_SERVICE_H