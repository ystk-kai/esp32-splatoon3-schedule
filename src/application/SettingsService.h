// SettingsService.h
// 設定管理のためのサービスインターフェース

#ifndef SETTINGS_SERVICE_H
#define SETTINGS_SERVICE_H

#include <Arduino.h>
#include "../domain/DisplaySettings.h"

namespace Application
{
    // 設定管理のためのサービスインターフェース
    class SettingsService
    {
    public:
        virtual ~SettingsService() = default;

        // 表示設定を保存する
        virtual bool saveDisplaySettings(const Domain::DisplaySettings &settings) = 0;

        // 表示設定を読み込む
        virtual Domain::DisplaySettings loadDisplaySettings() = 0;

        // 画面反転設定を保存する
        virtual bool saveInvertedDisplaySetting(bool inverted) = 0;

        // 画面反転設定を読み込む
        virtual bool loadInvertedDisplaySetting() = 0;

        // デバイス固有の設定を初期化
        virtual void initializeDeviceSpecificSettings() = 0;
    };
} // namespace Application

#endif // SETTINGS_SERVICE_H