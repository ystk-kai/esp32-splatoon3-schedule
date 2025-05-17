// ESP32WiFiService.h
// ESP32用WiFiサービスの実装

#ifndef ESP32_WIFI_SERVICE_H
#define ESP32_WIFI_SERVICE_H

#include <Arduino.h>
#include <WiFi.h>
#include <DNSServer.h>
#include <WebServer.h>
#include <Preferences.h>
#include "../application/WiFiService.h"
#include "../domain/WiFiSettings.h"
#include "../domain/DisplaySettings.h"

namespace Infrastructure
{
    // WiFi接続の状態
    enum class WiFiState
    {
        CONNECTING,    // WiFiに接続しようとしている
        CONNECTED,     // WiFiに接続済み
        PORTAL_ACTIVE, // キャプティブポータルアクティブ
        DISCONNECTED   // 切断状態
    };

    // ESP32用WiFiサービスの実装
    class ESP32WiFiService : public Application::WiFiService
    {
    private:
        // DNS・Webサーバー
        DNSServer dnsServer;
        WebServer webServer;

        // 設定保存
        Preferences preferences;

        // 状態管理
        WiFiState state;
        unsigned long lastConnectionAttempt;
        unsigned long connectionTimeout;
        unsigned long portalTimeout;
        unsigned long portalStartTime;

        // キャプティブポータル設定
        String apName;
        String apPassword;
        bool captivePortalActive;
        bool portalConnectionDetected; // キャプティブポータルへの接続検出

        // 表示設定
        Domain::DisplaySettings displaySettings;

        // HTML/JS/CSSのための文字列
        const char *portal_html;

        // 内部処理メソッド
        void setupCaptivePortal();
        void handleRoot();
        void handleWiFiSave();
        void handleSettings();
        void handleNotFound();
        String getWiFiScanJson();
        void sendHeader();
        bool connectToWiFi(const Domain::WiFiSettings &settings);

    public:
        // コンストラクタ
        ESP32WiFiService();

        // WiFi設定をロードする
        bool loadSettings(Domain::WiFiSettings &settings) override;

        // WiFi設定を保存する
        bool saveSettings(const Domain::WiFiSettings &settings) override;

        // 表示設定をロードする
        bool loadDisplaySettings(Domain::DisplaySettings &settings);

        // 表示設定を保存する
        bool saveDisplaySettings(const Domain::DisplaySettings &settings);

        // キャプティブポータルを開始する
        bool startCaptivePortal(const char *apName, const char *apPassword) override;

        // キャプティブポータルを停止する
        bool stopCaptivePortal() override;

        // WiFiに接続する
        bool connect(const Domain::WiFiSettings &settings) override;

        // WiFiの接続状態を確認する
        bool isConnected() override;

        // キャプティブポータルが実行中かどうかを確認する
        bool isCaptivePortalActive() override;

        // キャプティブポータルに接続があったかどうかを確認する
        bool hasPortalConnections() override;

        // WiFi接続プロセスを処理する（メインループで呼び出す）
        void process() override;

        // タイムアウト設定
        void setConnectionTimeout(unsigned long milliseconds) { connectionTimeout = milliseconds; }
        void setPortalTimeout(unsigned long milliseconds) { portalTimeout = milliseconds; }
    };
}

#endif // ESP32_WIFI_SERVICE_H