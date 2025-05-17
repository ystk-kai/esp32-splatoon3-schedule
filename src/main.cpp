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

// Infrastructure layer
#include "infrastructure/TFTDisplayService.h"
#include "infrastructure/ESP32NetworkService.h"
#include "infrastructure/APIScheduleRepository.h"
#include "infrastructure/ESP32WiFiService.h"

// Preferences for storing WiFi credentials
Preferences preferences;

// Display settings
#define TFT_BL 21 // バックライトピンをUser_Setup.hと同じに設定

// Application services
Infrastructure::ESP32NetworkService networkService;
Infrastructure::APIScheduleRepository scheduleRepository(networkService);
Application::ScheduleService scheduleService(scheduleRepository);
Infrastructure::TFTDisplayService displayService(TFT_BL, 0); // PWM Channel 0 for backlight control
Infrastructure::ESP32WiFiService wifiService;
Application::ScheduleApplicationService applicationService(scheduleService, displayService, networkService);

// Timer for periodic updates
unsigned long lastDataUpdateTime = 0;
unsigned long lastTimeDisplayUpdateTime = 0;
unsigned long lastWifiProcessTime = 0;
unsigned long lastInitializeCheckTime = 0;
unsigned long wifiSettingDisplayTime = 0;                     // WiFi設定表示の開始時間
unsigned long wifiConnectionStartTime = 0;                    // WiFi接続開始時間
const unsigned long DATA_UPDATE_INTERVAL = 5 * 60 * 1000;     // 5分ごとにデータを更新（5分 x 60秒 x 1000ms）
const unsigned long TIME_DISPLAY_UPDATE_INTERVAL = 10 * 1000; // 10秒ごとに時間表示を更新
const unsigned long WIFI_PROCESS_INTERVAL = 100;              // 100msごとにWiFi処理を実行
const unsigned long INITIALIZE_CHECK_INTERVAL = 1000;         // 1秒ごとに初期化チェック
const unsigned long WIFI_SETTING_DISPLAY_DURATION = 10000;    // WiFi設定表示時間を10秒に変更

// アプリケーション状態
bool appInitialized = false;
bool wifiSettingsDisplayed = false;      // WiFi設定情報が表示済みかどうか
bool waitingForPortalConnection = false; // キャプティブポータルへのアクセス待ち

// データ取得中かどうかのフラグ
bool isDataFetching = false;

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

