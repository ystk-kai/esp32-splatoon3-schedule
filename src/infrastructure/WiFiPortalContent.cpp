// WiFiPortalContent.cpp
// キャプティブポータル用のHTML、CSS、JavaScriptコンテンツの実装

#include "WiFiPortalContent.h"

// 外部HTMLファイルの内容をPROGMEMに取り込む
namespace Infrastructure
{
    // HTMLファイルの内容をR"()"構文を使って埋め込む
    const char PORTAL_HTML[] PROGMEM = R"rawliteral(<!DOCTYPE html>
<html lang="ja">

<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>ESP32 Splatoon3 Schedule</title>
    <style>
        :root {
            --primary-color: #00a0e9;
            --accent-color: #f39800;
            --bg-color: #f5f5f5;
            --text-color: #333;
            --border-color: #ddd;
            --error-color: #e74c3c;
            --success-color: #2ecc71;
        }

        * {
            box-sizing: border-box;
            margin: 0;
            padding: 0;
        }

        body {
            font-family: 'Helvetica Neue', Arial, sans-serif;
            background: var(--bg-color);
            color: var(--text-color);
            line-height: 1.6;
        }

        .container {
            max-width: 480px;
            margin: 0 auto;
            padding: 20px;
        }

        header {
            text-align: center;
            margin-bottom: 30px;
        }

        h1 {
            font-size: 28px;
            margin-bottom: 10px;
            color: var(--primary-color);
        }

        .form-card {
            background: #fff;
            border-radius: 8px;
            box-shadow: 0 2px 10px rgba(0, 0, 0, 0.1);
            padding: 20px;
            margin-bottom: 20px;
        }

        .form-group {
            margin-bottom: 20px;
        }

        label {
            display: block;
            font-weight: bold;
            margin-bottom: 5px;
        }

        input[type="text"],
        input[type="password"],
        select {
            width: 100%;
            padding: 10px;
            border: 1px solid var(--border-color);
            border-radius: 4px;
            font-size: 16px;
        }

        input[type="checkbox"] {
            margin-right: 8px;
        }

        button {
            background-color: var(--primary-color);
            color: white;
            border: none;
            padding: 12px 20px;
            border-radius: 4px;
            cursor: pointer;
            font-size: 16px;
            width: 100%;
            font-weight: bold;
            transition: background-color 0.3s;
        }

        button:hover {
            background-color: #0080c0;
        }

        .networks-list {
            max-height: 200px;
            overflow-y: auto;
            border: 1px solid var(--border-color);
            border-radius: 4px;
            margin-bottom: 15px;
        }

        .network-item {
            padding: 10px 15px;
            border-bottom: 1px solid var(--border-color);
            cursor: pointer;
            transition: background-color 0.2s;
            display: flex;
            align-items: center;
            justify-content: space-between;
        }

        .network-item:hover {
            background-color: #f0f0f0;
        }

        .network-item:last-child {
            border-bottom: none;
        }

        .signal-strength {
            display: inline-block;
            width: 20px;
            margin-left: 10px;
        }

        .signal-bar {
            margin-left: 10px;
            width: 40px;
            height: 16px;
            position: relative;
            display: inline-block;
        }

        .signal-bar:before {
            content: "";
            position: absolute;
            top: 0;
            left: 0;
            height: 100%;
            background-color: var(--primary-color);
        }

        .signal-level-1:before {
            width: 25%;
        }

        .signal-level-2:before {
            width: 50%;
        }

        .signal-level-3:before {
            width: 75%;
        }

        .signal-level-4:before {
            width: 100%;
        }

        .hidden {
            display: none;
        }

        .ip-fields {
            margin-top: 15px;
        }

        .text-center {
            text-align: center;
        }

        .tab-buttons {
            display: flex;
            margin-bottom: 20px;
        }

        .tab-button {
            flex: 1;
            padding: 10px;
            border: 1px solid var(--border-color);
            background: #f0f0f0;
            cursor: pointer;
            text-align: center;
            font-weight: bold;
        }

