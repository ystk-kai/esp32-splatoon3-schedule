// TFTDisplayService.h
// TFT_eSPI based implementation of DisplayService

#ifndef TFT_DISPLAY_SERVICE_H
#define TFT_DISPLAY_SERVICE_H

#include <TFT_eSPI.h>
#include "../application/DisplayService.h"

namespace Infrastructure
{

    // TFT_eSPI based implementation of DisplayService
    class TFTDisplayService : public Application::DisplayService
    {
    public:
        TFTDisplayService(uint8_t backlightPin, uint8_t pwmChannel)
            : backlightPin(backlightPin), pwmChannel(pwmChannel) {}

        // Initialize display hardware
        void initialize() override
        {
            // Setup backlight PWM
            ledcSetup(pwmChannel, 5000, 8);
            ledcAttachPin(backlightPin, pwmChannel);

            // TFT initialization
            tft.init();
            tft.setRotation(1); // Landscape mode
        }

        // Set backlight brightness level (0-255)
        void setBacklight(uint8_t brightness) override
        {
            ledcWrite(pwmChannel, brightness);
        }

        // Clear the display screen
        void clearScreen() override
        {
            tft.fillScreen(TFT_BLACK);
        }

        // Update the entire screen with all schedule data
        void updateScreen(
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
            const Domain::DisplaySettings &displaySettings) override;

        // Update just the bottom info bar with date/time and update time
        void updateBottomInfo(
            const char *currentDateTime,
            const char *lastUpdateTime) override;

        // Display startup screen
        void showStartupScreen() override;

        // Show connection status screen
        void showConnectionStatus(bool connected, const char *statusMessage) override;

        // Show loading message
        void showLoadingMessage(const char *message) override;

    private:
        TFT_eSPI tft;
        uint8_t backlightPin;
        uint8_t pwmChannel;

        // Constants
        static constexpr int SCREEN_WIDTH = 320;
        static constexpr int SCREEN_HEIGHT = 240;
        static constexpr int QUADRANT_WIDTH = SCREEN_WIDTH / 2;
        static constexpr int QUADRANT_HEIGHT = SCREEN_HEIGHT / 2;

        // Helper method to draw a battle quadrant
        void drawBattleQuadrant(
            int x,
            int y,
            const Domain::BattleSchedule &current,
            const Domain::BattleSchedule &next,
            const Domain::DisplaySettings &displaySettings);
    };

} // namespace Infrastructure

#endif // TFT_DISPLAY_SERVICE_H