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

- Arduino IDE
- ライブラリ:
  - [TFT_eSPI](https://github.com/Bodmer/TFT_eSPI)
  - [ArduinoJson](https://arduinojson.org/)
  - WiFi, HTTPClient（ESP32標準）

## 使い方

1. `splatoon3_schedule.ino` をArduino IDEで開きます。
2. WiFi設定（`ssid`, `password`）を自分の環境に合わせて書き換えます。
3. 必要なライブラリをインストールします。
4. ESP32に書き込みます。

## 機能

- Splatoon3の4種バトル（Turf War, X Battle, Anarchy Challenge, Anarchy Open）の現在・次回スケジュールを表示
- ルール名・ステージ名を日本語→英語変換
- ルールごとに色分け・シンボル表示
- 画面下部に現在時刻・最終更新時刻を表示
- 10分ごとに自動更新
- 表示モード切り替え（英語/ローマ字）

## 画面イメージ

![capture](assets/capture.jpg)

- 左上: Turf War（ナワバリ）
- 右上: X Battle（Xマッチ）
- 左下: Anarchy Challenge（バンカラチャレンジ）
- 右下: Anarchy Open（バンカラオープン）
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

表示モードを変更するには、`splatoon3_schedule.ino`ファイル内の対応する`#define`文を編集してください。  
英語表示モードの場合は値を`0`に、ローマ字表示モードの場合は値を`1`に設定します。

例えば、バトル種別をローマ字表示に、ルールと場所を英語表示にするには：

```cpp
#define BATTLETYPE_DISPLAY_MODE BATTLETYPE_DISPLAY_ROMAJI  // ローマ字表示 (1)
#define RULE_DISPLAY_MODE RULE_DISPLAY_ENGLISH             // 英語表示 (0)
#define STAGE_DISPLAY_MODE STAGE_DISPLAY_ENGLISH           // 英語表示 (0)
```

## 参考API
- [spla3.yuu26.com](https://spla3.yuu26.com/)
