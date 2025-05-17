// WiFiService.h
// WiFi設定と管理を行うサービスインターフェース

#ifndef WIFI_SERVICE_H
#define WIFI_SERVICE_H

#include <Arduino.h>
#include "../domain/WiFiSettings.h"

namespace Application
{
    // WiFi管理のためのサービスインターフェース
    class WiFiService
    {
    public:
        virtual ~WiFiService() = default;

        // WiFi設定をロードする
        virtual bool loadSettings(Domain::WiFiSettings &settings) = 0;

        // WiFi設定を保存する
        virtual bool saveSettings(const Domain::WiFiSettings &settings) = 0;

        // キャプティブポータルを開始する
        virtual bool startCaptivePortal(const char *apName, const char *apPassword) = 0;

        // キャプティブポータルを停止する
        virtual bool stopCaptivePortal() = 0;

        // WiFiに接続する
        virtual bool connect(const Domain::WiFiSettings &settings) = 0;

        // WiFiの接続状態を確認する
        virtual bool isConnected() = 0;

        // キャプティブポータルが実行中かどうかを確認する
        virtual bool isCaptivePortalActive() = 0;

        // キャプティブポータルに接続があったかどうかを確認する
        virtual bool hasPortalConnections() = 0;

        // WiFi接続プロセスを処理する（メインループで呼び出す）
        virtual void process() = 0;
    };
}

#endif // WIFI_SERVICE_H