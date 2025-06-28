// DeviceInfo.h
// ESP32デバイス情報の取得と表示

#ifndef DEVICE_INFO_H
#define DEVICE_INFO_H

#include <Arduino.h>

namespace Infrastructure
{
    class DeviceInfo
    {
    public:
        // デバイス情報を取得して表示
        static void printDeviceInfo();

        // デバイス種類を取得
        static String getDeviceType();

        // チップ情報を取得
        static String getChipInfo();

        // メモリ情報を取得
        static String getMemoryInfo();

        // フラッシュ情報を取得
        static String getFlashInfo();

        // ディスプレイ情報を取得
        static String getDisplayInfo();

        // ディスプレイの詳細情報を取得
        static void getDisplayDetails(int &width, int &height, String &displayType);

        // ディスプレイの色深度情報を取得
        static String getDisplayColorDepth();

        // ディスプレイの向き情報を取得
        static String getDisplayOrientation();

        // デバイス固有のデフォルト設定を取得
        static void getDeviceDefaults(int &defaultBrightness, int &defaultUpdateInterval);

    private:
        // デバイス種類の判定
        static String determineDeviceType();

        // チップモデルの判定
        static String determineChipModel();
    };
}

#endif // DEVICE_INFO_H