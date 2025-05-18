// ESP32WiFiConnectionManager.h
// ESP32用のWiFi接続管理サービスの実装

#ifndef ESP32_WIFI_CONNECTION_MANAGER_H
#define ESP32_WIFI_CONNECTION_MANAGER_H

#include <Arduino.h>
#include <WiFi.h>
#include "../application/WiFiConnectionManager.h"
#include "../application/WiFiService.h"
#include "../application/DisplayService.h"
#include "../infrastructure/AppStateManager.h"

namespace Infrastructure
{
    // ESP32用のWiFi接続管理サービスの実装
    class ESP32WiFiConnectionManager : public Application::WiFiConnectionManager
    {
    private:
        Application::WiFiService &wifiService;
        Application::DisplayService &displayService;
        AppStateManager &appStateManager;

        // WiFi接続関連の定数
        static constexpr const char *AP_SSID = "ESP32-Splatoon3-Schedule";
        static constexpr const char *AP_PASSWORD = nullptr; // パスワードなし

        // 内部状態
        Application::WiFiConnectionState currentState;
        bool connectionCompleted;

    public:
        // コンストラクタ
        ESP32WiFiConnectionManager(
            Application::WiFiService &wifiService,
            Application::DisplayService &displayService,
            AppStateManager &appStateManager)
            : wifiService(wifiService),
              displayService(displayService),
              appStateManager(appStateManager),
              currentState(Application::WiFiConnectionState::DISCONNECTED),
              connectionCompleted(false)
        {
        }

        // WiFi接続の初期セットアップ
        void setupWiFiConnection() override
        {
            // WiFi状態の初期化
            WiFi.mode(WIFI_STA);
            delay(500);

            // 現在のWiFi接続状態を確認
            if (WiFi.status() == WL_CONNECTED)
            {
                Serial.println("起動時にWiFiが既に接続されています");
                showConnectedStatus();

                // 接続完了フラグを設定
                connectionCompleted = true;
                currentState = Application::WiFiConnectionState::CONNECTED;

                // 即座に初期化開始フラグを設定
                WiFi.setAutoReconnect(true);
                return;
            }

            // WiFi設定の読み込み
            Domain::WiFiSettings wifiSettings;
            bool hasWifiSettings = wifiService.loadSettings(wifiSettings);

            if (hasWifiSettings)
            {
                Serial.println("WiFi settings loaded from preferences");
                // 既存の設定があるが接続されていない場合はキャプティブポータルを起動
                startCaptivePortal();
            }
            else
            {
                // 保存された設定がない場合はキャプティブポータルを無期限に起動
                Serial.println("No WiFi settings found. Starting captive portal.");
                wifiService.startCaptivePortal(AP_SSID, AP_PASSWORD);

                // キャプティブポータルのIPアドレスを表示
                String apIP = WiFi.softAPIP().toString();
                String portalMessage = "WiFi Setup Mode";
                portalMessage += "\nSSID: ";
                portalMessage += AP_SSID;
                portalMessage += "\nIP: " + apIP;
                portalMessage += "\nWaiting for setup...";
                displayService.showConnectionStatus(false, portalMessage.c_str());

                // 待機フラグをセット
                appStateManager.setWaitingForPortalConnection(false); // 設定がないので無期限に待機
                currentState = Application::WiFiConnectionState::PORTAL_ACTIVE;
            }
        }

        // キャプティブポータルの起動
        bool startCaptivePortal() override
        {
            Serial.println("Starting captive portal for setup...");
            bool result = wifiService.startCaptivePortal(AP_SSID, AP_PASSWORD);

            if (result)
            {
                // キャプティブポータルのIPアドレスを表示
                String apIP = WiFi.softAPIP().toString();
                String portalMessage = "WiFi Setup Mode";
                portalMessage += "\nSSID: ";
                portalMessage += AP_SSID;
                portalMessage += "\nIP: " + apIP;
                portalMessage += "\nConnecting in 15s...";
                displayService.showConnectionStatus(false, portalMessage.c_str());

                // 表示時間と状態フラグを設定
                appStateManager.setWifiSettingDisplayTime(millis());
                appStateManager.setWifiSettingsDisplayed(true);
                appStateManager.setWaitingForPortalConnection(true);

                // カウントダウン開始ログ
                Serial.println("【カウントダウン開始】15秒後に自動接続を開始します");
                Serial.println("理由: WiFi設定があり、キャプティブポータルを一時的に表示");

                currentState = Application::WiFiConnectionState::PORTAL_ACTIVE;
            }

            return result;
        }

        // WiFiへの接続
        bool connectToWiFi() override
        {
            Domain::WiFiSettings wifiSettings;
            bool hasSettings = wifiService.loadSettings(wifiSettings);

            if (!hasSettings || !wifiSettings.isValid())
            {
                Serial.println("WiFi settings are not valid. Cannot connect.");
                return false;
            }

            Serial.print("接続先SSID: ");
            Serial.println(wifiSettings.getSsid());

            // 接続中表示に切り替え
            String ssid = wifiSettings.getSsid();
            String connectingMessage = "Connecting to WiFi";
            connectingMessage += "\nSSID: " + ssid;
            connectingMessage += "\nPlease wait...";
            displayService.showConnectionStatus(false, connectingMessage.c_str());

            // 接続開始
            bool result = wifiService.connect(wifiSettings);

            if (result)
            {
                currentState = Application::WiFiConnectionState::CONNECTING;
                appStateManager.setWifiConnectionStartTime(millis());
            }

            return result;
        }

