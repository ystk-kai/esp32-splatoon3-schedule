// ESP32WiFiService.cpp
// ESP32用WiFiサービスの実装

#include "ESP32WiFiService.h"
#include "WiFiPortalContent.h"
#include <WiFi.h>
#include <DNSServer.h>
#include <WebServer.h>
#include <Preferences.h>
#include <esp_wifi.h>

namespace Infrastructure
{
    // DNSポート
    const byte DNS_PORT = 53;

    // 設定保存用のPreferencesネームスペース
    const char *PREF_NAMESPACE = "wifiConfig";

    // IPアドレス用定数
    const IPAddress AP_IP(192, 168, 4, 1);
    const IPAddress AP_GATEWAY(192, 168, 4, 1);
    const IPAddress AP_SUBNET(255, 255, 255, 0);

    // コンストラクタ
    ESP32WiFiService::ESP32WiFiService()
        : webServer(80),
          state(WiFiState::DISCONNECTED),
          lastConnectionAttempt(0),
          connectionTimeout(20000), // 20秒のデフォルト接続タイムアウト
          portalTimeout(300000),    // 5分のデフォルトポータルタイムアウト
          portalStartTime(0),
          captivePortalActive(false),
          portalConnectionDetected(false),
          displaySettings(Domain::DisplaySettings::createDefault()), // DisplaySettingsをデフォルト値で初期化
          portal_html(PORTAL_HTML)
    {
    }

    // WiFi設定をロードする
    bool ESP32WiFiService::loadSettings(Domain::WiFiSettings &settings)
    {
        preferences.begin(PREF_NAMESPACE, true); // 読み取り専用モード

        // 設定がPreferencesに保存されているか確認
        bool hasSettings = preferences.getBool("hasSettings", false);

        if (hasSettings)
        {
            settings.setSsid(preferences.getString("ssid", ""));
            settings.setPassword(preferences.getString("password", ""));
            settings.setDhcp(preferences.getBool("dhcp", true));

            // DHCPが無効の場合は静的IP設定を読み込む
            if (!settings.getDhcp())
            {
                settings.setIpAddress(preferences.getString("ip", ""));
                settings.setGateway(preferences.getString("gateway", ""));
                settings.setSubnet(preferences.getString("subnet", ""));
                settings.setDns1(preferences.getString("dns1", ""));
                settings.setDns2(preferences.getString("dns2", ""));
            }
        }

        preferences.end();
        return hasSettings && settings.isValid();
    }

    // 表示設定をロードする
    bool ESP32WiFiService::loadDisplaySettings(Domain::DisplaySettings &settings)
    {
        preferences.begin("splatoon3", true); // 読み取り専用モード

        // 表示設定を読み込む
        bool useRomajiForBattleType = preferences.getBool("romaji_battle", true); // デフォルトはローマ字表示
        bool useRomajiForRule = preferences.getBool("romaji_rule", true);         // デフォルトはローマ字表示
        bool useRomajiForStage = preferences.getBool("romaji_stage", true);       // デフォルトはローマ字表示

        preferences.end();

        // 表示設定を作成して返す
        settings = Domain::DisplaySettings::create(
            useRomajiForBattleType,
            useRomajiForRule,
            useRomajiForStage);

        return true;
    }

    // 表示設定を保存する
    bool ESP32WiFiService::saveDisplaySettings(const Domain::DisplaySettings &settings)
    {
        preferences.begin("splatoon3", false); // 書き込みモード

        // 表示設定を保存
        preferences.putBool("romaji_battle", settings.isUseRomajiForBattleType());
        preferences.putBool("romaji_rule", settings.isUseRomajiForRule());
        preferences.putBool("romaji_stage", settings.isUseRomajiForStage());

        preferences.end();
        return true;
    }

    // 画面反転設定をロードする
    bool ESP32WiFiService::loadInvertedDisplaySetting(bool &inverted)
    {
        // 明示的に名前空間を指定して一貫性を確保
        preferences.begin("splatoon3", true); // 読み取り専用モード

        // 反転設定を読み込む
        inverted = preferences.getBool("inv_disp", false);

        Serial.print("反転表示設定を読み込み: ");
        Serial.println(inverted ? "有効" : "無効");

        preferences.end();
        return true;
    }

