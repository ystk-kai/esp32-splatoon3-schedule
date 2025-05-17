#include "TFTDisplayService.h"

namespace Infrastructure
{

    void TFTDisplayService::updateScreen(
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
        const Domain::DisplaySettings &displaySettings)
    {
        // Clear the screen first
        tft.fillScreen(TFT_BLACK);

        // Draw each battle quadrant
        drawBattleQuadrant(0, 0, regularSchedule, regularNextSchedule, displaySettings);
        drawBattleQuadrant(QUADRANT_WIDTH, 0, xMatchSchedule, xMatchNextSchedule, displaySettings);
        drawBattleQuadrant(0, QUADRANT_HEIGHT, bankaraChallengeSchedule, bankaraChallengeNextSchedule, displaySettings);
        drawBattleQuadrant(QUADRANT_WIDTH, QUADRANT_HEIGHT, bankaraOpenSchedule, bankaraOpenNextSchedule, displaySettings);

        // Draw dividing lines
        tft.drawLine(QUADRANT_WIDTH, 0, QUADRANT_WIDTH, SCREEN_HEIGHT, TFT_WHITE);
        tft.drawLine(0, QUADRANT_HEIGHT, SCREEN_WIDTH, QUADRANT_HEIGHT, TFT_WHITE);

        // Update bottom info
        updateBottomInfo(currentDateTime, lastUpdateTime);
    }

    void TFTDisplayService::updateBottomInfo(
        const char *currentDateTime,
        const char *lastUpdateTime)
    {
        // Clear the bottom info area
        tft.fillRect(0, SCREEN_HEIGHT - 12, SCREEN_WIDTH, 12, TFT_BLACK);

        // Display current date/time
        tft.setTextColor(TFT_WHITE);
        tft.setTextSize(1);
        tft.setCursor(4, SCREEN_HEIGHT - 12);
        tft.print(currentDateTime);

        // Display last update time at the right side
        tft.setTextColor(TFT_WHITE);
        tft.setTextSize(1);
        int textWidth = tft.textWidth(lastUpdateTime) + tft.textWidth("Updated: ");
        tft.setCursor(SCREEN_WIDTH - textWidth - 10, SCREEN_HEIGHT - 12);
        tft.print("Updated: ");
        tft.print(lastUpdateTime);

        // Redraw any dividing line that crosses the bottom info area
        tft.drawLine(QUADRANT_WIDTH, SCREEN_HEIGHT - 12, QUADRANT_WIDTH, SCREEN_HEIGHT, TFT_WHITE);
    }

    void TFTDisplayService::showStartupScreen()
    {
        clearScreen();
        tft.setTextColor(TFT_GREEN);
        tft.setTextSize(2);
        tft.setCursor(40, 100);
        tft.println("Splatoon3");
        tft.setCursor(40, 140);
        tft.println("Schedule");
    }

    void TFTDisplayService::showConnectionStatus(bool connected, const char *statusMessage)
    {
        tft.setTextColor(connected ? TFT_GREEN : TFT_RED);
        tft.setTextSize(1);
        tft.setCursor(70, 180);
        tft.println(statusMessage);
    }

    void TFTDisplayService::showLoadingMessage(const char *message)
    {
        clearScreen();
        tft.setTextColor(TFT_WHITE);
        tft.setTextSize(1);
        tft.setCursor(50, 150);
        tft.println(message);
    }