        .tab-button.active {
            background: var(--primary-color);
            color: white;
            border-color: var(--primary-color);
        }

        .tab-button:first-child {
            border-radius: 4px 0 0 4px;
        }

        .tab-button:last-child {
            border-radius: 0 4px 4px 0;
        }

        .tab-content {
            display: none;
        }

        .tab-content.active {
            display: block;
        }

        .status-message {
            padding: 10px;
            border-radius: 4px;
            margin-bottom: 15px;
            display: none;
        }

        .error {
            background-color: #fde0dc;
            color: var(--error-color);
            display: block;
        }

        .success {
            background-color: #d4edda;
            color: var(--success-color);
            display: block;
        }

        .secure-network {
            display: inline-block;
            width: 16px;
            height: 16px;
            margin-right: 5px;
            background-image: url('data:image/svg+xml;utf8,<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 20 20" fill="currentColor"><path fill-rule="evenodd" d="M5 9V7a5 5 0 0110 0v2a2 2 0 012 2v5a2 2 0 01-2 2H5a2 2 0 01-2-2v-5a2 2 0 012-2zm8-2v2H7V7a3 3 0 016 0z" clip-rule="evenodd" /></svg>');
            background-size: contain;
        }

        .refresh-button {
            background: none;
            border: none;
            color: var(--primary-color);
            cursor: pointer;
            font-size: 14px;
            padding: 5px 10px;
            width: auto;
            display: inline-block;
            text-decoration: underline;
        }

        @media (max-width: 480px) {
            .container {
                padding: 10px;
            }

            h1 {
                font-size: 24px;
            }

            .form-card {
                padding: 15px;
            }
        }

        .loading {
            display: inline-block;
            width: 20px;
            height: 20px;
            border: 3px solid rgba(0, 0, 0, 0.1);
            border-radius: 50%;
            border-top-color: var(--primary-color);
            animation: spin 1s ease-in-out infinite;
            margin-left: 10px;
            vertical-align: middle;
        }

        .network-settings {
            margin-top: 20px;
            padding-top: 15px;
            border-top: 1px solid var(--border-color);
        }

        .network-settings h4 {
            margin-bottom: 15px;
            color: var(--primary-color);
        }

        .button-group {
            display: flex;
            gap: 10px;
            justify-content: center;
        }

        .setting-button {
            background-color: #f0f0f0;
            color: var(--text-color);
            border: 1px solid var(--border-color);
            padding: 8px 15px;
            border-radius: 4px;
            cursor: pointer;
            font-size: 14px;
            flex: 1;
            max-width: 150px;
            transition: all 0.2s;
        }

        .setting-button:hover {
            background-color: var(--primary-color);
            color: white;
        }

        @keyframes spin {
            to {
                transform: rotate(360deg);
            }
        }
    </style>
</head>

<body>
    <div class="container">
        <header>
            <h1>ESP32 Splatoon3 Schedule</h1>
            <p>Wi-Fiと表示設定を行います</p>
        </header>

        <div class="tab-buttons">
            <div class="tab-button active" data-tab="wifi-tab">WiFi設定</div>
            <div class="tab-button" data-tab="display-tab">表示設定</div>
        </div>

        <div id="status-message" class="status-message"></div>

        <div id="wifi-tab" class="tab-content active">
            <div class="form-card">
                <div class="form-group">
                    <label for="ssid">利用可能なネットワーク</label>
                    <button type="button" id="refresh-networks" class="refresh-button">
                        <span>更新</span>
                        <span id="loading-spinner" class="loading hidden"></span>
                    </button>
                    <div class="networks-list" id="networks-list">
                        <div class="text-center">スキャン中...</div>
                    </div>
                </div>

                <form id="wifi-form">
                    <div class="form-group">
                        <label for="ssid">SSID</label>
                        <input type="text" id="ssid" name="ssid" required>
                    </div>
                    <div class="form-group">
                        <label for="password">パスワード</label>
                        <input type="password" id="password" name="password">
                    </div>
                    
