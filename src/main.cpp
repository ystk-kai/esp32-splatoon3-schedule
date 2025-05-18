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
const unsigned long WIFI_SETTING_DISPLAY_DURATION = 15000;    // WiFi設定表示時間を15秒に設定

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

// 画面反転設定を読み込む
bool loadInvertedDisplaySetting()
{
  // Preferencesを開く
  preferences.begin("splatoon3", true); // 読み取り専用モード

  // 反転設定を読み込む
  bool inverted = preferences.getBool("inv_disp", false);

  Serial.print("画面反転設定を読み込み: ");
  Serial.println(inverted ? "有効" : "無効");

  // Preferencesを閉じる
  preferences.end();

  return inverted;
}

// 画面反転設定を保存する
bool saveInvertedDisplaySetting(bool inverted)
{
  Serial.print("画面反転設定を保存: ");
  Serial.println(inverted ? "有効" : "無効");

  // Preferencesを開く
  preferences.begin("splatoon3", false); // 書き込みモード

  // 反転設定を保存
  bool result = preferences.putBool("inv_disp", inverted);

  // Preferencesを閉じる
  preferences.end();

  return result;
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

// 画面更新フラグをリセットする関数
void resetDisplayState()
{
  // TFTDisplayServiceクラス内の静的変数をリセットする
  displayService.resetDisplayState();
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

  Serial.println("Starting application initialization...");

  // デバイスの基本設定
  // 表示設定を読み込む
  Domain::DisplaySettings displaySettings = loadDisplaySettings();
  applicationService.setDisplaySettings(displaySettings);

  // 画面反転設定を読み込む
  bool invertedDisplay = loadInvertedDisplaySetting();
  Serial.print("Display invert setting loaded: ");
  Serial.println(invertedDisplay ? "enabled" : "disabled");

  // 表示状態をリセット（先にリセットして余計な画面更新を防止）
  resetDisplayState();

  // ディスプレイは setup() で既に初期化済みなので再初期化しない
  // 反転設定のみ適用
  displayService.invertDisplay(invertedDisplay);

  // 接続成功時の表示
  String ipAddress = WiFi.localIP().toString();
  String ssid = WiFi.SSID();
  String connectionMessage = "Connection OK";
  connectionMessage += "\nSSID: " + ssid;
  connectionMessage += "\nIP: " + ipAddress;
  connectionMessage += "\nInitializing...";
  displayService.showConnectionStatus(true, connectionMessage.c_str());

  // 再接続のための安全対策
  WiFi.setAutoReconnect(true); // 自動再接続を有効化

  // WiFi接続状態を確認・安定化
  if (!WiFi.isConnected())
  {
    Serial.println("WiFi接続が切断されました。再接続を試みます...");
    // 保存済みのWiFi設定を使用して再接続
    Domain::WiFiSettings wifiSettings;
    if (wifiService.loadSettings(wifiSettings) && wifiSettings.isValid())
    {
      wifiService.connect(wifiSettings);
      delay(500); // 接続処理の開始を待機
    }

    // 接続を再確認
    if (!WiFi.isConnected())
    {
      Serial.println("再接続に失敗しました。初期化を中止します。");
      isDataFetching = false;
      return; // 接続できないため初期化中止
    }
  }

  // WiFi接続を維持するために十分な安定化時間
  delay(500);

  // 時刻サービスを設定
  networkService.configureTimeService();

  // 初期データ取得前の通知
  displayService.showLoadingMessage("Fetching data...");
  isDataFetching = true; // データ取得フラグをオン
  // データ取得はloop内で実行される

  Serial.println("Application initialization started");
}

void setup()
{
  // シリアル初期化
  Serial.begin(115200);
  Serial.println("\nSplatoon3 Schedule Viewer");
  Serial.println("Initializing...");

  // 画面反転設定をメモリに読み込む（先に実行）
  bool invertedDisplay = loadInvertedDisplaySetting();

  // ディスプレイを初期化
  displayService.initialize();

  // 反転設定を適用（初期化後に適用）
  displayService.invertDisplay(invertedDisplay);

  // 起動画面表示（反転設定後に表示）
  displayService.showStartupScreen();

  // sleep 1 second
  delay(1000);

  // WiFi状態の初期化（重要：WiFiは起動時にSTA_APモードを設定）
  WiFi.mode(WIFI_STA);
  delay(500);

  // 現在のWiFi接続状態を確認
  if (WiFi.status() == WL_CONNECTED)
  {
    Serial.println("起動時にWiFiが既に接続されています");
    // 接続済みであれば直接起動プロセスへ進む
    String ipAddress = WiFi.localIP().toString();
    String ssid = WiFi.SSID();
    String connectingMessage = "Connection OK";
    connectingMessage += "\nSSID: " + ssid;
    connectingMessage += "\nIP: " + ipAddress;
    connectingMessage += "\nInitializing...";
    displayService.showConnectionStatus(true, connectingMessage.c_str());

    // 即座に初期化開始
    WiFi.setAutoReconnect(true);
    delay(500);
    tryInitializeApp();
    return;
  }

  // WiFiサービスをセットアップ
  Domain::WiFiSettings wifiSettings;
  bool hasWifiSettings = wifiService.loadSettings(wifiSettings);

  // キャプティブポータルの設定
  const char *apSSID = "ESP32-Splatoon3-Schedule";
  const char *apPassword = nullptr; // パスワードなしに設定

  if (hasWifiSettings)
  {
    Serial.println("WiFi settings loaded from preferences");
    // WiFiの接続状態を検証（既に接続されている場合はポータルをスキップ）
    if (WiFi.status() == WL_CONNECTED)
    {
      Serial.println("WiFi already connected, skipping captive portal");

      // 接続情報を表示して初期化へ進む
      String ipAddress = WiFi.localIP().toString();
      String ssid = WiFi.SSID();
      String connectingMessage = "Connection OK";
      connectingMessage += "\nSSID: " + ssid;
      connectingMessage += "\nIP: " + ipAddress;
      connectingMessage += "\nInitializing...";
      displayService.showConnectionStatus(true, connectingMessage.c_str());

      WiFi.setAutoReconnect(true);
      delay(500);
      tryInitializeApp();
      return;
    }

    // 既存のWiFi設定があるけど接続されていない場合、キャプティブポータルを起動
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

    // カウントダウン開始ログ
    Serial.println("【カウントダウン開始】15秒後に自動接続を開始します");
    Serial.println("理由: WiFi設定があり、キャプティブポータルを一時的に表示");

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

  // データが取得中かつ初期化されていない場合は、データを取得して表示を更新
  if (isDataFetching && !appInitialized)
  {
    // データフェッチ前に再度接続確認
    if (wifiService.isConnected())
    {
      // WiFi接続を安定化
      WiFi.setAutoReconnect(true);

      // 念のため追加の安定化遅延
      delay(300);

      // 接続状態を再確認
      if (wifiService.isConnected())
      {
        Serial.println("WiFi接続が安定しています。データ取得を開始します...");
        applicationService.updateAllData();
        appInitialized = true;
        isDataFetching = false; // データ取得完了
        Serial.println("Application initialized successfully");
      }
      else
      {
        // 接続が不安定になった場合
        Serial.println("WiFi connection became unstable. Retrying...");
        isDataFetching = false;

        // 接続を再試行
        Domain::WiFiSettings wifiSettings;
        if (wifiService.loadSettings(wifiSettings) && wifiSettings.isValid())
        {
          Serial.println("Attempting to reconnect with saved settings...");
          wifiService.connect(wifiSettings);
          delay(1000); // 接続処理の開始を待機
        }
      }
    }
    else
    {
      // 接続が切れている場合はフラグをリセットして再接続を試みる
      Serial.println("WiFi connection lost during initialization. Resetting...");
      isDataFetching = false;

      // 接続を再試行するため、状態をリセット
      Domain::WiFiSettings wifiSettings;
      if (wifiService.loadSettings(wifiSettings) && wifiSettings.isValid())
      {
        Serial.println("Attempting to reconnect with saved settings...");
        wifiService.connect(wifiSettings);
        delay(1000); // 接続処理の開始を待機
      }
    }
  }

  // WiFi処理（キャプティブポータルを含む）
  if (currentMillis - lastWifiProcessTime >= WIFI_PROCESS_INTERVAL)
  {
    // デバッグ: 現在のフラグ状態をログ出力
    static unsigned long lastDebugLogTime = 0;
    if (currentMillis - lastDebugLogTime >= 1000)
    { // 1秒ごとにログ出力
      // ポータル接続状態を確認
      bool hasPortalConnection = wifiService.hasPortalConnections();

      // ポータル接続中は詳細なデバッグログを出力しない
      if (!hasPortalConnection || !wifiService.isCaptivePortalActive())
      {
        Serial.println("===== WiFi状態のデバッグ情報 =====");
        Serial.print("waitingForPortalConnection: ");
        Serial.println(waitingForPortalConnection ? "true" : "false");
        Serial.print("wifiSettingsDisplayed: ");
        Serial.println(wifiSettingsDisplayed ? "true" : "false");
        Serial.print("WiFi接続状態: ");
        Serial.println(WiFi.status() == WL_CONNECTED ? "接続済み" : "未接続");
        Serial.print("isDataFetching: ");
        Serial.println(isDataFetching ? "true" : "false");
        Serial.print("appInitialized: ");
        Serial.println(appInitialized ? "true" : "false");

        // カウントダウン関連情報（ポータル接続は上でチェック済み）
        if (wifiSettingsDisplayed && waitingForPortalConnection)
        {
          int remainingSeconds = (WIFI_SETTING_DISPLAY_DURATION - (currentMillis - wifiSettingDisplayTime)) / 1000;
          Serial.print("カウントダウン残り: ");
          Serial.print(remainingSeconds);
          Serial.println("秒");
        }

        Serial.println("================================");
      }
      lastDebugLogTime = currentMillis;
    }

    // WiFiサービスの処理を実行
    wifiService.process();

    // WiFi接続状態をチェック
    static bool lastWiFiConnected = false;
    bool currentWiFiConnected = wifiService.isConnected() || WiFi.status() == WL_CONNECTED;

    // WiFiが新たに接続された場合（未接続→接続）
    if (currentWiFiConnected && !lastWiFiConnected)
    {
      Serial.println("============================================");
      Serial.println("【重要】WiFi接続が検出されました!");
      Serial.println("SSID: " + WiFi.SSID());
      Serial.println("IP: " + WiFi.localIP().toString());

      // キャプティブポータルへの接続があるかどうかを確認
      bool hasPortalConnection = wifiService.hasPortalConnections();

      if (hasPortalConnection)
      {
        // ポータル接続中はWiFi接続があっても設定画面を維持
        Serial.println("ポータル接続中のためWiFi接続を検出しましたが、設定画面を維持します");
        Serial.println("ブラウザを閉じるとカウントダウンを再開し、WiFi接続へ進みます");

        // カウントダウンは停止するが、フラグは維持
        wifiSettingDisplayTime = currentMillis + (WIFI_SETTING_DISPLAY_DURATION * 8);

        Serial.println("============================================");
        return; // フローチャート通りポータル接続中はWiFi接続処理をスキップ
      }

      // ポータル接続がない場合は通常のWiFi接続処理を継続
      Serial.println("ポータル接続がないため、カウントダウンを含む全てのタイマーを即時停止します");

      // カウントダウン状態のログ
      if (waitingForPortalConnection)
      {
        Serial.println("【カウントダウン中断】WiFi接続により中断されました");
        int remainingSeconds = (WIFI_SETTING_DISPLAY_DURATION - (currentMillis - wifiSettingDisplayTime)) / 1000;
        Serial.print("残り時間: ");
        Serial.print(remainingSeconds);
        Serial.println("秒でした");
      }

      // この時点で全てのカウントダウン関連フラグを確実にリセットする
      // これによりカウントダウンは即時停止する必要がある
      waitingForPortalConnection = false;
      wifiSettingsDisplayed = false;
      wifiSettingDisplayTime = 0;
      wifiConnectionStartTime = 0;

      Serial.println("カウントダウンフラグをリセットしました");

      // ポータル検出フラグも確実にリセット（ポータル接続がない場合のみ）
      if (wifiService.isCaptivePortalActive())
      {
        Serial.println("キャプティブポータルを即時停止します（WiFi接続検出のため）");
        wifiService.stopCaptivePortal();
      }

      Serial.println("============================================");

      // WiFi設定表示中の場合、カウントダウンを停止
      if (wifiSettingsDisplayed)
      {
        Serial.println("Stopping countdown due to WiFi connection");
        // キャプティブポータルを停止
        if (wifiService.isCaptivePortalActive())
        {
          Serial.println("Stopping captive portal due to WiFi connection");
          wifiService.stopCaptivePortal();
        }

        // カウントダウン関連のフラグをすべて確実にリセット
        Serial.println("明示的にすべてのカウントダウン関連フラグをリセットします");
        waitingForPortalConnection = false;
        wifiSettingsDisplayed = false;

        // 接続開始時間をリセット (カウントダウンを確実に停止するため)
        wifiSettingDisplayTime = 0;
        wifiConnectionStartTime = 0; // カウントダウンを完全に停止

        // ポータルフラグも明示的にリセット
        wifiService.resetPortalConnectionDetected();

        // 接続成功の表示
        String ipAddress = WiFi.localIP().toString();
        String ssid = WiFi.SSID();
        String connectingMessage = "Connection OK";
        connectingMessage += "\nSSID: " + ssid;
        connectingMessage += "\nIP: " + ipAddress;
        connectingMessage += "\nPlease wait...";
        displayService.showConnectionStatus(true, connectingMessage.c_str());

        // 表示を安定させ、WiFi接続を確実にするために十分待機
        delay(1000);

        // WiFi接続状態を再確認
        if (wifiService.isConnected())
        {
          Serial.println("WiFi接続が安定しています。アプリケーション初期化を開始します...");

          // WiFi接続を安定化
          WiFi.setAutoReconnect(true);

          // アプリケーションの初期化を開始（未初期化かつデータ取得中でない場合）
          if (!appInitialized && !isDataFetching)
          {
            tryInitializeApp();
          }
        }
        else
        {
          Serial.println("WiFi接続が不安定です。再接続を試みます...");
          // 既存のWiFi設定で再接続
          Domain::WiFiSettings wifiSettings;
          if (wifiService.loadSettings(wifiSettings) && wifiSettings.isValid())
          {
            wifiService.connect(wifiSettings);
          }
        }
      }
    }

    lastWiFiConnected = currentWiFiConnected;

    // キャプティブポータルが起動している場合
    if (wifiService.isCaptivePortalActive())
    {
      // ポータル接続状態を必ず最初に確認する
      bool hasPortalConnection = wifiService.hasPortalConnections();

      // ポータル接続がある場合は、WiFi接続状態に関わらず、設定中画面を優先
      if (hasPortalConnection)
      {
        // 設定中画面を表示し続ける
        String apIP = WiFi.softAPIP().toString();
        String portalMessage = "WiFi Setup in Progress";
        portalMessage += "\nSSID: ESP32-Splatoon3-Schedule";
        portalMessage += "\nIP: " + apIP;
        portalMessage += "\nConfiguration in progress...";
        displayService.showConnectionStatus(false, portalMessage.c_str());
        lastWifiProcessTime = currentMillis;

        // スパムログを防ぐためにフラグを使用
        static bool loggedSkipMessage = false;
        if (!loggedSkipMessage)
        {
          Serial.println("ポータル接続中のため、WiFi接続状態の処理をスキップします");
          loggedSkipMessage = true;
        }
        return;
      }
      else
      {
        // ポータル接続が終了したらログフラグをリセット
        static bool loggedSkipMessage = false;
        loggedSkipMessage = false;
      }

      // ポータル接続がない場合のみWiFi接続状態を確認
      // ポータル接続中はWiFi接続があっても設定画面を維持する（フローチャート通り）
      if (WiFi.status() == WL_CONNECTED && !hasPortalConnection)
      {
        Serial.println("WiFi connected while portal active - stopping portal only if no portal connection");

        // ポータル接続がない場合のみポータルを停止してアプリ初期化へ進む
        Serial.println("No portal connection detected, proceeding to stop portal and connect");
        wifiService.stopCaptivePortal();
        waitingForPortalConnection = false;
        wifiSettingsDisplayed = false;
        wifiSettingDisplayTime = 0;
        wifiConnectionStartTime = 0;

        // 接続成功の表示
        String ipAddress = WiFi.localIP().toString();
        String ssid = WiFi.SSID();
        String connectingMessage = "Connection OK";
        connectingMessage += "\nSSID: " + ssid;
        connectingMessage += "\nIP: " + ipAddress;
        connectingMessage += "\nPlease wait...";
        displayService.showConnectionStatus(true, connectingMessage.c_str());

        // WiFi接続の安定化を待つ
        delay(1000);

        // アプリケーション初期化へ進む
        if (!appInitialized && !isDataFetching)
        {
          tryInitializeApp();
        }

        lastWifiProcessTime = currentMillis;
        return; // ループを抜けて最初から再実行
      }
      else if (WiFi.status() == WL_CONNECTED && hasPortalConnection)
      {
        // WiFi接続があってもポータル接続中は設定画面を維持
        Serial.println("WiFi connected but portal connection active - maintaining setup screen");
      }

      // キャプティブポータルにアクセスがあったかチェック
      static bool lastPortalConnectionState = false;
      // 前回表示した秒数
      static int lastDisplayedSeconds = -1;

      bool hasPortalConnections = wifiService.hasPortalConnections();

      // アクセスがあった場合は、接続タイマーをリセット
      if (hasPortalConnections && !lastPortalConnectionState)
      {
        lastPortalConnectionState = true;
        waitingForPortalConnection = true;
        // カウントダウンを十分先の未来に設定して確実に停止する
        // 通常より長いタイムアウト（2分）を設定し、ブラウザを閉じるまで設定画面を維持
        wifiSettingDisplayTime = currentMillis + (WIFI_SETTING_DISPLAY_DURATION * 8);

        // アクセスがあった場合は設定中メッセージを表示
        String apIP = WiFi.softAPIP().toString();
        String portalMessage = "WiFi Setup in Progress";
        portalMessage += "\nSSID: ESP32-Splatoon3-Schedule";
        portalMessage += "\nIP: " + apIP;
        portalMessage += "\nConfiguration in progress...";
        displayService.showConnectionStatus(false, portalMessage.c_str());

        Serial.println("キャプティブポータルに接続があり、設定中画面を表示します (カウントダウン停止)");
        Serial.println("タイマー値を未来（2分先）に設定することでカウントダウン表示を一時停止します");
      }
      else if (!hasPortalConnections && lastPortalConnectionState)
      {
        // アクセスがなくなった場合
        lastPortalConnectionState = false;

        // ここでタイマーを現在時刻に更新してカウントダウンを再開
        wifiSettingDisplayTime = currentMillis;

        Serial.println("ポータル接続が終了し、カウントダウンを再開します");
        Serial.println("タイマーを現在時刻に更新しました、残り15秒からカウントダウンを再開します");

        if (waitingForPortalConnection && wifiSettingsDisplayed)
        {
          // カウントダウン表示（初期表示）
          int remainingSeconds = WIFI_SETTING_DISPLAY_DURATION / 1000; // 新しいカウントダウン（最初から）
          lastDisplayedSeconds = remainingSeconds;

          String apIP = WiFi.softAPIP().toString();
          String portalMessage = "WiFi Setup Mode";
          portalMessage += "\nSSID: ESP32-Splatoon3-Schedule";
          portalMessage += "\nIP: " + apIP;
          portalMessage += "\nConnecting in " + String(remainingSeconds) + "s...";
          displayService.showConnectionStatus(false, portalMessage.c_str());
        }
      }
      else if (waitingForPortalConnection && wifiSettingsDisplayed)
      {
        // WiFi接続状態を複数の方法で確認（二重チェック）
        bool reallyConnected = wifiService.isConnected() || WiFi.status() == WL_CONNECTED;

        // WiFi接続とポータル接続状態を両方確認
        if (reallyConnected && !hasPortalConnections)
        {
          // ポータル接続がない場合のみWiFi接続に切り替える
          Serial.println("WiFi connected during countdown and no portal connection. Proceeding to connect.");

          // すべてのフラグを確実にリセット
          waitingForPortalConnection = false;
          wifiSettingsDisplayed = false;
          wifiSettingDisplayTime = 0;
          wifiConnectionStartTime = 0;

          // キャプティブポータルが実行中なら停止
          if (wifiService.isCaptivePortalActive())
          {
            Serial.println("Stopping active captive portal due to WiFi connection");
            wifiService.stopCaptivePortal();
            delay(500); // ポータル停止の安定化を待つ
          }

          // 接続成功の表示
          String ipAddress = WiFi.localIP().toString();
          String ssid = WiFi.SSID();
          String connectingMessage = "Connection OK";
          connectingMessage += "\nSSID: " + ssid;
          connectingMessage += "\nIP: " + ipAddress;
          connectingMessage += "\nPlease wait...";
          displayService.showConnectionStatus(true, connectingMessage.c_str());

          // 表示とWiFi接続を安定させるために待機
          delay(1000);

          // アプリケーション初期化を開始
          if (!appInitialized && !isDataFetching)
          {
            tryInitializeApp();
          }

          // ループを終了して最初から再実行
          lastWifiProcessTime = currentMillis;
          return;
        }
        else if (reallyConnected && hasPortalConnections)
        {
          // WiFi接続があってもポータル接続中は設定画面を維持（フローチャート通り）
          Serial.println("WiFi connected during countdown but portal connection active - maintaining setup screen");
          // カウントダウンを長時間停止するだけで接続画面には遷移しない
          wifiSettingDisplayTime = currentMillis + (WIFI_SETTING_DISPLAY_DURATION * 8);
        }
        // ポータルへの接続が検出されていればカウントダウンを停止して設定中表示に
        else if (wifiService.hasPortalConnections())
        {
          // ポータル接続中の場合はカウントダウンを表示しない
          String apIP = WiFi.softAPIP().toString();
          String portalMessage = "WiFi Setup in Progress";
          portalMessage += "\nSSID: ESP32-Splatoon3-Schedule";
          portalMessage += "\nIP: " + apIP;
          portalMessage += "\nConfiguration in progress...";
          displayService.showConnectionStatus(false, portalMessage.c_str());

          // カウントダウンを長時間停止するためのタイマー設定（2分先）
          wifiSettingDisplayTime = currentMillis + (WIFI_SETTING_DISPLAY_DURATION * 8);

          if (!lastPortalConnectionState)
          {
            lastPortalConnectionState = true;
            Serial.println("接続検出によりカウントダウンを停止し、設定中表示に切り替えました");
          }
        }
        else
        {
          // 接続がない場合のみカウントダウンを表示（lastPortalConnectionStateは念のためfalseに）
          lastPortalConnectionState = false;

          // カウントダウン中は1秒ごとに表示を更新
          int remainingSeconds = (WIFI_SETTING_DISPLAY_DURATION - (currentMillis - wifiSettingDisplayTime)) / 1000;
          if (remainingSeconds < 0)
            remainingSeconds = 0;

          // カウントダウンが0になったが、portalConnectionDetectedがtrueのままの場合、
          // 表示は維持するが、カウントダウンをリセットしてすぐに接続処理へ移行しないようにする
          if (remainingSeconds <= 0)
          {
            // 0秒になったらportalConnectionDetectedを改めてチェック
            bool stillConnected = wifiService.hasPortalConnections();
            if (stillConnected)
            {
              // まだポータル接続中なら、タイマーを延長して「0秒」表示にならないようにする
              Serial.println("カウントダウンが0になりましたが、まだポータル接続中です");
              Serial.println("カウントダウン表示を15秒にリセットします");
              wifiSettingDisplayTime = currentMillis;
              remainingSeconds = WIFI_SETTING_DISPLAY_DURATION / 1000;
            }
            else
            {
              // 接続がなければ0秒表示を許可（この後のif分岐でタイムアウト処理に進む）
              Serial.println("カウントダウンが0になり、ポータル接続もありません");
              Serial.println("WiFi接続プロセスに進みます");
            }
          }

          // 前回表示した秒数と異なる場合のみ更新
          if (remainingSeconds != lastDisplayedSeconds)
          {
            // カウントダウン経過ログ (0, 5, 10, 15秒のみ詳細表示)
            if (remainingSeconds == 15 || remainingSeconds == 10 ||
                remainingSeconds == 5 || remainingSeconds == 0)
            {
              Serial.print("【カウントダウン進行中】残り ");
              Serial.print(remainingSeconds);
              Serial.println(" 秒");
            }

            lastDisplayedSeconds = remainingSeconds;

            String apIP = WiFi.softAPIP().toString();
            String portalMessage = "WiFi Setup Mode";
            portalMessage += "\nSSID: ESP32-Splatoon3-Schedule";
            portalMessage += "\nIP: " + apIP;
            portalMessage += "\nConnecting in " + String(remainingSeconds) + "s...";
            displayService.showConnectionStatus(false, portalMessage.c_str());
          }
        }
      }
    }
    // WiFiが接続されたら、一度だけアプリケーション初期化を試みる
    else if (wifiService.isConnected() && !appInitialized && !isDataFetching)
    {
      // カウントダウンなどのフラグをリセット
      waitingForPortalConnection = false;
      wifiSettingsDisplayed = false;
      wifiConnectionStartTime = 0;

      // アプリケーションの初期化を開始
      tryInitializeApp();
    }

    lastWifiProcessTime = currentMillis;
  }

  // WiFi接続状態を二重チェックして確実に接続状態を把握
  bool isWifiReallyConnected = wifiService.isConnected() || WiFi.status() == WL_CONNECTED;

  // 既存のWiFi設定がある場合、表示時間が経過したら接続を開始
  if (wifiSettingsDisplayed && !isWifiReallyConnected && wifiConnectionStartTime == 0 && waitingForPortalConnection)
  {
    // 接続状態の再検証（接続していたらタイムアウト処理をスキップ）
    if (isWifiReallyConnected)
    {
      Serial.println("WiFi接続が確認されたため、タイムアウト処理をスキップします");
      waitingForPortalConnection = false;
      wifiSettingsDisplayed = false;
      wifiSettingDisplayTime = 0;
      wifiConnectionStartTime = 0;
      return;
    }

    // すでに待機状態で、かつポータルへの接続がない場合のみタイムアウト処理
    if (currentMillis - wifiSettingDisplayTime >= WIFI_SETTING_DISPLAY_DURATION)
    {
      // WiFiが接続されていないことを再度確認
      if (!wifiService.isConnected())
      {
        // ポータル接続状態を再確認（ポータル接続中はタイムアウト処理をスキップ）
        bool hasPortalConnection = wifiService.hasPortalConnections();

        if (hasPortalConnection)
        {
          // ログスパム防止のためのフラグ
          static bool loggedExtension = false;

          if (!loggedExtension)
          {
            // ポータル接続中はカウントダウンを再延長（正確には停止）して処理をスキップ
            Serial.println("============================================");
            Serial.println("【カウントダウン停止】ポータル接続中のためカウントダウンを停止します");
            Serial.println("============================================");
            loggedExtension = true;
          }

          // カウントダウンを停止（十分に未来の時間を設定）
          wifiSettingDisplayTime = currentMillis + (WIFI_SETTING_DISPLAY_DURATION * 8);
          return;
        }
        else
        {
          // ポータル接続がなくなったらフラグをリセット
          static bool loggedExtension = false;
          loggedExtension = false;
        }

        // 待機時間が経過したのでキャプティブポータルを停止し、WiFi接続を開始
        Serial.println("============================================");
        Serial.println("【カウントダウン終了】15秒のタイムアウトに達しました");
        Serial.println("キャプティブポータルを停止し、WiFi接続を開始します");
        wifiService.stopCaptivePortal();

        // WiFi設定情報の表示が十分行われたので、接続を開始
        Domain::WiFiSettings wifiSettings;
        wifiService.loadSettings(wifiSettings);
        Serial.print("接続先SSID: ");
        Serial.println(wifiSettings.getSsid());
        wifiService.connect(wifiSettings);

        // 接続開始時間を記録
        wifiConnectionStartTime = currentMillis;
        Serial.println("============================================");

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
      else
      {
        // WiFi接続が既に確立している場合は待機フラグをクリア
        Serial.println("WiFi already connected at timeout check. Skipping connection attempt.");
        waitingForPortalConnection = false;
        wifiSettingsDisplayed = false;
      }
    }
  }

  // アプリケーションの初期化チェック - 1秒に1回だけチェック
  if (!appInitialized && !isDataFetching && currentMillis - lastInitializeCheckTime >= INITIALIZE_CHECK_INTERVAL)
  {
    // 接続状態が変わったときだけ初期化を試行（無駄な呼び出しを防ぐ）
    static bool lastConnectionState = false;
    bool currentConnectionState = wifiService.isConnected();

    if (currentConnectionState && !lastConnectionState)
    {
      tryInitializeApp();
    }

    lastConnectionState = currentConnectionState;
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
