// WiFiSettings.h
// WiFi設定のためのドメインモデル

#ifndef WIFI_SETTINGS_H
#define WIFI_SETTINGS_H

#include <Arduino.h>

namespace Domain
{
    // WiFi設定を保持するドメインモデル
    class WiFiSettings
    {
    private:
        String ssid;
        String password;
        bool isDhcp; // DHCP使用の場合true、静的IPの場合false
        String ipAddress;
        String gateway;
        String subnet;
        String dns1;
        String dns2;

    public:
        // コンストラクタ
        WiFiSettings() : isDhcp(true) {}

        // 静的メソッド：デフォルト設定の作成
        static WiFiSettings createDefault()
        {
            WiFiSettings settings;
            settings.isDhcp = true;
            return settings;
        }

        // SSID設定・取得
        void setSsid(const String &value) { ssid = value; }
        String getSsid() const { return ssid; }

        // パスワード設定・取得
        void setPassword(const String &value) { password = value; }
        String getPassword() const { return password; }

        // DHCP設定・取得
        void setDhcp(bool value) { isDhcp = value; }
        bool getDhcp() const { return isDhcp; }

        // IP設定・取得
        void setIpAddress(const String &value) { ipAddress = value; }
        String getIpAddress() const { return ipAddress; }

        // ゲートウェイ設定・取得
        void setGateway(const String &value) { gateway = value; }
        String getGateway() const { return gateway; }

        // サブネット設定・取得
        void setSubnet(const String &value) { subnet = value; }
        String getSubnet() const { return subnet; }

        // DNS1設定・取得
        void setDns1(const String &value) { dns1 = value; }
        String getDns1() const { return dns1; }

        // DNS2設定・取得
        void setDns2(const String &value) { dns2 = value; }
        String getDns2() const { return dns2; }

        // 設定が有効かどうかを確認（少なくともSSIDが設定されているか）
        bool isValid() const
        {
            return ssid.length() > 0;
        }
    };
}

#endif // WIFI_SETTINGS_H