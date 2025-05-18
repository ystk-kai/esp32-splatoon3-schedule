// WiFiConnectionManager.h
// WiFi接続管理のサービスインターフェース

#ifndef WIFI_CONNECTION_MANAGER_H
#define WIFI_CONNECTION_MANAGER_H

#include <Arduino.h>
#include "WiFiService.h"
#include "../domain/WiFiSettings.h"

namespace Application
{
    // WiFi接続の状態
    enum class WiFiConnectionState
    {
        DISCONNECTED,
        CONNECTING,
        CONNECTED,
        PORTAL_ACTIVE,
        PORTAL_WITH_CONNECTION
    };

    // WiFi接続管理のサービスインターフェース
    class WiFiConnectionManager
    {
    public:
        virtual ~WiFiConnectionManager() = default;

        // WiFi接続の初期セットアップ
        virtual void setupWiFiConnection() = 0;

        // WiFi接続状態の処理を実行
        virtual void processWiFiConnection(unsigned long currentMillis) = 0;

        // キャプティブポータルの起動
        virtual bool startCaptivePortal() = 0;

        // WiFiへの接続
        virtual bool connectToWiFi() = 0;

        // 現在のWiFi接続状態を取得
        virtual WiFiConnectionState getConnectionState() = 0;

        // WiFi接続状態のリセット
        virtual void resetConnectionState() = 0;

        // 接続が完了したかどうか
        virtual bool isConnectionCompleted() = 0;
    };
}

#endif // WIFI_CONNECTION_MANAGER_H