    void TFTDisplayService::drawBattleQuadrant(
        int x,
        int y,
        const Domain::BattleSchedule &current,
        const Domain::BattleSchedule &next,
        const Domain::DisplaySettings &displaySettings)
    {
        // Get the battle type for title and color
        const Domain::BattleType &battleType = current.getBattleType();
        uint16_t titleColor = battleType.getColor();

        // Draw title background with specific color
        tft.fillRect(x, y, QUADRANT_WIDTH, 16, titleColor);

        // Draw title text
        tft.setTextColor(TFT_BLACK); // Black text on colored background
        tft.setTextSize(1);
        tft.setCursor(x + 4, y + 4);
        tft.println(battleType.getDisplayName(displaySettings.isUseRomajiForBattleType()));

        // If the schedule is not valid, show error and return
        if (!current.isValid())
        {
            tft.setTextColor(TFT_RED);
            tft.setCursor(x + 4, y + 20);
            tft.println("Data Error");
            return;
        }

        // Now draw the current schedule details
        tft.setTextColor(TFT_WHITE);
        int yPos = 20;

        // 1. Time period
        tft.setCursor(x + 4, y + yPos);
        tft.print(current.getStartTime());
        tft.print("-");
        tft.println(current.getEndTime());
        yPos += 10;

        // 2. Rule with symbol (if not empty)
        const Domain::Rule &rule = current.getRule();
        const char *ruleSymbol = rule.getSymbol();
        const char *ruleName = rule.getDisplayName(displaySettings.isUseRomajiForRule());

        if (strlen(ruleName) > 0 || strlen(ruleSymbol) > 0)
        {
            // Draw rule symbol with its color
            tft.setCursor(x + 4, y + yPos);
            tft.setTextColor(rule.getSymbolColor());
            tft.print(ruleSymbol);

            // Draw rule name in white
            tft.setTextColor(TFT_WHITE);
            tft.println(ruleName);
            yPos += 10;
        }

        // 3. Stage names in light grey
        tft.setTextColor(TFT_LIGHTGREY);

        // Stage 1
        tft.setCursor(x + 4, y + yPos);
        tft.print("- ");

        // Get stage name and limit to 13 chars
        const char *stage1Name = current.getStage1().getDisplayName(displaySettings.isUseRomajiForStage());
        char shortenedStage[14]; // 13 chars + null terminator
        strncpy(shortenedStage, stage1Name, 13);
        shortenedStage[13] = '\0';
        tft.println(shortenedStage);
        yPos += 10;

        // Stage 2
        tft.setCursor(x + 4, y + yPos);
        tft.print("- ");

        // Get stage name and limit to 13 chars
        const char *stage2Name = current.getStage2().getDisplayName(displaySettings.isUseRomajiForStage());
        strncpy(shortenedStage, stage2Name, 13);
        shortenedStage[13] = '\0';
        tft.println(shortenedStage);
        yPos += 15; // Slightly larger gap before next section

        // Next battle schedule (if valid)
        if (next.isValid())
        {
            tft.setTextColor(TFT_WHITE);

            // 1. Time period
            tft.setCursor(x + 4, y + yPos);
            tft.print(next.getStartTime());
            tft.print("-");
            tft.println(next.getEndTime());
            yPos += 10;

            // 2. Rule with symbol (if not empty)
            const Domain::Rule &nextRule = next.getRule();
            const char *nextRuleSymbol = nextRule.getSymbol();
            const char *nextRuleName = nextRule.getDisplayName(displaySettings.isUseRomajiForRule());

            if (strlen(nextRuleName) > 0 || strlen(nextRuleSymbol) > 0)
            {
                // Draw rule symbol with its color
                tft.setCursor(x + 4, y + yPos);
                tft.setTextColor(nextRule.getSymbolColor());
                tft.print(nextRuleSymbol);

                // Draw rule name in white
                tft.setTextColor(TFT_WHITE);
                tft.println(nextRuleName);
                yPos += 10;
            }

            // 3. Stage names in light grey
            tft.setTextColor(TFT_LIGHTGREY);

            // Stage 1
            tft.setCursor(x + 4, y + yPos);
            tft.print("- ");

            // Get stage name and limit to 13 chars
            const char *nextStage1Name = next.getStage1().getDisplayName(displaySettings.isUseRomajiForStage());
            strncpy(shortenedStage, nextStage1Name, 13);
            shortenedStage[13] = '\0';
            tft.println(shortenedStage);
            yPos += 10;

            // Stage 2
            tft.setCursor(x + 4, y + yPos);
            tft.print("- ");

            // Get stage name and limit to 13 chars
            const char *nextStage2Name = next.getStage2().getDisplayName(displaySettings.isUseRomajiForStage());
            strncpy(shortenedStage, nextStage2Name, 13);
            shortenedStage[13] = '\0';
            tft.println(shortenedStage);
        }
    }

} // namespace Infrastructure