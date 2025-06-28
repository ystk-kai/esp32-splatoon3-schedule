#include <Arduino.h>
#include <TFT_eSPI.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <time.h>
#include <Preferences.h>

// Domain layer
#include "domain/BattleType.h"
#include "domain/Rule.h"
#include "domain/Stage.h"
#include "domain/BattleSchedule.h"
#include "domain/DisplaySettings.h"
#include "domain/WiFiSettings.h"

// Application layer
#include "application/ScheduleRepository.h"
#include "application/DisplayService.h"
#include "application/NetworkService.h"
#include "application/ScheduleService.h"
#include "application/ScheduleApplicationService.h"
#include "application/WiFiService.h"
#include "application/SettingsService.h"
#include "application/AppInitializationService.h"
#include "application/WiFiConnectionManager.h"

// Infrastructure layer
#include "infrastructure/TFTDisplayService.h"
#include "infrastructure/ESP32NetworkService.h"
#include "infrastructure/APIScheduleRepository.h"
#include "infrastructure/ESP32WiFiService.h"
#include "infrastructure/PreferencesSettingsService.h"
#include "infrastructure/AppStateManager.h"
#include "infrastructure/ESP32AppInitializationService.h"
#include "infrastructure/ESP32WiFiConnectionManager.h"
#include "infrastructure/MemoryManager.h"
#include "infrastructure/DeviceInfo.h"

// Preferences for storing WiFi credentials and user settings
Preferences preferences;

// Display settings
#define TFT_BL 21 // バックライトピンをUser_Setup.hと同じに設定

// Applicationの状態を管理するマネージャ
Infrastructure::AppStateManager appStateManager;

// Infrastructure services
Infrastructure::ESP32NetworkService networkService;
Infrastructure::APIScheduleRepository scheduleRepository(networkService);
Infrastructure::TFTDisplayService displayService(TFT_BL, 0); // PWM Channel 0 for backlight control
Infrastructure::ESP32WiFiService wifiService;
Infrastructure::PreferencesSettingsService settingsService(preferences);

// Application services
Application::ScheduleService scheduleService(scheduleRepository);
Application::ScheduleApplicationService applicationService(scheduleService, displayService, networkService);

// WiFi接続とアプリ初期化を管理するサービス
Infrastructure::ESP32WiFiConnectionManager wifiConnectionManager(wifiService, displayService, appStateManager);
Infrastructure::ESP32AppInitializationService appInitializationService(
    networkService,
    displayService,
    applicationService,
    wifiConnectionManager,
    settingsService,
    appStateManager);

// メモリ監視用の変数
unsigned long lastMemoryCheck = 0;
const unsigned long MEMORY_CHECK_INTERVAL = 60000; // 1分ごとにメモリチェック

// 表示設定切り替え関数
void switchToEnglishDisplay()
{
    Domain::DisplaySettings settings = Domain::DisplaySettings::create(false, false, false);
    settingsService.saveDisplaySettings(settings);
    applicationService.setDisplaySettings(settings);
    applicationService.updateDisplay(); // 表示を更新
}

void switchToRomajiDisplay()
{
    Domain::DisplaySettings settings = Domain::DisplaySettings::create(true, true, true);
    settingsService.saveDisplaySettings(settings);
    applicationService.setDisplaySettings(settings);
    applicationService.updateDisplay(); // 表示を更新
}

void setup()
{
    // シリアル初期化
    Serial.begin(115200);

    // デバイス情報を表示
    Infrastructure::DeviceInfo::printDeviceInfo();

    // メモリ統計をリセット
    Infrastructure::MemoryManager::resetMemoryStats();

    // 初期メモリ使用量をログ
    Infrastructure::MemoryManager::logMemoryUsage("Setup start");

    // アプリケーションのセットアップ処理
    appInitializationService.performSetup();

    // セットアップ完了後のメモリ使用量をログ
    Infrastructure::MemoryManager::logMemoryUsage("Setup complete");
}

void loop()
{
    // 現在時刻を取得
    unsigned long currentMillis = millis();

    // 定期的なメモリ監視
    if (currentMillis - lastMemoryCheck >= MEMORY_CHECK_INTERVAL)
    {
        Infrastructure::MemoryManager::logMemoryUsage("Periodic check");
        lastMemoryCheck = currentMillis;

        // メモリ不足の場合は強制ガベージコレクションを試行
        if (Infrastructure::MemoryManager::isLowMemory())
        {
            Infrastructure::MemoryManager::forceGarbageCollection();
        }

        // メモリ傾向分析を実行
        Infrastructure::MemoryManager::analyzeMemoryTrend();
    }

    // 定期的なメモリクリーンアップ
    Infrastructure::MemoryManager::performPeriodicCleanup();

    // アプリケーションのメインループ処理
    appInitializationService.performLoop(currentMillis);

    // 適度な遅延
    delay(10);
}