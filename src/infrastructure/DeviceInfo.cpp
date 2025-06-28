// DeviceInfo.cpp
// ESP32デバイス情報の取得と表示の実装

#include "DeviceInfo.h"
#include <esp_system.h>
#include <esp_chip_info.h>
#include <esp_flash.h>

namespace Infrastructure
{
    void DeviceInfo::printDeviceInfo()
    {
        Serial.println("\n=== ESP32 Device Information ===");

        // デバイス種類
        Serial.print("Device Type: ");
        Serial.println(getDeviceType());

        // チップ情報
        Serial.print("Chip Info: ");
        Serial.println(getChipInfo());

        // メモリ情報
        Serial.print("Memory Info: ");
        Serial.println(getMemoryInfo());

        // フラッシュ情報
        Serial.print("Flash Info: ");
        Serial.println(getFlashInfo());

        // ディスプレイ情報
        Serial.print("Display Info: ");
        Serial.println(getDisplayInfo());

        // ディスプレイ色深度情報
        Serial.print("Color Depth: ");
        Serial.println(getDisplayColorDepth());

        // ディスプレイ向き情報
        Serial.print("Display Orientation: ");
        Serial.println(getDisplayOrientation());

        // デバイス固有のデフォルト設定
        int defaultBrightness, defaultUpdateInterval;
        getDeviceDefaults(defaultBrightness, defaultUpdateInterval);
        Serial.print("Default Brightness: ");
        Serial.print(defaultBrightness);
        Serial.print(", Default Update Interval: ");
        Serial.print(defaultUpdateInterval);
        Serial.println("ms");

        Serial.println("================================\n");
    }

    String DeviceInfo::getDeviceType()
    {
        return determineDeviceType();
    }

    String DeviceInfo::getChipInfo()
    {
        return determineChipModel();
    }

    String DeviceInfo::getMemoryInfo()
    {
        size_t freeHeap = ESP.getFreeHeap();
        size_t minFreeHeap = ESP.getMinFreeHeap();
        size_t maxAllocHeap = ESP.getMaxAllocHeap();

        String info = "Free: ";
        info += String(freeHeap);
        info += " bytes";

        return info;
    }

    String DeviceInfo::getFlashInfo()
    {
        uint32_t flashSize = ESP.getFlashChipSize();
        uint32_t flashSpeed = ESP.getFlashChipSpeed();

        String info = "Size: ";
        info += String(flashSize / 1024 / 1024);
        info += "MB, Speed: ";
        info += String(flashSpeed / 1000000);
        info += "MHz";

        return info;
    }

    String DeviceInfo::getDisplayInfo()
    {
        int width, height;
        String displayType;
        getDisplayDetails(width, height, displayType);

        String info = displayType;
        info += " ";
        info += String(width);
        info += "x";
        info += String(height);

        return info;
    }

    void DeviceInfo::getDisplayDetails(int &width, int &height, String &displayType)
    {
        // デバイス種類に基づいてディスプレイ情報を判定
        String deviceType = determineDeviceType();

        if (deviceType.indexOf("ESP32-2432S028R") >= 0 || deviceType.indexOf("ESP32-2432S028") >= 0)
        {
            // ESP32-2432S028R/S028 (2.4インチTFT)
            width = 320;
            height = 240;
            displayType = "ILI9341 TFT";
        }
        else if (deviceType.indexOf("ESP32-S3") >= 0)
        {
            // ESP32-S3 (一般的な開発ボード)
            width = 320;
            height = 240;
            displayType = "TFT (Generic)";
        }
        else if (deviceType.indexOf("ESP32-C3") >= 0)
        {
            // ESP32-C3 (一般的な開発ボード)
            width = 320;
            height = 240;
            displayType = "TFT (Generic)";
        }
        else if (deviceType.indexOf("ESP32-WROOM") >= 0)
        {
            // ESP32-WROOM (一般的な開発ボード)
            width = 320;
            height = 240;
            displayType = "TFT (Generic)";
        }
        else
        {
            // その他のデバイス（デフォルト値）
            width = 320;
            height = 240;
            displayType = "TFT (Unknown)";
        }
    }

    void DeviceInfo::getDeviceDefaults(int &defaultBrightness, int &defaultUpdateInterval)
    {
        String deviceType = determineDeviceType();

        if (deviceType.indexOf("ESP32-2432S028R") >= 0)
        {
            // ESP32-2432S028R (2.4インチTFT付き)
            defaultBrightness = 200;        // 明るめの設定
            defaultUpdateInterval = 300000; // 5分間隔
        }
        else if (deviceType.indexOf("ESP32-2432S028") >= 0)
        {
            // ESP32-2432S028 (2.4インチTFT付き、別バージョン)
            defaultBrightness = 180;        // やや明るめ
            defaultUpdateInterval = 300000; // 5分間隔
        }
        else if (deviceType.indexOf("ESP32-S3") >= 0)
        {
            // ESP32-S3シリーズ
            defaultBrightness = 150;        // 標準的な明るさ
            defaultUpdateInterval = 240000; // 4分間隔（S3は高速）
        }
        else if (deviceType.indexOf("ESP32-C3") >= 0)
        {
            // ESP32-C3シリーズ
            defaultBrightness = 120;        // やや暗め（省電力重視）
            defaultUpdateInterval = 360000; // 6分間隔（省電力）
        }
        else if (deviceType.indexOf("ESP32-WROOM") >= 0)
        {
            // ESP32-WROOMシリーズ
            defaultBrightness = 100;        // 標準的な明るさ
            defaultUpdateInterval = 300000; // 5分間隔
        }
        else
        {
            // その他のESP32デバイス（デフォルト値）
            defaultBrightness = 128;        // 中間の明るさ
            defaultUpdateInterval = 300000; // 5分間隔
        }
    }