    // 画面反転設定を保存する
    bool ESP32WiFiService::saveInvertedDisplaySetting(bool inverted)
    {
        Serial.print("反転表示設定を保存: ");
        Serial.println(inverted ? "有効" : "無効");

        // 明示的に名前空間を指定して一貫性を確保
        preferences.begin("splatoon3", false); // 書き込みモード

        // 反転設定を保存
        bool result = preferences.putBool("inv_disp", inverted);

        preferences.end();
        return result;
    }

    // WiFi設定を保存する
    bool ESP32WiFiService::saveSettings(const Domain::WiFiSettings &settings)
    {
        if (!settings.isValid())
        {
            return false;
        }

        preferences.begin(PREF_NAMESPACE, false); // 書き込みモード

        preferences.putBool("hasSettings", true);
        preferences.putString("ssid", settings.getSsid());
        preferences.putString("password", settings.getPassword());
        preferences.putBool("dhcp", settings.getDhcp());

        // DHCPが無効の場合は静的IP設定を保存
        if (!settings.getDhcp())
        {
            preferences.putString("ip", settings.getIpAddress());
            preferences.putString("gateway", settings.getGateway());
            preferences.putString("subnet", settings.getSubnet());
            preferences.putString("dns1", settings.getDns1());
            preferences.putString("dns2", settings.getDns2());
        }

        preferences.end();
        return true;
    }

    // キャプティブポータルを開始する
    bool ESP32WiFiService::startCaptivePortal(const char *apName, const char *apPassword)
    {
        if (captivePortalActive)
        {
            Serial.println("キャプティブポータルはすでに起動中です");
            return true; // すでに起動中
        }

        Serial.println("キャプティブポータルを起動します");

        // 現在の接続を切断
        WiFi.disconnect(true);
        delay(500);

        // APモードに設定
        WiFi.mode(WIFI_AP);
        delay(500);

        // AP IP設定
        bool configSuccess = WiFi.softAPConfig(AP_IP, AP_GATEWAY, AP_SUBNET);
        if (!configSuccess)
        {
            Serial.println("APの設定に失敗しました");
            return false;
        }
        Serial.println("APの設定に成功しました");

        // APを開始
        this->apName = apName;
        this->apPassword = apPassword;

        bool apStarted;
        if (apPassword && strlen(apPassword) >= 8)
        {
            Serial.println("パスワード付きでAPを開始します");
            apStarted = WiFi.softAP(apName, apPassword);
        }
        else
        {
            // パスワードなしまたは短すぎる場合、オープンネットワークとして開始
            Serial.println("オープンネットワークとしてAPを開始します");
            apStarted = WiFi.softAP(apName);
        }

        if (!apStarted)
        {
            Serial.println("AP起動に失敗しました");
            return false;
        }

        Serial.print("APを起動しました: ");
        Serial.println(apName);
        Serial.print("IP: ");
        Serial.println(WiFi.softAPIP());

        // DNSサーバーの開始
        dnsServer.start(DNS_PORT, "*", AP_IP);
        Serial.println("DNSサーバーを開始しました");

        // Webサーバーのハンドラーを設定
        setupCaptivePortal();
        Serial.println("キャプティブポータルのハンドラーを設定しました");

        // Webサーバーを開始
        webServer.begin();
        Serial.println("Webサーバーを開始しました");

        // 状態とタイマーを更新
        state = WiFiState::PORTAL_ACTIVE;
        portalStartTime = millis();
        captivePortalActive = true;
        portalConnectionDetected = false; // 接続検出フラグをリセット

        return true;
    }

    // キャプティブポータルを停止する
    bool ESP32WiFiService::stopCaptivePortal()
    {
        if (!captivePortalActive)
        {
            return true; // すでに停止している
        }

        Serial.println("キャプティブポータルを停止します");

        // ポータル接続検出フラグも必ずリセット
        portalConnectionDetected = false;
        Serial.println("ポータル接続検出フラグもリセットします");

        // DNSサーバーを停止
        dnsServer.stop();

        // Webサーバーを停止
        webServer.stop();

        // APモードを停止
        WiFi.softAPdisconnect(true);

        // フラグ更新
        captivePortalActive = false;
        state = WiFiState::DISCONNECTED;

        return true;
    }

