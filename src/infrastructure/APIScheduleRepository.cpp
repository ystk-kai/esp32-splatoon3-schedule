#include "APIScheduleRepository.h"

namespace Infrastructure
{

    Domain::BattleSchedule APIScheduleRepository::fetchCurrentSchedule(const Domain::BattleType &battleType)
    {
        if (battleType == Domain::BattleType::regular())
        {
            return regularSchedule;
        }
        else if (battleType == Domain::BattleType::xMatch())
        {
            return xMatchSchedule;
        }
        else if (battleType == Domain::BattleType::bankaraChallenge())
        {
            return bankaraChallengeSchedule;
        }
        else if (battleType == Domain::BattleType::bankaraOpen())
        {
            return bankaraOpenSchedule;
        }

        // Default to an empty schedule
        return Domain::BattleSchedule::createEmpty(battleType);
    }

    Domain::BattleSchedule APIScheduleRepository::fetchNextSchedule(const Domain::BattleType &battleType)
    {
        if (battleType == Domain::BattleType::regular())
        {
            return regularNextSchedule;
        }
        else if (battleType == Domain::BattleType::xMatch())
        {
            return xMatchNextSchedule;
        }
        else if (battleType == Domain::BattleType::bankaraChallenge())
        {
            return bankaraChallengeNextSchedule;
        }
        else if (battleType == Domain::BattleType::bankaraOpen())
        {
            return bankaraOpenNextSchedule;
        }

        // Default to an empty schedule
        return Domain::BattleSchedule::createEmpty(battleType);
    }

    void APIScheduleRepository::updateAllSchedules()
    {
        // メモリ使用量をログ
        logMemoryUsage("Before updateAllSchedules");

        // Update each battle type
        updateScheduleForBattleType(Domain::BattleType::regular());
        delay(200);
        updateScheduleForBattleType(Domain::BattleType::xMatch());
        delay(200);
        updateScheduleForBattleType(Domain::BattleType::bankaraChallenge());
        delay(200);
        updateScheduleForBattleType(Domain::BattleType::bankaraOpen());

        // メモリ使用量をログ
        logMemoryUsage("After updateAllSchedules");
    }

    void APIScheduleRepository::initializeSchedules()
    {
        // 各バトルタイプの空のスケジュールを作成
        regularSchedule = Domain::BattleSchedule::createEmpty(Domain::BattleType::regular());
        xMatchSchedule = Domain::BattleSchedule::createEmpty(Domain::BattleType::xMatch());
        bankaraChallengeSchedule = Domain::BattleSchedule::createEmpty(Domain::BattleType::bankaraChallenge());
        bankaraOpenSchedule = Domain::BattleSchedule::createEmpty(Domain::BattleType::bankaraOpen());

        // 次のスケジュールも同様に初期化
        regularNextSchedule = Domain::BattleSchedule::createEmpty(Domain::BattleType::regular());
        xMatchNextSchedule = Domain::BattleSchedule::createEmpty(Domain::BattleType::xMatch());
        bankaraChallengeNextSchedule = Domain::BattleSchedule::createEmpty(Domain::BattleType::bankaraChallenge());
        bankaraOpenNextSchedule = Domain::BattleSchedule::createEmpty(Domain::BattleType::bankaraOpen());

        Serial.println("All schedules initialized to empty state");
    }

    void APIScheduleRepository::updateScheduleForBattleType(const Domain::BattleType &battleType)
    {
        // Debug message
        Serial.print("Updating ");
        Serial.print(battleType.getEnglishName());
        Serial.println(" data...");

        // Get current schedule
        String currentResponse = networkService.httpGet(battleType.getCurrentScheduleUrl());
        if (currentResponse.length() > 0)
        {
            Domain::BattleSchedule schedule = parseScheduleFromJson(currentResponse, battleType, true);

            // Store in appropriate cache
            if (battleType == Domain::BattleType::regular())
            {
                regularSchedule = schedule;
            }
            else if (battleType == Domain::BattleType::xMatch())
            {
                xMatchSchedule = schedule;
            }
            else if (battleType == Domain::BattleType::bankaraChallenge())
            {
                bankaraChallengeSchedule = schedule;
            }
            else if (battleType == Domain::BattleType::bankaraOpen())
            {
                bankaraOpenSchedule = schedule;
            }
        }

        // レスポンス文字列を明示的に解放
        currentResponse.clear();

        delay(200); // Short delay between requests

        // Get next schedule
        String nextResponse = networkService.httpGet(battleType.getNextScheduleUrl());
        if (nextResponse.length() > 0)
        {
            Domain::BattleSchedule schedule = parseScheduleFromJson(nextResponse, battleType, false);

            // Store in appropriate cache
            if (battleType == Domain::BattleType::regular())
            {
                regularNextSchedule = schedule;
            }
            else if (battleType == Domain::BattleType::xMatch())
            {
                xMatchNextSchedule = schedule;
            }
            else if (battleType == Domain::BattleType::bankaraChallenge())
            {
                bankaraChallengeNextSchedule = schedule;
            }
            else if (battleType == Domain::BattleType::bankaraOpen())
            {
                bankaraOpenNextSchedule = schedule;
            }
        }

        // レスポンス文字列を明示的に解放
        nextResponse.clear();
    }