// 表示設定を読み込む
Domain::DisplaySettings loadDisplaySettings()
{
  // Preferencesを開く
  preferences.begin("splatoon3", true); // 読み取り専用モード

  // 表示設定を読み込む
  bool useRomajiForBattleType = preferences.getBool("romaji_battle", true); // デフォルトはローマ字表示
  bool useRomajiForRule = preferences.getBool("romaji_rule", true);         // デフォルトはローマ字表示
  bool useRomajiForStage = preferences.getBool("romaji_stage", true);       // デフォルトはローマ字表示

  // Preferencesを閉じる
  preferences.end();

  // 表示設定を作成して返す
  return Domain::DisplaySettings::create(
      useRomajiForBattleType,
      useRomajiForRule,
      useRomajiForStage);
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

// アプリケーションの初期化を試みる
void tryInitializeApp()
{
  // すでに初期化済みなら何もしない
  if (appInitialized)
  {
    return;
  }

  // WiFiが接続されていない場合は初期化しない
  if (!wifiService.isConnected())
  {
    return;
  }

  // 表示設定を読み込む
  Domain::DisplaySettings displaySettings = loadDisplaySettings();
  applicationService.setDisplaySettings(displaySettings);

  // アプリケーションを初期化
  displayService.initialize();
  displayService.showStartupScreen();

  // 接続成功時にIPアドレスを表示
  String ipAddress = WiFi.localIP().toString();
  String ssid = WiFi.SSID();
  String connectionMessage = "Connected to WiFi!";
  connectionMessage += "\nSSID: " + ssid;
  connectionMessage += "\nIP: " + ipAddress;
  connectionMessage += "\nNetwork ready";
  displayService.showConnectionStatus(true, connectionMessage.c_str());

  // 時刻サービスを設定
  networkService.configureTimeService();

  // 初期データ取得
  displayService.showLoadingMessage("Fetching data...");
  isDataFetching = true; // データ取得フラグをオン
  // applicationService.updateAllData(); // ここでのデータ取得は移動し、loop内で実行

  // 初期化フラグのみを設定（データ取得はloop内で行う）
  // appInitialized = true; // ループ内でデータ取得完了後に設定する
  Serial.println("Application initialization started");
}

// 画面更新フラグをリセットする関数
void resetDisplayState()
{
  // TFTDisplayServiceクラス内の静的変数をリセットする
  displayService.resetDisplayState();
}

void setup()
{
  // シリアル初期化
  Serial.begin(115200);
  Serial.println("\nSplatoon3 Schedule Viewer");
  Serial.println("Initializing...");

  // ディスプレイを初期化して起動画面を表示
  displayService.initialize();
  displayService.showStartupScreen();

  // sleep 1 second
  delay(1000);

  // WiFiサービスをセットアップ
  Domain::WiFiSettings wifiSettings;
  bool hasWifiSettings = wifiService.loadSettings(wifiSettings);

  // キャプティブポータルの設定
  const char *apSSID = "ESP32-Splatoon3-Schedule";
  const char *apPassword = nullptr; // パスワードなしに設定

  if (hasWifiSettings)
  {
    Serial.println("WiFi settings loaded from preferences");
    // 既存のWiFi設定がある場合でも、キャプティブポータルを起動
    Serial.println("Starting captive portal for 10 seconds before connecting...");
    wifiService.startCaptivePortal(apSSID, apPassword);

    // キャプティブポータルのIPアドレスを表示
    String apIP = WiFi.softAPIP().toString();
    String portalMessage = "WiFi Setup Mode";
    portalMessage += "\nSSID: ";
    portalMessage += apSSID;
    portalMessage += "\nIP: " + apIP;
    portalMessage += "\nConnecting in 10s...";
    displayService.showConnectionStatus(false, portalMessage.c_str());

    // 表示時間を記録
    wifiSettingDisplayTime = millis();
    wifiSettingsDisplayed = true;
    waitingForPortalConnection = true;

    // この時点では接続は開始しない。loopで待機時間後に接続する
  }
  else
  {
    // 保存された設定がない場合はキャプティブポータルを起動
    Serial.println("No WiFi settings found. Starting captive portal.");
    wifiService.startCaptivePortal(apSSID, apPassword);

    // キャプティブポータルのIPアドレスを表示
    String apIP = WiFi.softAPIP().toString();
    String portalMessage = "WiFi Setup Mode";
    portalMessage += "\nSSID: ";
    portalMessage += apSSID;
    portalMessage += "\nIP: " + apIP;
    portalMessage += "\nWaiting for setup...";
    displayService.showConnectionStatus(false, portalMessage.c_str());

    waitingForPortalConnection = false; // 設定がないので無期限に待機
  }
}

void loop()
{
  unsigned long currentMillis = millis();

  if (isDataFetching && !appInitialized)
  {
    applicationService.updateAllData();
    appInitialized = true;
    isDataFetching = false; // データ取得完了
    Serial.println("Application initialized successfully");

    // 表示状態をリセット（接続状態の変更）
    resetDisplayState();
  }

  // WiFi処理（キャプティブポータルを含む）
  if (currentMillis - lastWifiProcessTime >= WIFI_PROCESS_INTERVAL)
  {
    wifiService.process();

    // キャプティブポータルが起動した場合、IPアドレスを表示
    if (wifiService.isCaptivePortalActive())
    {
      // キャプティブポータルにアクセスがあったかチェック
      if (wifiService.hasPortalConnections())
      {
        // アクセスがあった場合は、接続タイマーをリセット
        waitingForPortalConnection = true;
        wifiSettingDisplayTime = currentMillis; // 接続があった時点でタイマーをリセット

        // アクセスがあった場合は設定中メッセージを表示
        String apIP = WiFi.softAPIP().toString();
        String portalMessage = "WiFi Setup in Progress";
        portalMessage += "\nSSID: ESP32-Splatoon3-Schedule";
        portalMessage += "\nIP: " + apIP;
        portalMessage += "\nConfiguration active...";
        displayService.showConnectionStatus(false, portalMessage.c_str());
      }
      else if (waitingForPortalConnection && wifiSettingsDisplayed)
      {
        // カウントダウン表示
        int remainingSeconds = (WIFI_SETTING_DISPLAY_DURATION - (currentMillis - wifiSettingDisplayTime)) / 1000;
        if (remainingSeconds <= 0)
          remainingSeconds = 0;

        String apIP = WiFi.softAPIP().toString();
        String portalMessage = "WiFi Setup Mode";
        portalMessage += "\nSSID: ESP32-Splatoon3-Schedule";
        portalMessage += "\nIP: " + apIP;
        portalMessage += "\nConnecting in " + String(remainingSeconds) + "s...";
        displayService.showConnectionStatus(false, portalMessage.c_str());
      }
      else
      {
        // 通常の待機表示
        String apIP = WiFi.softAPIP().toString();
        String portalMessage = "WiFi Setup Mode";
        portalMessage += "\nSSID: ESP32-Splatoon3-Schedule";
        portalMessage += "\nIP: " + apIP;
        portalMessage += "\nWaiting for setup...";
        displayService.showConnectionStatus(false, portalMessage.c_str());
      }
    }

    lastWifiProcessTime = currentMillis;
  }

  // 既存のWiFi設定がある場合、表示時間が経過したら接続を開始
  if (wifiSettingsDisplayed && !wifiService.isConnected() && wifiConnectionStartTime == 0)
  {
    // すでに待機状態で、かつポータルへの接続がない場合のみタイムアウト処理
    if (waitingForPortalConnection && currentMillis - wifiSettingDisplayTime >= WIFI_SETTING_DISPLAY_DURATION)
    {
      // 待機時間が経過したのでキャプティブポータルを停止し、WiFi接続を開始
      Serial.println("Timeout reached. Stopping captive portal and connecting to WiFi...");
      wifiService.stopCaptivePortal();

      // WiFi設定情報の表示が十分行われたので、接続を開始
      Domain::WiFiSettings wifiSettings;
      wifiService.loadSettings(wifiSettings);
      wifiService.connect(wifiSettings);

      // 接続開始時間を記録
      wifiConnectionStartTime = currentMillis;

      // 接続中表示に切り替え
      Domain::WiFiSettings ws;
      wifiService.loadSettings(ws);
      String ssid = ws.getSsid();
      String connectingMessage = "Connecting to WiFi";
      connectingMessage += "\nSSID: " + ssid;
      connectingMessage += "\nPlease wait...";
      displayService.showConnectionStatus(false, connectingMessage.c_str());

      // 待機フラグをクリア
      waitingForPortalConnection = false;
    }
  }

  // アプリケーションの初期化チェック
  if (currentMillis - lastInitializeCheckTime >= INITIALIZE_CHECK_INTERVAL)
  {
    tryInitializeApp();
    lastInitializeCheckTime = currentMillis;
  }

  // アプリケーションが初期化済みかつWiFiが接続されている場合のみ、データ更新とディスプレイ更新を行う
  if (appInitialized && wifiService.isConnected())
  {
    // データの定期更新（5分ごと）
    if (currentMillis - lastDataUpdateTime >= DATA_UPDATE_INTERVAL)
    {
      Serial.println("Updating all schedule data...");
      displayService.showLoadingMessage("Updating data...");
      isDataFetching = true; // データ取得フラグをオン
      applicationService.updateAllData();
      isDataFetching = false; // データ取得完了
      lastDataUpdateTime = currentMillis;
    }

    // 時刻表示の更新（10秒ごと）
    if (currentMillis - lastTimeDisplayUpdateTime >= TIME_DISPLAY_UPDATE_INTERVAL)
    {
      applicationService.updateTimeDisplay();
      lastTimeDisplayUpdateTime = currentMillis;
    }
  }

  // 必要に応じて他の処理を追加
  delay(10); // 適度な遅延
}