    // WiFiに接続する
    bool ESP32WiFiService::connect(const Domain::WiFiSettings &settings)
    {
        if (!settings.isValid())
        {
            Serial.println("無効なWiFi設定です");
            return false;
        }

        // ポータルが起動している場合は停止
        if (captivePortalActive)
        {
            Serial.println("キャプティブポータルを停止し、WiFi接続を開始します");
            stopCaptivePortal();
            delay(1000); // 停止処理の完了を待つ時間を長めに
        }

        // WiFiモードを明示的にSTAに設定
        WiFi.mode(WIFI_STA);
        delay(500); // WiFiモード変更の安定化を待つ

        // 接続する前にWiFiを切断して設定をクリア
        WiFi.disconnect(true);
        delay(1000); // 切断の完了を十分待つ

        // 自動再接続を有効に設定
        WiFi.setAutoReconnect(true);
        WiFi.persistent(true);
        delay(200);

        // 静的IP設定（必要な場合）
        if (!settings.getDhcp())
        {
            Serial.println("静的IP設定を使用します");
            IPAddress ip, gateway, subnet, dns1, dns2;

            if (ip.fromString(settings.getIpAddress()) &&
                gateway.fromString(settings.getGateway()) &&
                subnet.fromString(settings.getSubnet()))
            {
                Serial.println("IP: " + settings.getIpAddress());
                Serial.println("Gateway: " + settings.getGateway());
                Serial.println("Subnet: " + settings.getSubnet());

                // DNS設定（オプション）
                if (settings.getDns1().length() > 0)
                {
                    Serial.println("DNS1: " + settings.getDns1());
                    dns1.fromString(settings.getDns1());

                    if (settings.getDns2().length() > 0)
                    {
                        Serial.println("DNS2: " + settings.getDns2());
                        dns2.fromString(settings.getDns2());
                        WiFi.config(ip, gateway, subnet, dns1, dns2);
                    }
                    else
                    {
                        WiFi.config(ip, gateway, subnet, dns1);
                    }
                }
                else
                {
                    WiFi.config(ip, gateway, subnet);
                }
                Serial.println("静的IP設定を適用しました");
            }
            else
            {
                Serial.println("静的IP設定の変換に失敗しました");
            }
        }
        else
        {
            Serial.println("DHCPを使用します");
        }

        // WiFi接続を開始
        Serial.print("WiFiに接続します: ");
        Serial.println(settings.getSsid());

        // WiFi設定
        WiFi.setAutoReconnect(true);
        WiFi.persistent(true);

        // WiFi接続開始
        WiFi.begin(settings.getSsid().c_str(), settings.getPassword().c_str());

        lastConnectionAttempt = millis();
        state = WiFiState::CONNECTING;

        // 接続試行の開始後、十分に待機して接続を確認
        Serial.println("接続の確立を待機中...");

        // 接続の成功を最大5秒間待機（一般的なWiFi接続は数秒で完了する）
        int attempts = 0;
        while (attempts < 10 && WiFi.status() != WL_CONNECTED)
        {
            delay(500);
            Serial.print(".");
            attempts++;
        }
        Serial.println();

        // 接続状態の最終確認
        if (WiFi.status() == WL_CONNECTED)
        {
            Serial.println("WiFi接続に成功しました");
            Serial.print("IP: ");
            Serial.println(WiFi.localIP());
            state = WiFiState::CONNECTED;
            return true;
        }

        // この時点では接続は進行中または失敗
        Serial.println("接続待機後もWiFi接続できていません。接続プロセスを継続します...");
        return false;
    }

    // WiFiの接続状態を確認する
    bool ESP32WiFiService::isConnected()
    {
        return WiFi.status() == WL_CONNECTED;
    }

    // キャプティブポータルが実行中かどうかを確認する
    bool ESP32WiFiService::isCaptivePortalActive()
    {
        return captivePortalActive;
    }

