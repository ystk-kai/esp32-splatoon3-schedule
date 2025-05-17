# esp32-splatoon3-schedule

Splatoon3のバトルスケジュール（ナワバリ、Xマッチ、バンカラチャレンジ、バンカラオープン）をESP32とTFT液晶（例: ESP32-2432S028R + ILI9341）で表示するプロジェクトです。

- WiFi経由でAPIからスケジュール情報を取得します。
- 4分割画面で現在・次回のルール、ステージ、時間を表示します。
- ルール名・ステージ名は日本語→英語変換に対応しています。
- 画面下部に現在時刻と最終更新時刻を表示します。

## ハードウェア要件

- ESP32（例: ESP32-2432S028R）
- 320x240 TFT液晶（ILI9341等、TFT_eSPIライブラリ対応）
- WiFi環境

## ソフトウェア要件

- PlatformIO
- ライブラリ:
  - [TFT_eSPI](https://github.com/Bodmer/TFT_eSPI)
  - [ArduinoJson](https://arduinojson.org/)
  - WiFi, HTTPClient（ESP32標準）

## 使い方

```bash
# プロジェクトをビルド
pio run

# ESPにアップロード
pio run --target upload

# シリアルモニターを開く
pio device monitor
```

## 機能

- Splatoon3の4種バトル（Turf War, X Battle, Anarchy Challenge, Anarchy Open）の現在・次回スケジュールを表示
- ルール名・ステージ名を日本語→英語変換
- ルールごとに色分け・シンボル表示
- 画面下部に現在時刻・最終更新時刻を表示
- 5分ごとに自動更新
- 表示モード切り替え（英語/ローマ字）
    - 初期値はローマ字表示
- Wi-Fi設定の保存とキャプティブポータルによる設定変更
- Web設定画面による各種表示設定の変更

## 画面イメージ

![capture](assets/capture.jpg)

- 左上: レギュラーマッチ
- 右上: Xマッチ
- 左下: バンカラマッチ チャレンジ
- 右下: バンカラマッチ オープン
- 各象限に現在・次回の時間/ルール/ステージを表示
- 下部に現在時刻・最終更新時刻

## バトル種別・ルール・ステージ対応表

## バトル種別

| 日本語        | 英語              | ローマ字/略称        |
|---------------|-------------------|-------------------|
| レギュラーマッチ      | Regular Battle    | Regular Match     |
| Xマッチ          | X Battle          | X Match           |
| バンカラマッチ チャレンジ | Anarchy Challenge | Bankara Challenge |
| バンカラマッチ オープン  | Anarchy Open      | Bankara Open      |

### ルール

| 記号  | 日本語  | 英語          | ローマ字/略称 |
|-------|---------|---------------|------------|
|       | ナワバリバトル | Turf War      | Nawabari   |
| [-]   | ガチエリア   | Splat Zones   | Area       |
| \|^\| | ガチヤグラ   | Tower Control | Yagura     |
| {*}   | ガチホコバトル | Rainmaker     | Hoko       |
| (+)   | ガチアサリ   | Clam Blitz    | Asari      |

### ステージ

| 日本語         | 英語                   | ローマ字/略称 |
|----------------|------------------------|------------|
| ユノハナ大渓谷     | Scorch Gorge           | Yunohana   |
| ゴンズイ地区       | Eeltail Alley          | Gonzui     |
| ヤガラ市場        | Hagglefish Market      | Yagara     |
| マテガイ放水路     | Undertow Spillway      | Mategai    |
| ナンプラー遺跡      | Um'ami Ruins           | Nampla     |
| ナメロウ金属       | Mincemeat Metalworks   | Namero     |
| クサヤ温泉        | Brinewater Springs     | Kusaya     |
| タラポートショッピングパーク | Barnacle & Dime        | Taraport   |
| ヒラメが丘団地     | Flounder Heights       | Hirame     |
| マサバ海峡大橋    | Hammerhead Bridge      | Masaba     |
| キンメダイ美術館    | Museum d'Alfonsino     | Kinmedai   |
| マヒマヒリゾート＆スパ    | Mahi-Mahi Resort       | Mahimahi   |
| 海女美術大学   | Inkblot Art Academy    | Amabi      |
| チョウザメ造船      | Sturgeon Shipyard      | Chouzame   |
| ザトウマーケット       | Mako Mart              | Zatou      |
| スメーシーワールド      | Wahoo World            | Sume-shi   |
| コンブトラック        | Humpback Pump Track    | Kombu      |
| マンタマリア号       | Manta Maria            | Mantamaria |
| タカアシ経済特区   | Crableg Capital        | Takaashi   |
| オヒョウ海運       | Shipshape Cargo Co.    | Ohyou      |
| バイガイ亭         | Bayside Bowl           | Baigai     |
| ネギトロ炭鉱       | Bluefin Depot          | Negitoro   |
| カジキ空港        | Marlin Airport         | Kajiki     |
| リュウグウターミナル     | Dragon Palace Terminal | Ryuuguu    |

## 表示モード切り替え

このプロジェクトでは、テキスト表示モードを英語（English）とローマ字（Romaji）の間で切り替えることができます。  
以下の3つの表示設定が可能です：

1. バトル種別の表示モード
   ```cpp
   #define BATTLETYPE_DISPLAY_ENGLISH 0  // 英語表示
   #define BATTLETYPE_DISPLAY_ROMAJI  1  // ローマ字表示
   #define BATTLETYPE_DISPLAY_MODE BATTLETYPE_DISPLAY_ENGLISH  // 表示モード選択
   ```

2. ルールの表示モード
   ```cpp
   #define RULE_DISPLAY_ENGLISH 0  // 英語表示
   #define RULE_DISPLAY_ROMAJI  1  // ローマ字表示
   #define RULE_DISPLAY_MODE RULE_DISPLAY_ENGLISH  // 表示モード選択
   ```

3. ステージの表示モード
   ```cpp
   #define STAGE_DISPLAY_ENGLISH 0  // 英語表示
   #define STAGE_DISPLAY_ROMAJI  1  // ローマ字表示
   #define STAGE_DISPLAY_MODE STAGE_DISPLAY_ENGLISH  // 表示モード選択
   ```

表示モードを変更するには、`src/utils/constants.h`ファイル内の対応する`#define`文を編集してください。  
英語表示モードの場合は値を`0`に、ローマ字表示モードの場合は値を`1`に設定します。

## Setup and Connection

### 1. USBデバイスをWSL2で利用可能にする方法

#### usbipd-winを使用する方法

1. **usbipd-winのインストール**（Windows側）
   
   PowerShellを管理者として開き、以下のコマンドを実行します：
   ```powershell
   winget install dorssel.usbipd-win
   ```

2. **WSL2側の設定**

   WSL2のターミナルで以下のコマンドを実行します：
   ```bash
   sudo apt update
   sudo apt install linux-tools-generic hwdata
   sudo update-alternatives --install /usr/local/bin/usbip usbip /usr/lib/linux-tools/*/usbip 20
   ```

3. **デバイスの確認とアタッチ**（Windows側）

   PowerShellを管理者として開き、以下のコマンドを実行します：
   ```powershell
   # 接続されているUSBデバイスの一覧を表示
   usbipd list
   
   # COM4に対応するデバイスIDを見つけ、アタッチする
   # 例: デバイスIDが1-8の場合
   usbipd bind --busid 1-8
   usbipd attach --wsl --busid 1-8
   ```

4. **WSL2側での確認**

   WSL2のターミナルで以下のコマンドを実行します：
   ```bash
   # デバイスが認識されているか確認
   lsusb
   dmesg | grep tty
   ```

   シリアルデバイスは通常、WSL2内で `/dev/ttyUSB0` や `/dev/ttyACM0` などとして認識されます。

### 2. シリアルポートを使用する

デバイスが正しく認識されたら、以下のようにアクセスできます：

```bash
# シリアルポートの確認
ls -l /dev/tty*

# シリアルポートに接続する例（例: 115200 bpsの場合）
sudo apt install screen
sudo screen /dev/ttyUSB0 115200
```


## 参考API

- [spla3.yuu26.com](https://spla3.yuu26.com/)
