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
            : backlightPin(backlightPin), pwmChannel(pwmChannel), isInverted(false)
        {
            Serial.print("TFTDisplayService constructed. Initial invert state: ");
            Serial.println(isInverted ? "true" : "false");
        }

        // Initialize display hardware
        void initialize() override
        {
            // Setup backlight PWM
            ledcSetup(pwmChannel, 5000, 8);
            ledcAttachPin(backlightPin, pwmChannel);

            // TFT initialization
            tft.init();
            tft.setRotation(1); // Landscape mode

            // 初期化時に前回の反転状態を適用
            Serial.print("TFTDisplayService::initialize - Setting initial invert state to: ");
            Serial.println(isInverted ? "true" : "false");
            tft.invertDisplay(isInverted);
        }

        // 表示状態をリセットする
        void resetDisplayState()
        {
            // 静的変数をリセットするために使用
            isFirstStatusCall = true;
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

        // 画面の色を反転する
        void invertDisplay(bool invert) override
        {
            Serial.print("TFTDisplayService::invertDisplay called with: ");
            Serial.println(invert ? "true" : "false");

            // 状態を更新
            isInverted = invert;

            // 反転状態を適用
            tft.invertDisplay(invert);

            Serial.print("Display invert state is now: ");
            Serial.println(isInverted ? "true" : "false");
        }

        // 画面が反転状態かどうかを切り替える
        void toggleInvertDisplay() override
        {
            // 現在の状態を反転
            isInverted = !isInverted;

            Serial.print("TFTDisplayService::toggleInvertDisplay - Toggle invert to: ");
            Serial.println(isInverted ? "true" : "false");

            // 反転状態を適用
            tft.invertDisplay(isInverted);
        }

        // 画面の反転状態を取得する
        bool getInvertStatus() override
        {
            return isInverted;
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

        // Show loading message with background update option
        void showLoadingMessage(const char *message, bool backgroundUpdate) override;

        // Show device information on screen
        void showDeviceInfo() override;

        // Update time display at bottom of screen
        void updateTimeDisplay() override
        {
            // 現在時刻を取得して表示を更新
            char currentDateTime[64];
            char lastUpdateTime[64];

            // 現在時刻のフォーマット（年月日と時分までを表示、秒は省略）
            time_t now;
            struct tm timeinfo;
            time(&now);
            localtime_r(&now, &timeinfo);
            strftime(currentDateTime, sizeof(currentDateTime), "%Y-%m-%d %H:%M", &timeinfo);

            // 最終更新時刻は現在時刻を5分単位に丸める（例：23:17→23:15）
            struct tm updateTime = timeinfo;
            // 分を5の倍数に丸める（0, 5, 10, 15...）
            updateTime.tm_min = (updateTime.tm_min / 5) * 5;
            updateTime.tm_sec = 0;

            // 時刻のみの形式でフォーマット（HH:MM）
            time_t roundedTime = mktime(&updateTime);
            strftime(lastUpdateTime, sizeof(lastUpdateTime), "%H:%M", &updateTime);

            // 下部情報バーを更新
            updateBottomInfo(currentDateTime, lastUpdateTime);
        }

        // Update display with the specified schedules
        void updateDisplay(
            const Domain::BattleSchedule &regularSchedule,
            const Domain::BattleSchedule &xSchedule,
            const Domain::BattleSchedule &anarchyChallengeSchedule,
            const Domain::BattleSchedule &anarchyOpenSchedule,
            const Domain::BattleSchedule &regularNextSchedule,
            const Domain::BattleSchedule &xNextSchedule,
            const Domain::BattleSchedule &anarchyChallengeNextSchedule,
            const Domain::BattleSchedule &anarchyOpenNextSchedule,
            const Domain::DisplaySettings &displaySettings) override
        {
            char currentDateTime[64];
            char lastUpdateTime[64];

            // 現在時刻のフォーマット（年月日と時分までを表示、秒は省略）
            time_t now;
            struct tm timeinfo;
            time(&now);
            localtime_r(&now, &timeinfo);
            strftime(currentDateTime, sizeof(currentDateTime), "%Y-%m-%d %H:%M", &timeinfo);

            // 最終更新時刻は現在時刻を5分単位に丸める（例：23:17→23:15）
            struct tm updateTime = timeinfo;
            // 分を5の倍数に丸める（0, 5, 10, 15...）
            updateTime.tm_min = (updateTime.tm_min / 5) * 5;
            updateTime.tm_sec = 0;

            // 時刻のみの形式でフォーマット（HH:MM）
            time_t roundedTime = mktime(&updateTime);
            strftime(lastUpdateTime, sizeof(lastUpdateTime), "%H:%M", &updateTime);

            // 画面全体を更新（次の予定情報も含めて表示）
            updateScreen(
                regularSchedule,
                regularNextSchedule,
                xSchedule,
                xNextSchedule,
                anarchyChallengeSchedule,
                anarchyChallengeNextSchedule,
                anarchyOpenSchedule,
                anarchyOpenNextSchedule,
                currentDateTime,
                lastUpdateTime,
                displaySettings);
        }

    private:
        TFT_eSPI tft;
        uint8_t backlightPin;
        uint8_t pwmChannel;
        bool isInverted; // 画面反転状態の管理用

        // showConnectionStatusメソッドの状態管理用
        static bool isFirstStatusCall;

        // Colors for Splatoon3 theme
        static constexpr uint16_t SPLATOON_BLUE = 0x04FF;   // 青インク色相当
        static constexpr uint16_t SPLATOON_ORANGE = 0xFD00; // オレンジインク色相当
        static constexpr uint16_t SPLATOON_GREEN = 0x07E0;  // 緑インク色相当
        static constexpr uint16_t SPLATOON_PINK = 0xF81F;   // ピンクインク色相当
        static constexpr uint16_t SPLATOON_YELLOW = 0xFFE0; // 黄色インク色相当
        static constexpr uint16_t SPLATOON_NAVY = 0x000F;   // 濃い青（背景用）
        static constexpr uint16_t SPLATOON_PURPLE = 0x780F; // 紫インク色相当

        // Constants
        static constexpr int SCREEN_WIDTH = 320;
        static constexpr int SCREEN_HEIGHT = 240;
        static constexpr int QUADRANT_WIDTH = SCREEN_WIDTH / 2;
        static constexpr int QUADRANT_HEIGHT = (SCREEN_HEIGHT - 12) / 2; // 下部12ピクセルは時刻表示用

        // バージョン情報
        static constexpr const char *VERSION = "v1.2.1";

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