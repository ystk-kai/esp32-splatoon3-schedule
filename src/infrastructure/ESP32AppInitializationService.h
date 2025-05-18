// ESP32AppInitializationService.h
// ESP32用のアプリケーション初期化サービスの実装

#ifndef ESP32_APP_INITIALIZATION_SERVICE_H
#define ESP32_APP_INITIALIZATION_SERVICE_H

#include <Arduino.h>
#include <WiFi.h>
#include "../application/AppInitializationService.h"
#include "../application/WiFiConnectionManager.h"
#include "../application/DisplayService.h"
#include "../application/NetworkService.h"
#include "../application/ScheduleApplicationService.h"
#include "../application/SettingsService.h"
#include "../infrastructure/AppStateManager.h"

namespace Infrastructure
{
    // ESP32用のアプリケーション初期化サービスの実装
    class ESP32AppInitializationService : public Application::AppInitializationService
    {
    private:
        Application::NetworkService &networkService;
        Application::DisplayService &displayService;
        Application::ScheduleApplicationService &applicationService;
        Application::WiFiConnectionManager &wifiConnectionManager;
        Application::SettingsService &settingsService;
        AppStateManager &appStateManager;

    public:
        // コンストラクタ
        ESP32AppInitializationService(
            Application::NetworkService &networkService,
            Application::DisplayService &displayService,
            Application::ScheduleApplicationService &applicationService,
            Application::WiFiConnectionManager &wifiConnectionManager,
            Application::SettingsService &settingsService,
            AppStateManager &appStateManager)
            : networkService(networkService),
              displayService(displayService),
              applicationService(applicationService),
              wifiConnectionManager(wifiConnectionManager),
              settingsService(settingsService),
              appStateManager(appStateManager)
        {
        }

        // セットアップフェーズの処理
        void performSetup() override
        {
            // シリアル初期化（既にmainで初期化されている前提）
            Serial.println("\nSplatoon3 Schedule Viewer");
            Serial.println("Initializing...");

            // 画面反転設定をメモリに読み込む
            bool invertedDisplay = settingsService.loadInvertedDisplaySetting();

            // ディスプレイを初期化
            displayService.initialize();

            // 反転設定を適用
            displayService.invertDisplay(invertedDisplay);

            // 起動画面表示
            displayService.showStartupScreen();

            // 初期化の間隔を空ける
            delay(1000);

            // WiFi接続の初期セットアップ
            wifiConnectionManager.setupWiFiConnection();

            // WiFi接続が既に完了している場合は初期化を試みる
            if (wifiConnectionManager.isConnectionCompleted())
            {
                tryInitializeApp();
            }
        }

        // 定期実行されるロジック
        void performLoop(unsigned long currentMillis) override
        {
            // データが取得中かつ初期化されていない場合のデータ取得処理
            processDataFetching();

            // WiFi接続処理
            wifiConnectionManager.processWiFiConnection(currentMillis);

            // アプリケーションの初期化チェック
            if (!appStateManager.isAppInitialized() &&
                !appStateManager.getIsDataFetching() &&
                currentMillis - appStateManager.getLastInitializeCheckTime() >= appStateManager.getInitializeCheckInterval())
            {

                // 接続状態が変わったときだけ初期化を試行
                static bool lastConnectionState = false;
                bool currentConnectionState = wifiConnectionManager.isConnectionCompleted();

                if (currentConnectionState && !lastConnectionState)
                {
                    tryInitializeApp();
                }

                lastConnectionState = currentConnectionState;
                appStateManager.setLastInitializeCheckTime(currentMillis);
            }

            // アプリケーションが初期化済みの場合の定期更新処理
            if (appStateManager.isAppInitialized() && wifiConnectionManager.isConnectionCompleted())
            {
                // データの定期更新
                if (currentMillis - appStateManager.getLastDataUpdateTime() >= appStateManager.getDataUpdateInterval())
                {
                    Serial.println("Updating all schedule data...");
                    displayService.showLoadingMessage("Updating data...");
                    appStateManager.setIsDataFetching(true);
                    applicationService.updateAllData();
                    appStateManager.setIsDataFetching(false);
                    appStateManager.setLastDataUpdateTime(currentMillis);
                }

                // 時刻表示の更新
                if (currentMillis - appStateManager.getLastTimeDisplayUpdateTime() >= appStateManager.getTimeDisplayUpdateInterval())
                {
                    applicationService.updateTimeDisplay();
                    appStateManager.setLastTimeDisplayUpdateTime(currentMillis);
                }
            }
        }

        // アプリケーションの初期化を試みる
        bool tryInitializeApp() override
        {
            // すでに初期化済みなら何もしない
            if (appStateManager.isAppInitialized())
            {
                return true;
            }

            // WiFiが接続されていない場合は初期化しない
            if (!wifiConnectionManager.isConnectionCompleted())
            {
                return false;
            }

            Serial.println("Starting application initialization...");

            // 表示設定を読み込んでアプリケーションに適用
            Domain::DisplaySettings displaySettings = settingsService.loadDisplaySettings();
            applicationService.setDisplaySettings(displaySettings);

            // 画面反転設定を読み込む
            bool invertedDisplay = settingsService.loadInvertedDisplaySetting();
            Serial.print("Display invert setting loaded: ");
            Serial.println(invertedDisplay ? "enabled" : "disabled");

            // 表示状態をリセット
            displayService.resetDisplayState();

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
            WiFi.setAutoReconnect(true);

            // WiFi接続状態を確認・安定化
            if (!WiFi.isConnected())
            {
                Serial.println("WiFi接続が不安定です。初期化を中止します。");
                appStateManager.setIsDataFetching(false);
                return false;
            }

            // WiFi接続を維持するために十分な安定化時間
            delay(500);

            // 時刻サービスを設定
            networkService.configureTimeService();

            // 初期データ取得前の通知
            displayService.showLoadingMessage("Fetching data...");
            appStateManager.setIsDataFetching(true);

            Serial.println("Application initialization started");
            return true;
        }

    private:
        // データ取得処理
        void processDataFetching()
        {
            if (appStateManager.getIsDataFetching() && !appStateManager.isAppInitialized())
            {
                // データフェッチ前に再度接続確認
                if (wifiConnectionManager.getConnectionState() == Application::WiFiConnectionState::CONNECTED)
                {
                    // WiFi接続を安定化
                    WiFi.setAutoReconnect(true);
                    delay(300);

                    // 接続状態を再確認
                    if (wifiConnectionManager.getConnectionState() == Application::WiFiConnectionState::CONNECTED)
                    {
                        Serial.println("WiFi接続が安定しています。データ取得を開始します...");
                        applicationService.updateAllData();
                        appStateManager.setAppInitialized(true);
                        appStateManager.setIsDataFetching(false);
                        Serial.println("Application initialized successfully");
                    }
                    else
                    {
                        // 接続が不安定になった場合
                        Serial.println("WiFi connection became unstable. Retrying...");
                        appStateManager.setIsDataFetching(false);
                        wifiConnectionManager.resetConnectionState();
                    }
                }
                else
                {
                    // 接続が切れている場合はフラグをリセット
                    Serial.println("WiFi connection lost during initialization. Resetting...");
                    appStateManager.setIsDataFetching(false);
                    wifiConnectionManager.resetConnectionState();
                }
            }
        }
    };
}

#endif // ESP32_APP_INITIALIZATION_SERVICE_H