                    <div class="form-group network-settings">
                        <h4>ネットワーク設定</h4>
                        <div class="form-group">
                            <label>
                                <input type="checkbox" id="dhcp" name="dhcp" checked>
                                DHCPを使用（自動設定）
                            </label>
                        </div>
                        <div class="ip-fields hidden">
                            <div class="form-group">
                                <label for="ip">IPアドレス</label>
                                <input type="text" id="ip" name="ip" placeholder="例: 192.168.1.100">
                            </div>
                            <div class="form-group">
                                <label for="gateway">ゲートウェイ</label>
                                <input type="text" id="gateway" name="gateway" placeholder="例: 192.168.1.1">
                            </div>
                            <div class="form-group">
                                <label for="subnet">サブネットマスク</label>
                                <input type="text" id="subnet" name="subnet" placeholder="例: 255.255.255.0">
                            </div>
                            <div class="form-group">
                                <label for="dns1">DNS1</label>
                                <input type="text" id="dns1" name="dns1" placeholder="例: 8.8.8.8">
                            </div>
                            <div class="form-group">
                                <label for="dns2">DNS2 (オプション)</label>
                                <input type="text" id="dns2" name="dns2" placeholder="例: 8.8.4.4">
                            </div>
                        </div>
                    </div>
                </form>
            </div>
        </div>

        <div id="display-tab" class="tab-content">
            <div class="form-card">
                <form id="display-form">
                    <div class="form-group">
                        <h3>表示言語設定</h3>
                        <p>Splatoon3の情報表示方法を選択できます。</p>
                    </div>
                    <div class="form-group">
                        <label>
                            <input type="checkbox" id="battle_romaji" name="battle_romaji">
                            バトルタイプを英語で表示
                        </label>
                    </div>
                    <div class="form-group">
                        <label>
                            <input type="checkbox" id="rule_romaji" name="rule_romaji">
                            ルールを英語で表示
                        </label>
                    </div>
                    <div class="form-group">
                        <label>
                            <input type="checkbox" id="stage_romaji" name="stage_romaji">
                            ステージを英語で表示
                        </label>
                    </div>
                    <div class="form-group button-group">
                        <button type="button" id="english-button" class="setting-button">英語表示</button>
                        <button type="button" id="romaji-button" class="setting-button">ローマ字表示</button>
                    </div>
                </form>
            </div>
        </div>

        <button id="save-button">保存して接続</button>
    </div>

