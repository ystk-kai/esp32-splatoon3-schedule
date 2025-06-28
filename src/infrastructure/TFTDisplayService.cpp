#include "TFTDisplayService.h"
#include <cstring>
#include "DeviceInfo.h"

namespace Infrastructure
{
    // 静的変数の初期化
    bool TFTDisplayService::isFirstStatusCall = true;

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
        // 現在の反転状態を保存
        bool currentInverted = isInverted;

        // Clear screen
        clearScreen();

        // Turn on backlight to full brightness
        setBacklight(255);

        // Draw each quadrant
        drawBattleQuadrant(0, 0, regularSchedule, regularNextSchedule, displaySettings);
        drawBattleQuadrant(QUADRANT_WIDTH, 0, xMatchSchedule, xMatchNextSchedule, displaySettings);
        drawBattleQuadrant(0, QUADRANT_HEIGHT, bankaraChallengeSchedule, bankaraChallengeNextSchedule, displaySettings);
        drawBattleQuadrant(QUADRANT_WIDTH, QUADRANT_HEIGHT, bankaraOpenSchedule, bankaraOpenNextSchedule, displaySettings);

        // Draw dividing lines
        tft.drawLine(QUADRANT_WIDTH, 0, QUADRANT_WIDTH, SCREEN_HEIGHT, TFT_WHITE);
        tft.drawLine(0, QUADRANT_HEIGHT, SCREEN_WIDTH, QUADRANT_HEIGHT, TFT_WHITE);

        // Update bottom info (current date/time and update time)
        updateBottomInfo(currentDateTime, lastUpdateTime);