    // WiFi接続プロセスを処理する（メインループで呼び出す）
    void ESP32WiFiService::process()
    {
        // 設定保存後の再起動処理を最初にチェック - 優先順位を最高に
        if (settingsSaved && millis() - settingsSaveTime >= 5000)
        {
            // 5秒経過したら再起動
            Serial.println("設定保存から5秒経過しました。再起動します...");

            // キャプティブポータルを停止し、WiFiも切断
            stopCaptivePortal();
            WiFi.disconnect(true);

            delay(1000);   // 安定化のために少し長く待機
            ESP.restart(); // ESP32を再起動
            delay(5000);   // 再起動完了を待機（念のため）
            return;        // 念のため
        }

        // 現在の状態に基づいて処理
        switch (state)
        {
        case WiFiState::CONNECTING:
            // 接続試行中
            if (isConnected())
            {
                // 接続成功
                Serial.println("WiFiに接続しました");
                Serial.print("IP: ");
                Serial.println(WiFi.localIP());
                state = WiFiState::CONNECTED;
                lastConnectionAttempt = 0; // タイムアウトカウンターをリセット
            }
            else if (millis() - lastConnectionAttempt > connectionTimeout)
            {
                // 接続タイムアウト、キャプティブポータルを開始
                Serial.println("WiFi接続タイムアウト");

                // すでに保存されている設定を一度試したが失敗した場合は、キャプティブポータルを開始
                Serial.println("キャプティブポータルを開始します");
                startCaptivePortal("ESP32-Splatoon3-Schedule", nullptr);
            }
            break;

        case WiFiState::CONNECTED:
            // 接続済み、切断されていないか確認
            if (!isConnected())
            {
                // 接続が切れた
                Serial.println("WiFi接続が切れました");
                state = WiFiState::DISCONNECTED;
                lastConnectionAttempt = millis();
            }
            break;

        case WiFiState::PORTAL_ACTIVE:
            // キャプティブポータルアクティブ
            if (captivePortalActive)
            {
                // DNSリクエストを処理
                dnsServer.processNextRequest();

                // HTTPリクエストを処理
                webServer.handleClient();

                // ポータルタイムアウトを確認
                if (millis() - portalStartTime > portalTimeout)
                {
                    Serial.println("ポータルタイムアウト、再起動します");
                    stopCaptivePortal();
                    ESP.restart(); // タイムアウト時にESP32を再起動
                }
            }
            break;

        case WiFiState::DISCONNECTED:
            // 切断状態、自動的に再接続を試みる
            Domain::WiFiSettings settings;
            if (loadSettings(settings) && settings.isValid())
            {
                Serial.println("保存済みのWiFi設定を使用して接続を試みます");
                connect(settings);
            }
            else
            {
                // 有効な設定がない場合はキャプティブポータルを開始
                Serial.println("有効なWiFi設定がありません。キャプティブポータルを開始します");
                startCaptivePortal("ESP32-Splatoon3-Schedule", nullptr);
            }
            break;
        }
    }

    // WiFi接続試行のヘルパーメソッド
    bool ESP32WiFiService::connectToWiFi(const Domain::WiFiSettings &settings)
    {
        if (WiFi.status() == WL_CONNECTED)
        {
            Serial.println("WiFiはすでに接続されています");
            state = WiFiState::CONNECTED;
            return true;
        }

        // 有効な設定がない場合はfalseを返す
        if (!settings.isValid())
        {
            Serial.println("WiFi設定が無効です");
            return false;
        }

        // この時点では接続は進行中
        Serial.println("WiFi接続を試行中...");
        state = WiFiState::CONNECTING; // 状態を明示的に設定
        return false;
    }

    // キャプティブポータルのセットアップ
    void ESP32WiFiService::setupCaptivePortal()
    {
        Serial.println("キャプティブポータルハンドラを設定します");

        // ルートページハンドラ
        webServer.on("/", HTTP_GET, [this]()
                     { this->handleRoot(); });

        // WiFiスキャンAPI - ラムダ関数を使って明示的に呼び出し
        webServer.on("/scan", HTTP_GET, [this]()
                     {
            Serial.println("スキャンAPIが呼び出されました");
            this->getWiFiScanJson(); });

        // 設定取得API
        webServer.on("/settings", HTTP_GET, [this]()
                     { this->handleSettings(); });

        // 設定保存API
        webServer.on("/save", HTTP_POST, [this]()
                     { this->handleWiFiSave(); });

        // 404ハンドラ（すべてをルートページにリダイレクト）
        webServer.onNotFound([this]()
                             { this->handleNotFound(); });

        Serial.println("すべてのハンドラが設定されました");
    }

    // ルートページを処理する
    void ESP32WiFiService::handleRoot()
    {
        // ページへのアクセスを検出
        portalConnectionDetected = true;
        Serial.println("キャプティブポータルにアクセスがありました");

        webServer.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
        webServer.sendHeader("Pragma", "no-cache");
        webServer.sendHeader("Expires", "-1");
        webServer.send(200, "text/html", portal_html);
    }

