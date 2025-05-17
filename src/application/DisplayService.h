// DisplayService.h
// Service interface for display output

#ifndef DISPLAY_SERVICE_H
#define DISPLAY_SERVICE_H

#include "../domain/BattleSchedule.h"
#include "../domain/DisplaySettings.h"

namespace Application
{

    // Service interface for display output operations
    class DisplayService
    {
    public:
        virtual ~DisplayService() = default;

        // Initialize display hardware
        virtual void initialize() = 0;

        // Set backlight brightness level (0-255)
        virtual void setBacklight(uint8_t brightness) = 0;

        // Clear the display screen
        virtual void clearScreen() = 0;

        // Update the entire screen with all schedule data
        virtual void updateScreen(
            const Domain::BattleSchedule &regularSchedule,
            const Domain::BattleSchedule &regularNextSchedule,
            const Domain::BattleSchedule &xMatchSchedule,
            const Domain::BattleSchedule &xMatchNextSchedule,
            const Domain::BattleSchedule &bankaraChallengeSchedule,
            const Domain::BattleSchedule &bankaraChallengeNextSchedule,
            const Domain::BattleSchedule &bankaraOpenSchedule,
            const Domain::BattleSchedule &bankaraOpenNextSchedule,
            const char *currentDateTime,
            const char *lastUpdateTime,
            const Domain::DisplaySettings &displaySettings) = 0;

        // Update just the bottom info bar with date/time and update time
        virtual void updateBottomInfo(
            const char *currentDateTime,
            const char *lastUpdateTime) = 0;

        // Display startup screen
        virtual void showStartupScreen() = 0;

        // Show connection status screen
        virtual void showConnectionStatus(bool connected, const char *statusMessage) = 0;

        // Show loading message
        virtual void showLoadingMessage(const char *message) = 0;

        // Update time display at bottom of screen
        virtual void updateTimeDisplay() = 0;

        // Update display with the specified schedules
        virtual void updateDisplay(
            const Domain::BattleSchedule &regularSchedule,
            const Domain::BattleSchedule &xSchedule,
            const Domain::BattleSchedule &anarchyChallengeSchedule,
            const Domain::BattleSchedule &anarchyOpenSchedule,
            const Domain::BattleSchedule &regularNextSchedule,
            const Domain::BattleSchedule &xNextSchedule,
            const Domain::BattleSchedule &anarchyChallengeNextSchedule,
            const Domain::BattleSchedule &anarchyOpenNextSchedule,
            const Domain::DisplaySettings &displaySettings) = 0;
    };

} // namespace Application

#endif // DISPLAY_SERVICE_H