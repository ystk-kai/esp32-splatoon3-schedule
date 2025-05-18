// AppStateManager.h
// アプリケーションの状態を管理するクラス

#ifndef APP_STATE_MANAGER_H
#define APP_STATE_MANAGER_H

#include <Arduino.h>

namespace Infrastructure
{
    // アプリケーション状態を管理するクラス
    class AppStateManager
    {
    private:
        // アプリケーション状態
        bool appInitialized;
        bool wifiSettingsDisplayed;      // WiFi設定情報が表示済みかどうか
        bool waitingForPortalConnection; // キャプティブポータルへのアクセス待ち
        bool isDataFetching;             // データ取得中かどうかのフラグ

        // タイマー関連の状態
        unsigned long lastDataUpdateTime;
        unsigned long lastTimeDisplayUpdateTime;
        unsigned long lastWifiProcessTime;
        unsigned long lastInitializeCheckTime;
        unsigned long wifiSettingDisplayTime;  // WiFi設定表示の開始時間
        unsigned long wifiConnectionStartTime; // WiFi接続開始時間

        // タイマー定数
        const unsigned long DATA_UPDATE_INTERVAL = 5 * 60 * 1000;     // 5分ごとにデータを更新（5分 x 60秒 x 1000ms）
        const unsigned long TIME_DISPLAY_UPDATE_INTERVAL = 10 * 1000; // 10秒ごとに時間表示を更新
        const unsigned long WIFI_PROCESS_INTERVAL = 100;              // 100msごとにWiFi処理を実行
        const unsigned long INITIALIZE_CHECK_INTERVAL = 1000;         // 1秒ごとに初期化チェック
        const unsigned long WIFI_SETTING_DISPLAY_DURATION = 15000;    // WiFi設定表示時間を15秒に設定

    public:
        // コンストラクタ
        AppStateManager()
            : appInitialized(false),
              wifiSettingsDisplayed(false),
              waitingForPortalConnection(false),
              isDataFetching(false),
              lastDataUpdateTime(0),
              lastTimeDisplayUpdateTime(0),
              lastWifiProcessTime(0),
              lastInitializeCheckTime(0),
              wifiSettingDisplayTime(0),
              wifiConnectionStartTime(0)
        {
        }

        // アプリケーション状態管理
        bool isAppInitialized() const { return appInitialized; }
        void setAppInitialized(bool initialized) { appInitialized = initialized; }

        bool isWifiSettingsDisplayed() const { return wifiSettingsDisplayed; }
        void setWifiSettingsDisplayed(bool displayed) { wifiSettingsDisplayed = displayed; }

        bool isWaitingForPortalConnection() const { return waitingForPortalConnection; }
        void setWaitingForPortalConnection(bool waiting) { waitingForPortalConnection = waiting; }

        bool getIsDataFetching() const { return isDataFetching; }
        void setIsDataFetching(bool fetching) { isDataFetching = fetching; }

        // タイマー関連のゲッター/セッター
        unsigned long getLastDataUpdateTime() const { return lastDataUpdateTime; }
        void setLastDataUpdateTime(unsigned long time) { lastDataUpdateTime = time; }

        unsigned long getLastTimeDisplayUpdateTime() const { return lastTimeDisplayUpdateTime; }
        void setLastTimeDisplayUpdateTime(unsigned long time) { lastTimeDisplayUpdateTime = time; }

        unsigned long getLastWifiProcessTime() const { return lastWifiProcessTime; }
        void setLastWifiProcessTime(unsigned long time) { lastWifiProcessTime = time; }

        unsigned long getLastInitializeCheckTime() const { return lastInitializeCheckTime; }
        void setLastInitializeCheckTime(unsigned long time) { lastInitializeCheckTime = time; }

        unsigned long getWifiSettingDisplayTime() const { return wifiSettingDisplayTime; }
        void setWifiSettingDisplayTime(unsigned long time) { wifiSettingDisplayTime = time; }

        unsigned long getWifiConnectionStartTime() const { return wifiConnectionStartTime; }
        void setWifiConnectionStartTime(unsigned long time) { wifiConnectionStartTime = time; }

        // タイマー定数
        unsigned long getDataUpdateInterval() const { return DATA_UPDATE_INTERVAL; }
        unsigned long getTimeDisplayUpdateInterval() const { return TIME_DISPLAY_UPDATE_INTERVAL; }
        unsigned long getWifiProcessInterval() const { return WIFI_PROCESS_INTERVAL; }
        unsigned long getInitializeCheckInterval() const { return INITIALIZE_CHECK_INTERVAL; }
        unsigned long getWifiSettingDisplayDuration() const { return WIFI_SETTING_DISPLAY_DURATION; }

        // 状態をリセットする
        void resetWifiPortalState()
        {
            waitingForPortalConnection = false;
            wifiSettingsDisplayed = false;
            wifiSettingDisplayTime = 0;
            wifiConnectionStartTime = 0;
        }
    };
}

#endif // APP_STATE_MANAGER_H