    String DeviceInfo::determineDeviceType()
    {
        // チップ情報を取得
        esp_chip_info_t chip_info;
        esp_chip_info(&chip_info);

        // フラッシュ情報を取得
        uint32_t flashSize = ESP.getFlashChipSize();

        // デバイス種類の判定
        if (chip_info.model == CHIP_ESP32)
        {
            if (flashSize >= 16 * 1024 * 1024) // 16MB以上
            {
                return "ESP32-2432S028R (2.4\" TFT + 16MB Flash)";
            }
            else if (flashSize >= 8 * 1024 * 1024) // 8MB以上
            {
                return "ESP32-2432S028 (2.4\" TFT + 8MB Flash)";
            }
            else if (flashSize >= 4 * 1024 * 1024) // 4MB以上
            {
                return "ESP32-WROOM-32 (4MB Flash)";
            }
            else
            {
                return "ESP32-WROOM-32 (Standard)";
            }
        }
        else if (chip_info.model == CHIP_ESP32S3)
        {
            if (flashSize >= 16 * 1024 * 1024)
            {
                return "ESP32-S3-DevKitC-1 (16MB Flash)";
            }
            else
            {
                return "ESP32-S3-DevKitC-1 (8MB Flash)";
            }
        }
        else if (chip_info.model == CHIP_ESP32C3)
        {
            return "ESP32-C3-DevKitM-1";
        }
        else if (chip_info.model == CHIP_ESP32S2)
        {
            return "ESP32-S2-DevKitC-1";
        }
        else
        {
            return "Unknown ESP32 Device";
        }
    }

    String DeviceInfo::determineChipModel()
    {
        esp_chip_info_t chip_info;
        esp_chip_info(&chip_info);

        String chipModel = "";

        switch (chip_info.model)
        {
        case CHIP_ESP32:
            chipModel = "ESP32";
            break;
        case CHIP_ESP32S2:
            chipModel = "ESP32-S2";
            break;
        case CHIP_ESP32S3:
            chipModel = "ESP32-S3";
            break;
        case CHIP_ESP32C3:
            chipModel = "ESP32-C3";
            break;
        case CHIP_ESP32H2:
            chipModel = "ESP32-H2";
            break;
        default:
            chipModel = "Unknown";
            break;
        }

        chipModel += " (";
        chipModel += String(chip_info.cores);
        chipModel += " cores, ";
        chipModel += String(chip_info.revision);
        chipModel += " revision)";

        return chipModel;
    }

    String DeviceInfo::getDisplayColorDepth()
    {
        // デバイス種類に基づいて色深度を判定
        String deviceType = determineDeviceType();

        if (deviceType.indexOf("ESP32-2432S028R") >= 0 || deviceType.indexOf("ESP32-2432S028") >= 0)
        {
            // ESP32-2432S028R/S028 (ILI9341)
            return "16-bit (65K colors)";
        }
        else if (deviceType.indexOf("ESP32-S3") >= 0)
        {
            // ESP32-S3 (一般的なTFT)
            return "16-bit (65K colors)";
        }
        else if (deviceType.indexOf("ESP32-C3") >= 0)
        {
            // ESP32-C3 (一般的なTFT)
            return "16-bit (65K colors)";
        }
        else if (deviceType.indexOf("ESP32-WROOM") >= 0)
        {
            // ESP32-WROOM (一般的なTFT)
            return "16-bit (65K colors)";
        }
        else
        {
            // その他のデバイス（デフォルト値）
            return "16-bit (65K colors)";
        }
    }

    String DeviceInfo::getDisplayOrientation()
    {
        // デバイス種類に基づいて向きを判定
        String deviceType = determineDeviceType();

        if (deviceType.indexOf("ESP32-2432S028R") >= 0 || deviceType.indexOf("ESP32-2432S028") >= 0)
        {
            // ESP32-2432S028R/S028 (縦向き)
            return "Portrait (240x320)";
        }
        else if (deviceType.indexOf("ESP32-S3") >= 0)
        {
            // ESP32-S3 (横向き)
            return "Landscape (320x240)";
        }
        else if (deviceType.indexOf("ESP32-C3") >= 0)
        {
            // ESP32-C3 (横向き)
            return "Landscape (320x240)";
        }
        else if (deviceType.indexOf("ESP32-WROOM") >= 0)
        {
            // ESP32-WROOM (横向き)
            return "Landscape (320x240)";
        }
        else
        {
            // その他のデバイス（デフォルト値）
            return "Landscape (320x240)";
        }
    }
}

// namespace Infrastructure