    // 404リクエストを処理する（すべてルートにリダイレクト）
    void ESP32WiFiService::handleNotFound()
    {
        // 404ページへのアクセスも検出
        portalConnectionDetected = true;
        Serial.println("未登録のパスへのリクエストをリダイレクト: " + webServer.uri());

        webServer.sendHeader("Location", "/", true);
        webServer.send(302, "text/plain", "");
    }

    // WiFiスキャン結果をJSONで返す
    void ESP32WiFiService::getWiFiScanJson()
    {
        // WiFiスキャンAPIへのアクセスを検出
        portalConnectionDetected = true;
        Serial.println("WiFiスキャンAPIにアクセスがありました");

        // スキャン開始
        Serial.println("WiFiネットワークのスキャンを開始します...");

        // 既存のスキャンが実行中でないことを確認
        int scanStatus = WiFi.scanComplete();
        if (scanStatus == WIFI_SCAN_RUNNING)
        {
            Serial.println("スキャン既に実行中です");
            // 進行中のスキャンが完了するまで短い時間待機
            delay(100);
            scanStatus = WiFi.scanComplete();
        }

        // 前回のスキャン結果をクリア
        if (scanStatus >= 0)
        {
            Serial.println("前回のスキャン結果をクリア");
            WiFi.scanDelete();
        }

        // スキャン開始（同期モードで実行し、信頼性を確保）
        Serial.println("WiFiスキャンを実行中...");
        int n = WiFi.scanNetworks(/*async=*/false, /*show_hidden=*/true, /*passive=*/false, /*max_ms_per_chan=*/300);

        Serial.print("スキャン完了。検出されたネットワーク数: ");
        Serial.println(n);

        // JSON作成
        String json = "{\"networks\":[";

        if (n > 0)
        {
            for (int i = 0; i < n; i++)
            {
                String ssid = WiFi.SSID(i);
                // エスケープが必要な文字をエスケープ
                ssid.replace("\"", "\\\"");
                ssid.replace("\\", "\\\\");

                json += "{";
                json += "\"ssid\":\"" + ssid + "\",";
                json += "\"rssi\":" + String(WiFi.RSSI(i)) + ",";
                json += "\"secure\":" + String(WiFi.encryptionType(i) != WIFI_AUTH_OPEN ? "true" : "false");
                json += "}";
                if (i < n - 1)
                {
                    json += ",";
                }
            }
        }
        json += "]}";

        Serial.println("スキャン結果JSON: " + json);
        webServer.send(200, "application/json", json);

        // スキャン結果をクリーンアップ
        WiFi.scanDelete();
    }

    // 設定情報をJSON形式で返す
    void ESP32WiFiService::handleSettings()
    {
        // 表示設定情報の取得
        Domain::DisplaySettings displaySettings = Domain::DisplaySettings::createDefault();
        loadDisplaySettings(displaySettings);

        // 画面反転設定の取得
        bool invertedDisplay = false;
        loadInvertedDisplaySetting(invertedDisplay);

        // WiFi設定情報の取得
        Domain::WiFiSettings wifiSettings;
        bool hasWifiSettings = loadSettings(wifiSettings);

        // WiFi設定のJSON部分を構築
        String wifiJson = "\"wifi\":{";
        if (hasWifiSettings && wifiSettings.isValid())
        {
            wifiJson += "\"ssid\":\"" + wifiSettings.getSsid() + "\",";
            // パスワードはセキュリティ上の理由で含めない
            wifiJson += "\"dhcp\":" + String(wifiSettings.getDhcp() ? "true" : "false");

            // 静的IP設定を含める（DHCPがfalseの場合のみ詳細を含める）
            if (!wifiSettings.getDhcp())
            {
                wifiJson += ",\"ip\":\"" + wifiSettings.getIpAddress() + "\"";
                wifiJson += ",\"gateway\":\"" + wifiSettings.getGateway() + "\"";
                wifiJson += ",\"subnet\":\"" + wifiSettings.getSubnet() + "\"";

                if (wifiSettings.getDns1().length() > 0)
                {
                    wifiJson += ",\"dns1\":\"" + wifiSettings.getDns1() + "\"";
                }

                if (wifiSettings.getDns2().length() > 0)
                {
                    wifiJson += ",\"dns2\":\"" + wifiSettings.getDns2() + "\"";
                }
            }
        }
        else
        {
            wifiJson += "\"dhcp\":true";
        }
        wifiJson += "}";

        // 表示設定のJSON部分を構築
        String displayJson = "\"display\":{";
        // ローマ字設定: true=ローマ字表示、false=英語表示
        displayJson += "\"battle_romaji\":" + String(displaySettings.isUseRomajiForBattleType() ? "true" : "false") + ",";
        displayJson += "\"rule_romaji\":" + String(displaySettings.isUseRomajiForRule() ? "true" : "false") + ",";
        displayJson += "\"stage_romaji\":" + String(displaySettings.isUseRomajiForStage() ? "true" : "false") + ",";
        // 画面反転設定: true=反転表示、false=通常表示
        displayJson += "\"inverted_display\":" + String(invertedDisplay ? "true" : "false");
        displayJson += "}";

        // 全体のJSONを構築
        String json = "{" + displayJson + "," + wifiJson + "}";

        Serial.println("設定情報を返します: " + json);

        // 画面反転設定値が正しくJSONに含まれているか確認
        if (json.indexOf("\"inverted_display\":true") >= 0)
        {
            Serial.println("JSON内の画面反転設定: 有効(true)");
        }
        else if (json.indexOf("\"inverted_display\":false") >= 0)
        {
            Serial.println("JSON内の画面反転設定: 無効(false)");
        }
        else
        {
            Serial.println("警告: JSON内に画面反転設定が見つかりません");
        }

        webServer.send(200, "application/json", json);
    }

