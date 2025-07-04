// PreferencesSettingsService.h
// Preferences APIを使用した設定管理サービスの実装

#ifndef PREFERENCES_SETTINGS_SERVICE_H
#define PREFERENCES_SETTINGS_SERVICE_H

#include <Arduino.h>
#include <Preferences.h>
#include "../application/SettingsService.h"
#include "../domain/DisplaySettings.h"
#include "DeviceInfo.h"

namespace Infrastructure
{
    // Preferences APIを使用した設定管理サービスの実装
    class PreferencesSettingsService : public Application::SettingsService
    {
    private:
        Preferences &preferences;
        static constexpr const char *PREF_NAMESPACE = "splatoon3";

    public:
        // コンストラクタ - Preferencesのインスタンスを受け取る
        explicit PreferencesSettingsService(Preferences &preferences)
            : preferences(preferences) {}

        // 表示設定を保存する
        bool saveDisplaySettings(const Domain::DisplaySettings &settings) override
        {
            preferences.begin(PREF_NAMESPACE, false);

            // 表示設定を保存
            preferences.putBool("romaji_battle", settings.isUseRomajiForBattleType());
            preferences.putBool("romaji_rule", settings.isUseRomajiForRule());
            preferences.putBool("romaji_stage", settings.isUseRomajiForStage());

            preferences.end();
            return true;
        }

        // 表示設定を読み込む
        Domain::DisplaySettings loadDisplaySettings() override
        {
            preferences.begin(PREF_NAMESPACE, true); // 読み取り専用モード

            // 表示設定を読み込む
            bool useRomajiForBattleType = preferences.getBool("romaji_battle", true); // デフォルトはローマ字表示
            bool useRomajiForRule = preferences.getBool("romaji_rule", true);         // デフォルトはローマ字表示
            bool useRomajiForStage = preferences.getBool("romaji_stage", true);       // デフォルトはローマ字表示

            preferences.end();

            // 表示設定を作成して返す
            return Domain::DisplaySettings::create(
                useRomajiForBattleType,
                useRomajiForRule,
                useRomajiForStage);
        }

        // 画面反転設定を保存する
        bool saveInvertedDisplaySetting(bool inverted) override
        {
            Serial.print("画面反転設定を保存: ");
            Serial.println(inverted ? "有効" : "無効");

            preferences.begin(PREF_NAMESPACE, false); // 書き込みモード
            bool result = preferences.putBool("inv_disp", inverted);
            preferences.end();

            return result;
        }

        // 画面反転設定を読み込む
        bool loadInvertedDisplaySetting() override
        {
            preferences.begin(PREF_NAMESPACE, true); // 読み取り専用モード

            // 反転設定を読み込む
            bool inverted = preferences.getBool("inv_disp", false);

            Serial.print("画面反転設定を読み込み: ");
            Serial.println(inverted ? "有効" : "無効");

            preferences.end();
            return inverted;
        }

        // デバイス固有のデフォルト設定を取得
        void getDeviceSpecificDefaults(int &brightness, int &updateInterval)
        {
            Infrastructure::DeviceInfo::getDeviceDefaults(brightness, updateInterval);
        }

        // デバイス固有の設定を初期化
        void initializeDeviceSpecificSettings()
        {
            int defaultBrightness, defaultUpdateInterval;
            getDeviceSpecificDefaults(defaultBrightness, defaultUpdateInterval);

            preferences.begin(PREF_NAMESPACE, false);

            // デバイス固有のデフォルト設定を保存（初回のみ）
            // getBoolの第2引数（デフォルト値）を使用して存在チェック
            if (preferences.getBool("brightness_set", false) == false)
            {
                preferences.putInt("brightness", defaultBrightness);
                preferences.putInt("update_interval", defaultUpdateInterval);
                preferences.putBool("brightness_set", true);

                Serial.print("Device-specific defaults initialized: ");
                Serial.print("Brightness=");
                Serial.print(defaultBrightness);
                Serial.print(", UpdateInterval=");
                Serial.print(defaultUpdateInterval);
                Serial.println("ms");
            }

            preferences.end();
        }
    };
}

#endif // PREFERENCES_SETTINGS_SERVICE_H