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

// Application layer
#include "application/ScheduleRepository.h"
#include "application/DisplayService.h"
#include "application/NetworkService.h"
#include "application/ScheduleService.h"
#include "application/ScheduleApplicationService.h"

// Infrastructure layer
#include "infrastructure/TFTDisplayService.h"
#include "infrastructure/ESP32NetworkService.h"
#include "infrastructure/APIScheduleRepository.h"

// Preferences for storing WiFi credentials
Preferences preferences;

// WiFi settings (will be loaded from preferences or set via captive portal)
String wifiSsid;
String wifiPassword;

// Display settings
#define TFT_BL 21 // バックライトピンをUser_Setup.hと同じに設定

// Application services
Infrastructure::ESP32NetworkService networkService;
Infrastructure::APIScheduleRepository scheduleRepository(networkService);
Application::ScheduleService scheduleService(scheduleRepository);
Infrastructure::TFTDisplayService displayService(TFT_BL, 0); // PWM Channel 0 for backlight control
Application::ScheduleApplicationService applicationService(scheduleService, displayService, networkService);

// Timer for periodic updates
unsigned long lastDataUpdateTime = 0;
unsigned long lastTimeDisplayUpdateTime = 0;
const unsigned long DATA_UPDATE_INTERVAL = 5 * 60 * 1000;     // 5分ごとにデータを更新（5分 x 60秒 x 1000ms）
const unsigned long TIME_DISPLAY_UPDATE_INTERVAL = 10 * 1000; // 10秒ごとに時間表示を更新

// 設定をPreferencesから読み込む
bool loadSettings()
{
  // Preferencesを開く
  preferences.begin("splatoon3", false); // 読み取り専用モード

  // WiFi設定を読み込む
  wifiSsid = preferences.getString("ssid", "");
  wifiPassword = preferences.getString("password", "");

  // 表示設定を読み込む
  bool useRomajiForBattleType = preferences.getBool("romaji_battle", true); // デフォルトはローマ字表示
  bool useRomajiForRule = preferences.getBool("romaji_rule", true);         // デフォルトはローマ字表示
  bool useRomajiForStage = preferences.getBool("romaji_stage", true);       // デフォルトはローマ字表示

  // 表示設定をアプリケーションサービスに設定
  Domain::DisplaySettings displaySettings = Domain::DisplaySettings::create(
      useRomajiForBattleType,
      useRomajiForRule,
      useRomajiForStage);
  applicationService.setDisplaySettings(displaySettings);

  // Preferencesを閉じる
  preferences.end();

  // WiFi設定が存在するか確認
  return (wifiSsid.length() > 0);
}

// 設定をPreferencesに保存する
void saveSettings(const String &ssid, const String &password)
{
  // Preferencesを開く（書き込みモード）
  preferences.begin("splatoon3", false);

  // WiFi設定を保存
  preferences.putString("ssid", ssid);
  preferences.putString("password", password);

  // Preferencesを閉じる
  preferences.end();

  // 変数に設定を反映
  wifiSsid = ssid;
  wifiPassword = password;
}

// 表示設定を保存する
void saveDisplaySettings(const Domain::DisplaySettings &settings)
{
  // Preferencesを開く（書き込みモード）
  preferences.begin("splatoon3", false);

  // 表示設定を保存
  preferences.putBool("romaji_battle", settings.isUseRomajiForBattleType());
  preferences.putBool("romaji_rule", settings.isUseRomajiForRule());
  preferences.putBool("romaji_stage", settings.isUseRomajiForStage());

  // Preferencesを閉じる
  preferences.end();

  // アプリケーションサービスに設定を反映
  applicationService.setDisplaySettings(settings);
}

// 表示設定を英語モードに切り替え
void switchToEnglishDisplay()
{
  Domain::DisplaySettings settings = Domain::DisplaySettings::create(false, false, false);
  saveDisplaySettings(settings);
  applicationService.updateDisplay(); // 表示を更新
}

// 表示設定をローマ字モードに切り替え
void switchToRomajiDisplay()
{
  Domain::DisplaySettings settings = Domain::DisplaySettings::create(true, true, true);
  saveDisplaySettings(settings);
  applicationService.updateDisplay(); // 表示を更新
}

void setup()
{
  // シリアル初期化
  Serial.begin(115200);
  Serial.println("\nSplatoon3 Schedule Viewer");
  Serial.println("Initializing...");

  // 設定の読み込み
  bool hasSettings = loadSettings();
  if (!hasSettings)
  {
    // デフォルト設定を使用（本来はキャプティブポータルなどで設定すべき）
    Serial.println("No WiFi settings found. Using default settings.");
    wifiSsid = "SSID名";
    wifiPassword = "パスワード";
  }
  else
  {
    Serial.println("WiFi settings loaded from preferences");
  }

  // Try to initialize the application
  bool initialized = applicationService.initialize(wifiSsid.c_str(), wifiPassword.c_str());

  if (!initialized)
  {
    Serial.println("Failed to initialize. Check WiFi connection.");
  }
}

void loop()
{
  unsigned long currentMillis = millis();

  // データの定期更新（5分ごと）
  if (currentMillis - lastDataUpdateTime >= DATA_UPDATE_INTERVAL)
  {
    Serial.println("Updating all schedule data...");
    applicationService.updateAllData();
    lastDataUpdateTime = currentMillis;
  }

  // 時刻表示の更新（10秒ごと）
  if (currentMillis - lastTimeDisplayUpdateTime >= TIME_DISPLAY_UPDATE_INTERVAL)
  {
    applicationService.updateTimeDisplay();
    lastTimeDisplayUpdateTime = currentMillis;
  }

  // 必要に応じて他の処理を追加
  delay(100); // 適度な遅延
}