    // WiFi設定の保存を処理する
    void ESP32WiFiService::handleWiFiSave()
    {
        // 設定保存APIへのアクセスを検出
        portalConnectionDetected = true;
        Serial.println("WiFi設定保存APIにアクセスがありました");

        // フォームデータを取得
        String ssid = webServer.arg("ssid");
        String password = webServer.arg("password");
        bool dhcp = webServer.arg("dhcp") == "1";

        // 設定を作成
        Domain::WiFiSettings settings;
        settings.setSsid(ssid);

        // パスワードが空の場合、既存のパスワードを保持する
        if (password.length() > 0)
        {
            settings.setPassword(password);
        }
        else
        {
            // 既存の設定からパスワードを取得
            Domain::WiFiSettings existingSettings;
            if (loadSettings(existingSettings) && existingSettings.isValid())
            {
                settings.setPassword(existingSettings.getPassword());
                Serial.println("パスワードが空のため、既存のパスワードを保持します");
            }
            else
            {
                // 既存の設定がない場合は空のパスワードを設定
                settings.setPassword("");
                Serial.println("既存の設定がないため、空のパスワードを設定します");
            }
        }

        settings.setDhcp(dhcp);

        // DHCPが無効の場合は静的IP設定を取得
        if (!dhcp)
        {
            settings.setIpAddress(webServer.arg("ip"));
            settings.setGateway(webServer.arg("gateway"));
            settings.setSubnet(webServer.arg("subnet"));
            settings.setDns1(webServer.arg("dns1"));
            settings.setDns2(webServer.arg("dns2"));
        }

        // 表示設定を取得して保存
        Serial.println("表示設定を保存します");
        Serial.print("バトルタイプ表示モード: ");
        Serial.println(webServer.arg("battle_romaji") == "1" ? "ローマ字" : "英語");
        Serial.print("ルール表示モード: ");
        Serial.println(webServer.arg("rule_romaji") == "1" ? "ローマ字" : "英語");
        Serial.print("ステージ表示モード: ");
        Serial.println(webServer.arg("stage_romaji") == "1" ? "ローマ字" : "英語");

        Domain::DisplaySettings displaySettings = Domain::DisplaySettings::create(
            webServer.arg("battle_romaji") == "1",
            webServer.arg("rule_romaji") == "1",
            webServer.arg("stage_romaji") == "1");
        bool displaySaved = saveDisplaySettings(displaySettings);
        Serial.print("表示設定の保存結果: ");
        Serial.println(displaySaved ? "成功" : "失敗");

        // 画面反転設定を保存（重要なので成功確認を追加）
        bool invertedDisplay = webServer.arg("inverted_display") == "1";
        bool invertSaved = saveInvertedDisplaySetting(invertedDisplay);
        Serial.print("画面反転設定: ");
        Serial.println(invertedDisplay ? "有効" : "無効");
        Serial.print("画面反転設定の保存結果: ");
        Serial.println(invertSaved ? "成功" : "失敗");

        // WiFi設定を保存
        bool saveResult = saveSettings(settings);

        // 表示設定の保存結果をログに出力
        Serial.print("表示設定の保存: ");
        Serial.print(settings.getSsid());
        Serial.print(", バトルタイプ: ");
        Serial.print(webServer.arg("battle_romaji") == "1" ? "ローマ字" : "英語");
        Serial.print(", ルール: ");
        Serial.print(webServer.arg("rule_romaji") == "1" ? "ローマ字" : "英語");
        Serial.print(", ステージ: ");
        Serial.println(webServer.arg("stage_romaji") == "1" ? "ローマ字" : "英語");

        if (saveResult)
        {
            // 保存成功を即座に通知
            webServer.send(200, "text/plain", "設定を保存しました");
            Serial.println("設定を保存しました。クライアントにレスポンスを送信しています...");

            // ポータル接続検出フラグをtrueに維持して「設定中」表示を継続
            portalConnectionDetected = true;

            // 保存完了後ユーザーに通知
            Serial.println("WiFi設定を保存しました。設定中画面を維持しています。");
            Serial.println("設定保存に成功したため、5秒後に再起動します...");

            // 設定保存フラグとタイマーをセット - process()メソッドで検出します
            settingsSaved = true;
            settingsSaveTime = millis();
            skipLogging = false; // ログ抑制フラグをリセット
        }
        else
        {
            Serial.println("設定の保存に失敗しました");
            webServer.send(400, "text/plain", "保存に失敗しました");
        }
    }