    <script>
        document.addEventListener('DOMContentLoaded', function () {
            // タブ切り替え
            const tabButtons = document.querySelectorAll('.tab-button');
            const tabContents = document.querySelectorAll('.tab-content');

            tabButtons.forEach(button => {
                button.addEventListener('click', () => {
                    const tabId = button.getAttribute('data-tab');

                    tabButtons.forEach(btn => btn.classList.remove('active'));
                    tabContents.forEach(content => content.classList.remove('active'));

                    button.classList.add('active');
                    document.getElementById(tabId).classList.add('active');
                });
            });

            // DHCPチェックボックスの処理
            const dhcpCheckbox = document.getElementById('dhcp');
            const ipFields = document.querySelector('.ip-fields');

            dhcpCheckbox.addEventListener('change', () => {
                if (dhcpCheckbox.checked) {
                    ipFields.classList.add('hidden');
                } else {
                    ipFields.classList.remove('hidden');
                }
            });

            // 表示設定のボタン
            const englishButton = document.getElementById('english-button');
            const romajiButton = document.getElementById('romaji-button');
            const battleRomajiCheckbox = document.getElementById('battle_romaji');
            const ruleRomajiCheckbox = document.getElementById('rule_romaji');
            const stageRomajiCheckbox = document.getElementById('stage_romaji');

            // 英語表示ボタンのイベントリスナー
            englishButton.addEventListener('click', () => {
                battleRomajiCheckbox.checked = true;  // 英語表示にチェック
                ruleRomajiCheckbox.checked = true;    // 英語表示にチェック
                stageRomajiCheckbox.checked = true;   // 英語表示にチェック
            });

            // ローマ字表示ボタンのイベントリスナー（デフォルト）
            romajiButton.addEventListener('click', () => {
                battleRomajiCheckbox.checked = false;  // ローマ字表示のチェックを外す
                ruleRomajiCheckbox.checked = false;    // ローマ字表示のチェックを外す
                stageRomajiCheckbox.checked = false;   // ローマ字表示のチェックを外す
            });

            // 設定を読み込む（WiFi設定と表示設定）
            function loadSettings() {
                console.log('設定を読み込んでいます...');
                // サーバーから設定を取得
                fetch('/settings')
                    .then(response => response.json())
                    .then(data => {
                        console.log('取得した設定:', data);
                        
                        // 表示設定の読み込み
                        if (data.display) {
                            // チェックボックスの表示を逆転: trueなら英語表示(=チェック有)、falseならローマ字表示(=チェック無)
                            battleRomajiCheckbox.checked = !data.display.battle_romaji;
                            ruleRomajiCheckbox.checked = !data.display.rule_romaji;
                            stageRomajiCheckbox.checked = !data.display.stage_romaji;
                        }
                        
                        // WiFi設定の読み込み
                        if (data.wifi && data.wifi.ssid) {
                            document.getElementById('ssid').value = data.wifi.ssid;
                            // パスワードは安全上の理由で表示しない
                            
                            // DHCP設定の読み込み
                            if (data.wifi.dhcp !== undefined) {
                                document.getElementById('dhcp').checked = data.wifi.dhcp;
                                if (!data.wifi.dhcp) {
                                    document.querySelector('.ip-fields').classList.remove('hidden');
                                    
                                    // 静的IP設定の読み込み
                                    if (data.wifi.ip) document.getElementById('ip').value = data.wifi.ip;
                                    if (data.wifi.gateway) document.getElementById('gateway').value = data.wifi.gateway;
                                    if (data.wifi.subnet) document.getElementById('subnet').value = data.wifi.subnet;
                                    if (data.wifi.dns1) document.getElementById('dns1').value = data.wifi.dns1;
                                    if (data.wifi.dns2) document.getElementById('dns2').value = data.wifi.dns2;
                                }
                            }
                        }
                    })
                    .catch(error => {
                        console.error('Error loading settings:', error);
                    });
            }

            // 初期設定読み込み
            loadSettings();

            // WiFiネットワークスキャン
            const refreshButton = document.getElementById('refresh-networks');
            const loadingSpinner = document.getElementById('loading-spinner');
            const networksList = document.getElementById('networks-list');

            function scanNetworks() {
                console.log('Wi-Fiネットワークスキャンを開始します');
                
                // 連続クリックを防止
                refreshButton.disabled = true;
                
                loadingSpinner.classList.remove('hidden');
                networksList.innerHTML = '<div class="text-center">スキャン中...</div>';
                
                // XMLHttpRequestを使用（fetch APIの代わりに）
                const xhr = new XMLHttpRequest();
                xhr.open('GET', '/scan', true);
                
                xhr.onreadystatechange = function() {
                    if (xhr.readyState === 4) {
                        console.log('スキャン応答受信:', xhr.status);
                        loadingSpinner.classList.add('hidden');
                        
                        // スキャン完了後、ボタンを再度有効化（最低1秒後）
                        setTimeout(() => {
                            refreshButton.disabled = false;
                        }, 1000);
                        
                        if (xhr.status === 200) {
                            try {
                                const response = JSON.parse(xhr.responseText);
                                console.log('スキャンデータ:', response);
                                networksList.innerHTML = '';
                                
                                if (response.networks && response.networks.length > 0) {
                                    // シグナル強度でソート
                                    response.networks.sort((a, b) => b.rssi - a.rssi);
                                    
                                    response.networks.forEach(network => {
                                        const signalLevel = getSignalLevel(network.rssi);
                                        const isSecure = network.secure;
                                        
                                        const networkItem = document.createElement('div');
                                        networkItem.className = 'network-item';
                                        networkItem.innerHTML = `
                                            <span>${isSecure ? '<span class="secure-network"></span>' : ''}${network.ssid}</span>
                                            <span class="signal-bar signal-level-${signalLevel}"></span>
                                        `;
                                        
                                        networkItem.addEventListener('click', () => {
                                            document.getElementById('ssid').value = network.ssid;
                                            if (!isSecure) {
                                                document.getElementById('password').value = '';
                                            }
                                            // パスワード欄にフォーカス（セキュアネットワークの場合）
                                            if (isSecure) {
                                                document.getElementById('password').focus();
                                            }
                                        });
                                        
                                        networksList.appendChild(networkItem);
                                    });
                                } else {
                                    networksList.innerHTML = '<div class="text-center">ネットワークが見つかりませんでした</div>';
                                }
                            } catch (e) {
                                console.error('JSONパースエラー:', e, xhr.responseText);
                                networksList.innerHTML = '<div class="text-center">レスポンスの解析に失敗しました</div>';
                            }
                        } else {
                            networksList.innerHTML = '<div class="text-center">スキャンに失敗しました <button type="button" class="refresh-button" onclick="scanNetworks()">再試行</button></div>';
                            console.error('スキャンHTTPエラー:', xhr.status);
                        }
                    }
                };
                
                xhr.onerror = function() {
                    console.error('スキャン通信エラー');
                    loadingSpinner.classList.add('hidden');
                    refreshButton.disabled = false;
                    networksList.innerHTML = '<div class="text-center">サーバーとの通信に失敗しました <button type="button" class="refresh-button" onclick="scanNetworks()">再試行</button></div>';
                };
                
                xhr.ontimeout = function() {
                    console.error('スキャンタイムアウト');
                    loadingSpinner.classList.add('hidden');
                    refreshButton.disabled = false;
                    networksList.innerHTML = '<div class="text-center">タイムアウトしました <button type="button" class="refresh-button" onclick="scanNetworks()">再試行</button></div>';
                };
                
                xhr.timeout = 10000; // 10秒タイムアウト
                console.log('スキャンリクエスト送信');
                xhr.send();
            }

            function getSignalLevel(rssi) {
                if (rssi >= -50) return 4;
                if (rssi >= -65) return 3;
                if (rssi >= -75) return 2;
                return 1;
            }

            refreshButton.addEventListener('click', scanNetworks);

            // 初回スキャン
            scanNetworks();

            // フォーム送信
            const saveButton = document.getElementById('save-button');
            const wifiForm = document.getElementById('wifi-form');
            const displayForm = document.getElementById('display-form');
            const statusMessage = document.getElementById('status-message');

            // サーバーに設定を送信する関数
            function submitSettings() {
                console.log('送信処理を開始します');
                
                // 基本的なバリデーション
                const ssid = document.getElementById('ssid').value.trim();
                if (!ssid) {
                    showMessage('SSIDを入力してください', 'error');
                    return false;
                }

                // DHCPが無効の場合はIPアドレスのバリデーション
                if (!dhcpCheckbox.checked) {
                    const ip = document.getElementById('ip').value.trim();
                    const gateway = document.getElementById('gateway').value.trim();
                    const subnet = document.getElementById('subnet').value.trim();

                    if (!ip || !gateway || !subnet) {
                        showMessage('IPアドレス、ゲートウェイ、サブネットマスクは必須です', 'error');
                        return false;
                    }

                    // 簡易的なIPアドレスバリデーション
                    const ipRegex = /^(\d{1,3}\.){3}\d{1,3}$/;
                    if (!ipRegex.test(ip) || !ipRegex.test(gateway) || !ipRegex.test(subnet)) {
                        showMessage('正しいIPアドレス形式で入力してください', 'error');
                        return false;
                    }
                }

                // 送信中の表示に変更
                saveButton.disabled = true;
                saveButton.textContent = '保存中...';
                showMessage('設定を保存しています...', 'success');

                // フォームデータの構築
                const formData = new FormData();
                formData.append('ssid', ssid);
                
                // パスワードが空の場合は、既存のパスワードをそのまま使用するための特殊処理
                const password = document.getElementById('password').value;
                formData.append('password', password);
                
                formData.append('dhcp', dhcpCheckbox.checked ? '1' : '0');
                
                // チェックボックスの値を反転して送信: チェックあり=英語表示=0、チェックなし=ローマ字表示=1
                formData.append('battle_romaji', !battleRomajiCheckbox.checked ? '1' : '0');
                formData.append('rule_romaji', !ruleRomajiCheckbox.checked ? '1' : '0');
                formData.append('stage_romaji', !stageRomajiCheckbox.checked ? '1' : '0');

                if (!dhcpCheckbox.checked) {
                    formData.append('ip', document.getElementById('ip').value);
                    formData.append('gateway', document.getElementById('gateway').value);
                    formData.append('subnet', document.getElementById('subnet').value);
                    formData.append('dns1', document.getElementById('dns1').value);
                    formData.append('dns2', document.getElementById('dns2').value);
                }

                // URLエンコードされた文字列に変換
                const formBody = Array.from(formData.entries())
                    .map(([key, value]) => `${encodeURIComponent(key)}=${encodeURIComponent(value)}`)
                    .join('&');

                console.log('送信データ:', formBody);

                // XMLHttpRequest を使用して同期的に送信（fetch APIより信頼性が高い場合がある）
                const xhr = new XMLHttpRequest();
                xhr.open('POST', '/save', true);
                xhr.setRequestHeader('Content-Type', 'application/x-www-form-urlencoded');
                
                xhr.onreadystatechange = function() {
                    if (xhr.readyState === 4) {
                        console.log('応答状態:', xhr.status);
                        
                        if (xhr.status === 200) {
                            console.log('保存成功');
                            
                            // 成功メッセージを表示
                            const statusDiv = document.createElement('div');
                            statusDiv.className = 'form-card';
                            statusDiv.innerHTML = `
                                <h3>設定が保存されました</h3>
                                <p>ESP32が再起動し、指定したWiFiネットワークに接続しています。</p>
                                <p>数秒後にESP32のディスプレイに接続状態が表示されます。</p>
                                <p>このウィンドウは閉じて構いません。</p>
                            `;
                            
                            // ページ内容をクリアして新しいメッセージだけ表示
                            document.querySelector('.container').innerHTML = '';
                            document.querySelector('.container').appendChild(statusDiv);
                        } else {
                            console.log('保存失敗:', xhr.responseText);
                            showMessage('保存に失敗しました: ' + xhr.responseText, 'error');
                            saveButton.disabled = false;
                            saveButton.textContent = '保存して接続';
                        }
                    }
                };
                
                xhr.onerror = function() {
                    console.log('通信エラー');
                    showMessage('通信エラーが発生しました', 'error');
                    saveButton.disabled = false;
                    saveButton.textContent = '保存して接続';
                };
                
                console.log('リクエスト送信開始');
                xhr.send(formBody);
                console.log('リクエスト送信完了');
                
                return true;
            }

            // 保存ボタンのクリックイベント
            saveButton.addEventListener('click', function(e) {
                e.preventDefault(); // フォームのデフォルト送信を防止
                submitSettings();
            });

            function showMessage(message, type) {
                statusMessage.textContent = message;
                statusMessage.className = 'status-message ' + type;
                statusMessage.style.display = 'block';
            }
        });
    </script>
</body>

</html>)rawliteral";

    // 注記: 本来は以下のようにwifi_portal.htmlファイルを読み込む仕組みが望ましいですが、
    // ESP32環境ではファイルシステムへのアクセス方法が異なるため、代替手段として上記のように
    // HTMLコンテンツをハードコードしています。
    //
    // 完全なファイル分離を実現するには、以下のような方法が考えられます：
    // 1. SPIFFS/LittleFSを使用してフラッシュメモリからファイルを読み込む
    // 2. ビルド時にスクリプトを使ってHTMLファイルをC++のヘッダーファイルに変換する
    // 3. ESP-IDFのembedded_files機能を使用する
} // namespace Infrastructure