        // 反転状態を復元
        if (currentInverted)
        {
            tft.invertDisplay(true);
        }
    }

    void TFTDisplayService::updateBottomInfo(
        const char *currentDateTime,
        const char *lastUpdateTime)
    {
        // 現在の反転状態を保存
        bool currentInverted = isInverted;

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

        // 反転状態を復元
        if (currentInverted)
        {
            tft.invertDisplay(true);
        }
    }

    void TFTDisplayService::showStartupScreen()
    {
        // 現在の反転状態を保存
        bool currentInverted = isInverted;

        // 一時的に反転を無効化して描画
        if (currentInverted)
        {
            tft.invertDisplay(false);
        }

        clearScreen();

        // 画面上半分に背景色を設定
        tft.fillRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT / 2, SPLATOON_BLUE);

        // Splatoon3タイトルを表示
        tft.setTextColor(TFT_WHITE);
        tft.setTextSize(3);
        tft.setCursor(30, 40);
        tft.println("Splatoon3");

        // Scheduleサブタイトルを表示
        tft.setTextSize(2);
        tft.setCursor(90, 80);
        tft.println("Schedule");

        // 下半分にロゴや説明などを表示
        tft.setTextColor(SPLATOON_ORANGE);
        tft.setTextSize(1);
        tft.setCursor(20, SCREEN_HEIGHT / 2 + 30);
        tft.println("WiFi connection required.");

        tft.setTextColor(SPLATOON_GREEN);
        tft.setCursor(20, SCREEN_HEIGHT / 2 + 50);
        tft.println("Shows current & upcoming stages.");

        // バージョン情報
        tft.setTextColor(TFT_LIGHTGREY);
        tft.setCursor(SCREEN_WIDTH - 70, SCREEN_HEIGHT - 20);
        tft.println(VERSION);

        // 反転状態を復元
        if (currentInverted)
        {
            tft.invertDisplay(true);
        }
    }

    void TFTDisplayService::showConnectionStatus(bool connected, const char *statusMessage)
    {
        // 現在の反転状態を保存（リソース変数として使用）
        bool currentInverted = isInverted;

        // 初回表示かどうかを判断するための静的変数
        static String previousPortalMessage = "";

        // 前回表示された秒数の位置を記録する変数
        static int lastSecondsPosX = 0;
        static int lastSecondsPosY = 0;
        static String lastSecondsValue = "";

        // 前回のconnected状態を記録
        static bool previousConnected = false;

        // メッセージを文字列に変換（メモリ効率化のため最小限に）
        String message = String(statusMessage);

        // すでに接続済みの"Connection OK"画面が表示されている場合は、
        // 同じ接続状態で同様のメッセージ内容であれば更新をスキップ
        if (!isFirstStatusCall && previousConnected == connected && connected == true)
        {
            // 接続済み状態の場合はメッセージが"Connection OK"で始まるかチェック
            if (previousPortalMessage.startsWith("Connection OK") && message.startsWith("Connection OK"))
            {
                // 基本的には更新せず、SSID部分と IP部分が一致するかだけチェック
                int prevSSIDPos = previousPortalMessage.indexOf("SSID: ");
                int newSSIDPos = message.indexOf("SSID: ");

                if (prevSSIDPos >= 0 && newSSIDPos >= 0)
                {
                    // 前回のSSIDと今回のSSIDが同じなら更新しない
                    String prevSSIDLine = previousPortalMessage.substring(prevSSIDPos, previousPortalMessage.indexOf("\n", prevSSIDPos));
                    String newSSIDLine = message.substring(newSSIDPos, message.indexOf("\n", newSSIDPos));

                    if (prevSSIDLine == newSSIDLine)
                    {
                        // メッセージ文字列を解放してから終了
                        message.clear();
                        return;
                    }
                }
            }
        }

        // メッセージが全く同じで接続状態も同じなら何もしない（不要な再描画を防止）
        if (!isFirstStatusCall && previousPortalMessage == message && previousConnected == connected)
        {
            // メッセージ文字列を解放してから終了
            message.clear();
            return;
        }

        // カウントダウン秒数のみの変更かどうかを判断
        bool onlySecondsChanged = false;
        if (previousPortalMessage.length() > 0 && message.length() > 0)
        {
            // 両方のメッセージにカウントダウンが含まれているか確認
            int prevConnPos = previousPortalMessage.indexOf("Connecting in ");
            int newConnPos = message.indexOf("Connecting in ");

            if (prevConnPos >= 0 && newConnPos >= 0)
            {
                // メッセージの先頭部分（カウントダウン前まで）が同じか確認
                String prevPrefix = previousPortalMessage.substring(0, prevConnPos + 13); // "Connecting in "の手前まで
                String newPrefix = message.substring(0, newConnPos + 13);

                if (prevPrefix == newPrefix)
                {
                    // メッセージの基本部分が一致→秒数だけの変更と判断
                    onlySecondsChanged = true;
                }
            }
        }

        // 初回呼び出し時のみ画面をクリアして背景やヘッダーを描画
        if (isFirstStatusCall)
        {
            // 初回時には一時的に反転を無効化して描画（見やすさのため）
            if (currentInverted)
            {
                tft.invertDisplay(false);
            }

            clearScreen();

            // 上部に背景色のヘッダーを表示
            tft.fillRect(0, 0, SCREEN_WIDTH, 30, connected ? SPLATOON_GREEN : SPLATOON_ORANGE);

            // ヘッダーにタイトルを表示
            tft.setTextColor(TFT_BLACK);
            tft.setTextSize(2);
            tft.setCursor(40, 5);
            tft.println(connected ? "Connection OK" : "WiFi Setup");

            // 画面下部に装飾
            tft.fillRect(0, SCREEN_HEIGHT - 10, SCREEN_WIDTH, 10, connected ? SPLATOON_GREEN : SPLATOON_ORANGE);

            isFirstStatusCall = false;
            previousPortalMessage = "";    // メッセージもリセット
            previousConnected = connected; // 接続状態を記録

            // 初回表示後、反転状態を復元
            if (currentInverted)
            {
                tft.invertDisplay(true);
            }
        }
        // 接続状態が変わった場合はヘッダーを再描画
        else if (previousConnected != connected)
        {
            // 反転状態に影響しないように一時的に元に戻す
            bool needRestore = false;
            if (currentInverted)
            {
                tft.invertDisplay(false);
                needRestore = true;
            }

            // ヘッダー部分のみ更新
            tft.fillRect(0, 0, SCREEN_WIDTH, 30, connected ? SPLATOON_GREEN : SPLATOON_ORANGE);
            tft.setTextColor(TFT_BLACK);
            tft.setTextSize(2);
            tft.setCursor(40, 5);
            tft.println(connected ? "Connection OK" : "WiFi Setup");

            // 画面下部の装飾も更新
            tft.fillRect(0, SCREEN_HEIGHT - 10, SCREEN_WIDTH, 10, connected ? SPLATOON_GREEN : SPLATOON_ORANGE);

            previousConnected = connected; // 接続状態を更新

            // 反転状態を元に戻す（必要な場合のみ）
            if (needRestore)
            {
                tft.invertDisplay(true);
            }
        }

        // メインメッセージ部分が変わった場合かつ秒数だけの変更でない場合はその部分をクリア
        if (previousPortalMessage != message && !onlySecondsChanged)
        {
            // 反転状態に影響しないように一時的に元に戻す
            bool needRestore = false;
            if (currentInverted)
            {
                tft.invertDisplay(false);
                needRestore = true;
            }

            // WiFi情報とカウントダウン以外のエリアをクリア（秒数部分は別で処理）
            tft.fillRect(0, 40, SCREEN_WIDTH, SCREEN_HEIGHT - 50, TFT_BLACK);

            // 反転状態を元に戻す（必要な場合のみ）
            if (needRestore)
            {
                tft.invertDisplay(true);
            }
        }

        // 秒数だけの更新の場合、反転処理は一切省略
        bool needRestoreForText = false;
        if (!onlySecondsChanged && currentInverted)
        {
            tft.invertDisplay(false);
            needRestoreForText = true;
        }

        // メッセージの行を分割して色分けして表示する
        int currentY = 50;
        int lastPos = 0;
        int pos = message.indexOf('\n');

        while (pos >= 0 || lastPos < message.length())
        {
            String line = pos >= 0 ? message.substring(lastPos, pos) : message.substring(lastPos);

            // SSIDの行
            if (line.startsWith("SSID:"))
            {
                // "SSID: " の部分をハイライト色で
                tft.setTextColor(SPLATOON_YELLOW);
                tft.setTextSize(1);
                tft.setCursor(40, currentY);
                tft.print("SSID: ");

                // 実際のSSID値を別の色で
                tft.setTextColor(TFT_WHITE);
                tft.print(line.substring(6)); // "SSID: " の後の部分
            }
            // IPの行
            else if (line.startsWith("IP:"))
            {
                // "IP: " の部分をハイライト色で
                tft.setTextColor(SPLATOON_YELLOW);
                tft.setTextSize(1);
                tft.setCursor(40, currentY);
                tft.print("IP: ");

                // 実際のIP値を別の色で
                tft.setTextColor(TFT_WHITE);
                tft.print(line.substring(4)); // "IP: " の後の部分
            }
            // カウントダウンの行
            else if (line.indexOf("Connecting in ") >= 0)
            {
                int numPos = line.indexOf("Connecting in ") + 14;
                int secPos = line.indexOf("s...");

                // 秒数を表示
                if (secPos > numPos)
                {
                    // 秒数部分の更新処理
                    String seconds = line.substring(numPos, secPos);

                    // 秒数が変わった場合だけ更新
                    if (seconds != lastSecondsValue || !onlySecondsChanged)
                    {
                        // カウントダウン行全体をクリア
                        tft.fillRect(40, currentY, SCREEN_WIDTH - 80, 10, TFT_BLACK);

                        // "Connecting in " の部分を再描画
                        tft.setTextColor(SPLATOON_PINK);
                        tft.setTextSize(1);
                        tft.setCursor(40, currentY);
                        tft.print("Connecting in ");

                        // 秒数文字列の開始位置を記録
                        int secondsPosX = tft.getCursorX();
                        int secondsPosY = tft.getCursorY();

                        lastSecondsValue = seconds;    // 現在の値を保存
                        lastSecondsPosX = secondsPosX; // 現在の位置を保存
                        lastSecondsPosY = secondsPosY;

                        // 秒数を表示（通常サイズで）
                        tft.setTextColor(TFT_WHITE);
                        tft.print(seconds);

                        // "s..." の部分
                        tft.setTextColor(SPLATOON_PINK);
                        tft.print("s...");
                    }
                }
                else
                {
                    // 秒数が抽出できない場合は通常どおり表示
                    tft.setTextColor(SPLATOON_PINK);
                    tft.setTextSize(1);
                    tft.setCursor(40, currentY);
                    tft.print("Connecting in ");
                    tft.print(line.substring(numPos));
                }
            }
            // その他のメッセージ
            else
            {
                tft.setTextColor(connected ? SPLATOON_GREEN : SPLATOON_PINK);
                tft.setTextSize(1);
                tft.setCursor(40, currentY);
                tft.println(line);
            }

            // 次の行へ
            currentY += 20;

            if (pos >= 0)
            {
                lastPos = pos + 1;
                pos = message.indexOf('\n', lastPos);
            }
            else
            {
                break;
            }
        }

        // メッセージを保存（次回比較用）
        previousPortalMessage = message;

        // 反転状態を元に戻す（必要な場合のみ）
        if (needRestoreForText)
        {
            tft.invertDisplay(true);
        }
    }

    // オリジナルのshowLoadingMessage: 後方互換性のために残します
    void TFTDisplayService::showLoadingMessage(const char *message)
    {
        // デフォルトでは背景更新モードではない（フルスクリーンロード表示）
        showLoadingMessage(message, false);
    }

    // バックグラウンド更新に対応したshowLoadingMessage
    void TFTDisplayService::showLoadingMessage(const char *message, bool backgroundUpdate)
    {
        // 現在の反転状態を保存
        bool currentInverted = isInverted;

        // 一時的に反転を無効化して描画
        if (currentInverted)
        {
            tft.invertDisplay(false);
        }

        if (!backgroundUpdate)
        {
            // 通常のロード画面（フルスクリーン）
            clearScreen();

            // 上部に背景色のヘッダーを表示
            tft.fillRect(0, 0, SCREEN_WIDTH, 30, SPLATOON_BLUE);

            // ヘッダーにタイトルを表示
            tft.setTextColor(TFT_WHITE);
            tft.setTextSize(2);
            tft.setCursor(80, 5);
            tft.println("Loading...");

            // メッセージを表示
            tft.setTextColor(SPLATOON_YELLOW);
            tft.setTextSize(1);
            tft.setCursor(40, 50);
            tft.println(message);

            // 進捗バーの背景
            tft.fillRect(40, 80, SCREEN_WIDTH - 80, 20, TFT_DARKGREY);

            // カラフルな進捗バー
            int loadWidth = (SCREEN_WIDTH - 80) / 4;
            tft.fillRect(40, 80, loadWidth, 20, SPLATOON_BLUE);
            tft.fillRect(40 + loadWidth, 80, loadWidth, 20, SPLATOON_ORANGE);
            tft.fillRect(40 + loadWidth * 2, 80, loadWidth, 20, SPLATOON_GREEN);
            tft.fillRect(40 + loadWidth * 3, 80, loadWidth, 20, SPLATOON_PINK);

            // ステータスメッセージ
            tft.setTextColor(SPLATOON_GREEN);
            tft.setTextSize(1);
            tft.setCursor(25, 130);
            tft.println("Getting latest Splatoon3 information...");
        }
        else
        {
            // バックグラウンド更新モード - 最小限の視覚的な変更だけ行う
            // 下部情報バーに更新インジケータを表示（小さな点滅ドット）
            static bool blinkState = false;
            blinkState = !blinkState;

            // 画面下部の右端に小さな更新インジケータを表示
            uint16_t indicatorColor = blinkState ? SPLATOON_GREEN : SPLATOON_BLUE;
            tft.fillCircle(SCREEN_WIDTH - 10, SCREEN_HEIGHT - 10, 3, indicatorColor);

            // シリアルログにのみ更新状態を出力
            Serial.print("Background updating: ");
            Serial.println(message);
        }

        // 反転状態を復元
        if (currentInverted)
        {
            tft.invertDisplay(true);
        }
    }

    void TFTDisplayService::drawBattleQuadrant(
        int x,
        int y,
        const Domain::BattleSchedule &current,
        const Domain::BattleSchedule &next,
        const Domain::DisplaySettings &displaySettings)
    {
        // 現在の反転状態を保存
        bool currentInverted = isInverted;

        // 一時的に反転を無効化して描画
        if (currentInverted)
        {
            tft.invertDisplay(false);
        }

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

            // 反転状態を復元
            if (currentInverted)
            {
                tft.invertDisplay(true);
            }
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

        // 反転状態を復元
        if (currentInverted)
        {
            tft.invertDisplay(true);
        }
    }

    void TFTDisplayService::showDeviceInfo()
    {
        // 現在の反転状態を保存
        bool currentInverted = isInverted;

        // 一時的に反転を無効化して描画
        if (currentInverted)
        {
            tft.invertDisplay(false);
        }

        clearScreen();

        // 上部に背景色のヘッダーを表示
        tft.fillRect(0, 0, SCREEN_WIDTH, 30, SPLATOON_BLUE);

        // ヘッダーにタイトルを表示
        tft.setTextColor(TFT_WHITE);
        tft.setTextSize(2);
        tft.setCursor(40, 5);
        tft.println("Device Info");

        // デバイス情報を表示
        tft.setTextColor(TFT_WHITE);
        tft.setTextSize(1);
        int yPos = 40;

        // デバイス種類
        tft.setCursor(10, yPos);
        tft.print("Device: ");
        tft.setTextColor(SPLATOON_YELLOW);
        tft.println(Infrastructure::DeviceInfo::getDeviceType());
        yPos += 18;

        // チップ情報
        tft.setTextColor(TFT_WHITE);
        tft.setCursor(10, yPos);
        tft.print("Chip: ");
        tft.setTextColor(SPLATOON_YELLOW);
        tft.println(Infrastructure::DeviceInfo::getChipInfo());
        yPos += 18;

        // メモリ情報
        tft.setTextColor(TFT_WHITE);
        tft.setCursor(10, yPos);
        tft.print("Memory: ");
        tft.setTextColor(SPLATOON_GREEN);

        // メモリ情報を複数行に分けて表示
        size_t freeHeap = ESP.getFreeHeap();
        size_t minFreeHeap = ESP.getMinFreeHeap();
        size_t maxAllocHeap = ESP.getMaxAllocHeap();

        tft.print("Free: ");
        tft.print(freeHeap);
        tft.println(" bytes");
        yPos += 12;

        tft.setCursor(10, yPos);
        tft.setTextColor(TFT_WHITE);
        tft.print("        Min: ");
        tft.setTextColor(SPLATOON_GREEN);
        tft.print(minFreeHeap);
        tft.println(" bytes");
        yPos += 12;

        tft.setCursor(10, yPos);
        tft.setTextColor(TFT_WHITE);
        tft.print("        Max: ");
        tft.setTextColor(SPLATOON_GREEN);
        tft.print(maxAllocHeap);
        tft.println(" bytes");
        yPos += 15;

        // フラッシュ情報
        tft.setTextColor(TFT_WHITE);
        tft.setCursor(10, yPos);
        tft.print("Flash: ");
        tft.setTextColor(SPLATOON_ORANGE);

        // フラッシュ情報を複数行に分けて表示
        uint32_t flashSize = ESP.getFlashChipSize();
        uint32_t flashSpeed = ESP.getFlashChipSpeed();

        tft.print("Size: ");
        tft.print(flashSize / 1024 / 1024);
        tft.println("MB");
        yPos += 12;

        tft.setCursor(10, yPos);
        tft.setTextColor(TFT_WHITE);
        tft.print("       Speed: ");
        tft.setTextColor(SPLATOON_ORANGE);
        tft.print(flashSpeed / 1000000);
        tft.println("MHz");
        yPos += 20;

        // ディスプレイ情報
        tft.setTextColor(TFT_WHITE);
        tft.setCursor(10, yPos);
        tft.print("Display: ");
        tft.setTextColor(SPLATOON_PINK);

        // ディスプレイ情報を複数行に分けて表示
        int width, height;
        String displayType;
        Infrastructure::DeviceInfo::getDisplayDetails(width, height, displayType);

        tft.print(displayType);
        tft.println(" ");
        yPos += 12;

        tft.setCursor(10, yPos);
        tft.setTextColor(TFT_WHITE);
        tft.print("         ");
        tft.setTextColor(SPLATOON_PINK);
        tft.print(width);
        tft.print("x");
        tft.print(height);
        tft.println(" pixels");
        yPos += 12;

        tft.setCursor(10, yPos);
        tft.setTextColor(TFT_WHITE);
        tft.print("         ");
        tft.setTextColor(SPLATOON_PINK);
        tft.println(Infrastructure::DeviceInfo::getDisplayColorDepth());
        yPos += 12;

        tft.setCursor(10, yPos);
        tft.setTextColor(TFT_WHITE);
        tft.print("         ");
        tft.setTextColor(SPLATOON_PINK);
        tft.println(Infrastructure::DeviceInfo::getDisplayOrientation());
        yPos += 20;

        // デフォルト設定情報
        int defaultBrightness, defaultUpdateInterval;
        Infrastructure::DeviceInfo::getDeviceDefaults(defaultBrightness, defaultUpdateInterval);

        tft.setTextColor(TFT_WHITE);
        tft.setCursor(10, yPos);
        tft.print("Default Brightness: ");
        tft.setTextColor(SPLATOON_PINK);
        tft.println(defaultBrightness);
        yPos += 18;

        tft.setTextColor(TFT_WHITE);
        tft.setCursor(10, yPos);
        tft.print("Update Interval: ");
        tft.setTextColor(SPLATOON_PINK);
        tft.print(defaultUpdateInterval / 1000);
        tft.println("s");

        // バージョン情報
        tft.setTextColor(TFT_LIGHTGREY);
        tft.setCursor(SCREEN_WIDTH - 70, SCREEN_HEIGHT - 20);
        tft.println(VERSION);

        // 反転状態を復元
        if (currentInverted)
        {
            tft.invertDisplay(true);
        }
    }

} // namespace Infrastructure