        // WiFi接続状態の処理を実行
        void processWiFiConnection(unsigned long currentMillis) override
        {
            // WiFiサービスの処理を実行（キャプティブポータル含む）
            wifiService.process();

            // 接続状態の確認
            bool isConnected = wifiService.isConnected() || WiFi.status() == WL_CONNECTED;
            bool hasPortalConnection = wifiService.hasPortalConnections();
            bool isPortalActive = wifiService.isCaptivePortalActive();

            // 状態に応じた処理
            switch (currentState)
            {
            case Application::WiFiConnectionState::DISCONNECTED:
                if (isConnected)
                {
                    showConnectedStatus();
                    currentState = Application::WiFiConnectionState::CONNECTED;
                    connectionCompleted = true;
                }
                break;

            case Application::WiFiConnectionState::CONNECTING:
                if (isConnected)
                {
                    showConnectedStatus();
                    currentState = Application::WiFiConnectionState::CONNECTED;
                    connectionCompleted = true;
                }
                break;

            case Application::WiFiConnectionState::PORTAL_ACTIVE:
                // ポータル接続の処理
                processPortalState(currentMillis, isConnected, hasPortalConnection, isPortalActive);
                break;

            case Application::WiFiConnectionState::CONNECTED:
                // 接続状態の維持
                if (!isConnected)
                {
                    currentState = Application::WiFiConnectionState::DISCONNECTED;
                    connectionCompleted = false;
                }
                break;

            default:
                break;
            }
        }

        // 現在のWiFi接続状態を取得
        Application::WiFiConnectionState getConnectionState() override
        {
            return currentState;
        }

        // WiFi接続状態のリセット
        void resetConnectionState() override
        {
            appStateManager.resetWifiPortalState();
            currentState = Application::WiFiConnectionState::DISCONNECTED;
            connectionCompleted = false;
        }

        // 接続が完了したかどうか
        bool isConnectionCompleted() override
        {
            return connectionCompleted;
        }

    private:
        // キャプティブポータル状態の処理
        void processPortalState(unsigned long currentMillis, bool isConnected, bool hasPortalConnection, bool isPortalActive)
        {
            // ポータルへの接続がある場合
            if (hasPortalConnection)
            {
                // 設定中画面を表示
                String apIP = WiFi.softAPIP().toString();
                String portalMessage = "WiFi Setup in Progress";
                portalMessage += "\nSSID: ESP32-Splatoon3-Schedule";
                portalMessage += "\nIP: " + apIP;
                portalMessage += "\nConfiguration in progress...";
                displayService.showConnectionStatus(false, portalMessage.c_str());

                // カウントダウンを一時停止（十分先の未来に設定）
                appStateManager.setWifiSettingDisplayTime(currentMillis + (appStateManager.getWifiSettingDisplayDuration() * 8));
                currentState = Application::WiFiConnectionState::PORTAL_WITH_CONNECTION;

                return;
            }

            // WiFiが接続されていて、ポータル接続がない場合
            if (isConnected && !hasPortalConnection && isPortalActive)
            {
                // ポータルを停止して通常接続に移行
                Serial.println("WiFi connected. Stopping portal and proceeding to connect.");
                wifiService.stopCaptivePortal();
                appStateManager.resetWifiPortalState();

                showConnectedStatus();
                currentState = Application::WiFiConnectionState::CONNECTED;
                connectionCompleted = true;

                return;
            }

            // カウントダウン中で、タイムアウトした場合
            if (appStateManager.isWifiSettingsDisplayed() &&
                appStateManager.isWaitingForPortalConnection() &&
                currentMillis - appStateManager.getWifiSettingDisplayTime() >= appStateManager.getWifiSettingDisplayDuration())
            {

                // ポータル接続がなければWiFi接続を開始
                if (!hasPortalConnection)
                {
                    Serial.println("カウントダウンが終了し、WiFi接続を開始します");
                    wifiService.stopCaptivePortal();
                    connectToWiFi();
                }
            }
            else if (appStateManager.isWifiSettingsDisplayed() && appStateManager.isWaitingForPortalConnection())
            {
                // カウントダウン中の表示更新
                updateCountdownDisplay(currentMillis);
            }
        }

        // カウントダウン表示の更新
        void updateCountdownDisplay(unsigned long currentMillis)
        {
            static int lastDisplayedSeconds = -1;

            // 残り秒数を計算
            int remainingSeconds = (appStateManager.getWifiSettingDisplayDuration() -
                                    (currentMillis - appStateManager.getWifiSettingDisplayTime())) /
                                   1000;
            if (remainingSeconds < 0)
                remainingSeconds = 0;

            // 表示が必要な場合のみ更新
            if (remainingSeconds != lastDisplayedSeconds)
            {
                String apIP = WiFi.softAPIP().toString();
                String portalMessage = "WiFi Setup Mode";
                portalMessage += "\nSSID: ESP32-Splatoon3-Schedule";
                portalMessage += "\nIP: " + apIP;
                portalMessage += "\nConnecting in " + String(remainingSeconds) + "s...";
                displayService.showConnectionStatus(false, portalMessage.c_str());

                lastDisplayedSeconds = remainingSeconds;
            }
        }

        // 接続成功状態の表示
        void showConnectedStatus()
        {
            String ipAddress = WiFi.localIP().toString();
            String ssid = WiFi.SSID();
            String connectingMessage = "Connection OK";
            connectingMessage += "\nSSID: " + ssid;
            connectingMessage += "\nIP: " + ipAddress;
            connectingMessage += "\nPlease wait...";
            displayService.showConnectionStatus(true, connectingMessage.c_str());

            // 接続の安定化のための遅延
            delay(500);
        }
    };
}

#endif // ESP32_WIFI_CONNECTION_MANAGER_H