    // キャプティブポータルへの接続があったかどうかを確認する
    bool ESP32WiFiService::hasPortalConnections()
    {
        // キャプティブポータル自体が無効になっている場合は接続もないはず
        if (!captivePortalActive)
        {
            // 念のためフラグもリセット
            if (portalConnectionDetected)
            {
                Serial.println("ポータルが非アクティブなのでportalConnectionDetectedをリセットします");
                portalConnectionDetected = false;
            }
            return false;
        }

        // WiFi接続状態よりもポータル接続状態を優先する
        // portalConnectionDetectedがtrueの場合はそれを優先して接続ありと判定
        if (portalConnectionDetected)
        {
            // ポータルで設定保存後は設定処理中の状態を維持する
            // これにより再起動まで「設定中...」表示を維持できる
            if (settingsSaved)
            {
                // 設定保存後は常に接続中と判定
                if (!skipLogging)
                {
                    Serial.println("設定保存済みのため、portalConnectionDetected を維持します");
                    skipLogging = true; // ログが繰り返し出力されるのを防止
                }
                return true;
            }

            // アクティブなクライアント接続があるか確認
            bool hasActiveClient = webServer.client();

            if (hasActiveClient)
            {
                // ログスパムを防止
                static bool activeLogPrinted = false;
                if (!activeLogPrinted)
                {
                    Serial.println("WebServerにアクティブな接続があります - portalConnectionDetected を維持します");
                    activeLogPrinted = true;
                }
                return true;
            }
            else
            {
                // クライアント接続がない場合でも、フラグを維持
                // これによりブラウザを閉じても「設定中...」表示が継続
                return true;
            }
        }

        // 接続が検出されていない場合は、WebServerのクライアント接続を確認
        bool hasActiveClient = webServer.client();

        // アクティブなクライアント接続がある場合はポータル接続ありと判定
        if (hasActiveClient)
        {
            // 接続を検出したらフラグを設定
            portalConnectionDetected = true;

            // ログが繰り返し出力されるのを防止
            static bool detectionLogPrinted = false;
            if (!detectionLogPrinted)
            {
                Serial.println("WebServerに新規の接続を検出しました - portalConnectionDetected = true");
                Serial.println("【ポータル接続状態変化】: false → true");
                detectionLogPrinted = true;
            }
            return true;
        }

        // クライアント接続がない場合はfalseを返す
        return false;
    }

    // キャプティブポータルの接続検出フラグをリセットする
    void ESP32WiFiService::resetPortalConnectionDetected()
    {
        portalConnectionDetected = false;
        Serial.println("ポータル接続検出フラグをリセットしました");
    }

} // namespace Infrastructure