    Domain::BattleSchedule APIScheduleRepository::parseScheduleFromJson(
        const String &jsonResponse,
        const Domain::BattleType &battleType,
        bool isCurrentSchedule)
    {
        // Parse JSON response - 最新のArduinoJsonの使用方法
        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, jsonResponse);

        if (error)
        {
            Serial.print("JSON parse error: ");
            Serial.println(error.c_str());
            // JSON ドキュメントを明示的に解放
            doc.clear();
            return Domain::BattleSchedule::createEmpty(battleType);
        }

        // バトルタイプに基づいて適切なルールを選択
        Domain::Rule rule; // デフォルトコンストラクタでUNKNOWNタイプに初期化される

        // レギュラーマッチの場合はナワバリバトル固定
        if (battleType == Domain::BattleType::regular())
        {
            rule = Domain::Rule::turfWar();
        }
        else
        {
            // その他のマッチタイプではAPIからルールを取得
            const char *japaneseRule = doc["results"][0]["rule"]["name"];
            if (japaneseRule && strlen(japaneseRule) > 0)
            {
                // 日本語ルール名からルールを取得
                rule = Domain::Rule::fromJapaneseName(japaneseRule);
            }
            // else節は不要。デフォルトでunknownになっている
        }

        // ステージ情報の抽出と処理
        const char *japaneseStage1 = doc["results"][0]["stages"][0]["name"];
        const char *japaneseStage2 = doc["results"][0]["stages"][1]["name"];

        // ステージ情報の変換
        Domain::Stage stage1 = (japaneseStage1 && strlen(japaneseStage1) > 0)
                                   ? Domain::Stage::fromJapaneseName(japaneseStage1)
                                   : Domain::Stage::fromJapaneseName("不明");

        Domain::Stage stage2 = (japaneseStage2 && strlen(japaneseStage2) > 0)
                                   ? Domain::Stage::fromJapaneseName(japaneseStage2)
                                   : Domain::Stage::fromJapaneseName("不明");

        // 時間情報の抽出
        char startTime[6] = "--:--";
        char endTime[6] = "--:--";

        const char *st = doc["results"][0]["start_time"];
        if (st && strlen(st) > 16)
        {
            strncpy(startTime, st + 11, 5);
            startTime[5] = '\0';
        }

        const char *et = doc["results"][0]["end_time"];
        if (et && strlen(et) > 16)
        {
            strncpy(endTime, et + 11, 5);
            endTime[5] = '\0';
        }

        // デバッグ情報の出力
        Serial.print("Rule: ");
        Serial.print(rule.getSymbol());
        Serial.println(rule.getEnglishName());
        Serial.print("Stage 1: ");
        Serial.println(stage1.getEnglishName());
        Serial.print("Stage 2: ");
        Serial.println(stage2.getEnglishName());
        Serial.print("Time: ");
        Serial.print(startTime);
        Serial.print(" - ");
        Serial.println(endTime);

        // バトルスケジュールを作成して返す
        Domain::BattleSchedule result = Domain::BattleSchedule::create(
            battleType,
            rule,
            stage1,
            stage2,
            startTime,
            endTime);

        // JSON ドキュメントを明示的に解放
        doc.clear();

        return result;
    }

    void APIScheduleRepository::logMemoryUsage(const char *operation)
    {
        // ESP32のメモリ使用量を取得
        size_t freeHeap = ESP.getFreeHeap();
        size_t minFreeHeap = ESP.getMinFreeHeap();
        size_t maxAllocHeap = ESP.getMaxAllocHeap();

        Serial.print("Memory [");
        Serial.print(operation);
        Serial.print("] - Free: ");
        Serial.print(freeHeap);
        Serial.print(" bytes, Min Free: ");
        Serial.print(minFreeHeap);
        Serial.print(" bytes, Max Alloc: ");
        Serial.print(maxAllocHeap);
        Serial.println(" bytes");

        // メモリ不足の警告
        if (freeHeap < 10000) // 10KB未満の場合
        {
            Serial.println("WARNING: Low memory detected!");
        }
    }

} // namespace Infrastructure