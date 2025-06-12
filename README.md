# esp32-splatoon3-schedule

Splatoon3 のバトルスケジュール（ナワバリ、X マッチ、バンカラチャレンジ、バンカラオープン）を ESP32 と TFT 液晶（例: ESP32-2432S028R + ILI9341）で表示するプロジェクトです。

- WiFi 経由で API からスケジュール情報を取得します。
- 4分割画面で現在・次回のルール、ステージ、時間を表示します。
- ルール名・ステージ名は日本語→英語変換に対応しています。
- 画面下部に現在時刻と最終更新時刻を表示します。

## ハードウェア要件

- ESP32（例: ESP32-2432S028R）
- 320x240 TFT 液晶（ILI9341 等、TFT_eSPI ライブラリ対応）
- WiFi 環境

## ソフトウェア要件

- PlatformIO
- ライブラリ:
  - [TFT_eSPI](https://github.com/Bodmer/TFT_eSPI)
  - [ArduinoJson](https://arduinojson.org/)
  - WiFi, HTTPClient（ESP32 標準）

## 使い方

```bash
# プロジェクトをビルド
pio run

# ESP にアップロード
pio run --target upload

# シリアルモニターを開く
pio device monitor
```

## 機能

- Splatoon3 の 4種バトル（Turf War, X Battle, Anarchy Challenge, Anarchy Open）の現在・次回スケジュールを表示
- ルール名・ステージ名を日本語→英語変換
- ルールごとに色分け・シンボル表示
- 画面下部に現在時刻・最終更新時刻を表示
- 5分ごとに自動更新
- Wi-Fi 設定の保存とキャプティブポータルによる設定変更
- Web 設定画面による各種表示設定の変更

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
| デカライン高架下     | Urchin Underpass       | Dekaline   |

## WiFi 設定（キャプティブポータル）

このプロジェクトでは、キャプティブポータルを使用して WiFi 設定を行います。

### 初回起動時／WiFi 設定がない場合

1. デバイスの電源を入れると、自動的にキャプティブポータルモードで起動します
2. 「ESP32-Splatoon3-Schedule」という SSID の WiFi アクセスポイントが作成されます
3. スマートフォンや PC でこのアクセスポイントに接続します
4. 接続すると自動的に設定ページが表示されるか、表示されない場合は「192.168.4.1」にブラウザでアクセスします
5. 表示された設定画面で、接続したい WiFi の SSID とパスワードを入力します
6. 設定を保存すると ESP32 が再起動し、設定した WiFi ネットワークに接続します

### 既存の WiFi 設定がある場合

1. デバイスの電源を入れると、起動時に 一定時間だけキャプティブポータルモードになります
2. その間に設定を変更する場合は、「ESP32-Splatoon3-Schedule」という SSID に接続し設定を変更できます
3. 何も操作がなければ、自動的に保存済みの WiFi 設定で接続を開始します
4. キャプティブポータルに接続すると、タイマーが一時停止し、設定を変更するための時間が確保されます

### 表示モード設定（英語/ローマ字）

キャプティブポータルの設定画面では、以下の表示モード設定が可能です：

1. バトル種別の表示モード - ローマ字（例: Regular Match）または英語（例: Regular Battle）
2. ルールの表示モード - ローマ字（例: Area）または英語（例: Splat Zones）
3. ステージの表示モード - ローマ字（例: Yunohana）または英語（例: Scorch Gorge）

これらの設定は Preferences に保存され、デバイス再起動後も維持されます。
初期値はすべてローマ字表示に設定されています。

### WiFi 接続状態の確認

- WiFi 設定中は画面上部がオレンジ色で「WiFi Setup」と表示されます
- 設定した WiFi に接続中は「Connecting to WiFi」と表示されます
- 接続成功すると画面上部が緑色になり「Connection OK」と表示されます
- 画面には SSID や IP アドレスが色分けされて表示され、接続状態が一目でわかるようになっています

### 設定後の動作

1. WiFi 設定を保存すると、設定は内部ストレージ（Preferences）に保存されます
2. 設定保存後、デバイスは自動的に再起動します
3. 再起動後、保存した WiFi 設定を使用して接続を試みます
4. 接続に成功すると、Splatoon3 のスケジュール情報を自動的に取得して表示を開始します
5. 表示モード設定（英語/ローマ字）も保存され、スケジュール表示に反映されます

正常に接続されると、自動的に Splatoon3 のスケジュール情報を取得して表示を開始します。スケジュール情報は 5分ごとに自動更新され、画面下部に最終更新時刻が表示されます。

## Setup and Connection

### 1. USB デバイスを WSL2 で利用可能にする方法

#### usbipd-win を使用する方法

1. **usbipd-win のインストール**（Windows 側）
   
   PowerShell を管理者として開き、以下のコマンドを実行します：
   ```powershell
   winget install dorssel.usbipd-win
   ```

2. **WSL2 側の設定**

   WSL2 のターミナルで以下のコマンドを実行します：
   ```bash
   sudo apt update
   sudo apt install linux-tools-generic hwdata
   sudo update-alternatives --install /usr/local/bin/usbip usbip /usr/lib/linux-tools/*/usbip 20
   ```

3. **デバイスの確認とアタッチ**（Windows 側）

   PowerShell を管理者として開き、以下のコマンドを実行します：
   ```powershell
   # 接続されている USB デバイスの一覧を表示
   usbipd list
   
   # COM4 に対応するデバイス ID を見つけ、アタッチする
   # 例: デバイス ID が 1-8 の場合
   usbipd bind --busid 1-8
   usbipd attach --wsl --busid 1-8
   ```

4. **WSL2 側での確認**

   WSL2 のターミナルで以下のコマンドを実行します：
   ```bash
   # デバイスが認識されているか確認
   lsusb
   dmesg | grep tty
   ```

   シリアルデバイスは通常、WSL2 内で `/dev/ttyUSB0` や `/dev/ttyACM0` などとして認識されます。

### 2. シリアルポートを使用する

デバイスが正しく認識されたら、以下のようにアクセスできます：

```bash
# シリアルポートの確認
ls -l /dev/tty*

# シリアルポートに接続する例（例: 115200 bps の場合）
sudo apt install screen
sudo screen /dev/ttyUSB0 115200
```


## 参考 API

- [spla3.yuu26.com](https://spla3.yuu26.com/)
