#include <TFT_eSPI.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <time.h>

// WiFi settings
const char* ssid = "SSID名";
const char* password = "パスワード";

// API URLs
const char* apiUrlRegular = "https://spla3.yuu26.com/api/regular/now";
const char* apiUrlRegularNext = "https://spla3.yuu26.com/api/regular/next";
const char* apiUrlXMatch = "https://spla3.yuu26.com/api/x/now";
const char* apiUrlXMatchNext = "https://spla3.yuu26.com/api/x/next";
const char* apiUrlBankaraChallenge = "https://spla3.yuu26.com/api/bankara-challenge/now";
const char* apiUrlBankaraChallengeNext = "https://spla3.yuu26.com/api/bankara-challenge/next";
const char* apiUrlBankaraOpen = "https://spla3.yuu26.com/api/bankara-open/now";
const char* apiUrlBankaraOpenNext = "https://spla3.yuu26.com/api/bankara-open/next";

// Display settings
#define TFT_BL_PIN 21
#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240
#define QUADRANT_WIDTH (SCREEN_WIDTH / 2)
#define QUADRANT_HEIGHT (SCREEN_HEIGHT / 2)

// Custom colors for each battle type (using provided colors)
#define COLOR_REGULAR 0x7FE0            // Chartreuse (#7FFF00)
#define COLOR_XMATCH 0x3E9C             // MediumTurquoise (#48D1CC)
#define COLOR_BANKARA_CHALLENGE 0xFD00  // Orange (#FFA500)
#define COLOR_BANKARA_OPEN 0xFD00       // Orange (#FFA500) - Same as Challenge

// ルールごとのシンボル色を定義
#define COLOR_RULE_AREA 0x07E0    // 緑色 (Green)
#define COLOR_RULE_YAGURA 0x15BD  // 青色 (Blue)
#define COLOR_RULE_HOKO 0xFE60    // 金色/黄色 (Gold/Yellow)
#define COLOR_RULE_ASARI 0x0BE0   // 濃い緑色 (Olive Green)

TFT_eSPI tft = TFT_eSPI();

// Battle type translations
struct BattleTypeTranslation {
  const char* japanese;
  const char* english;
  const char* romaji;
};

#define BATTLETYPE_DISPLAY_ENGLISH 0
#define BATTLETYPE_DISPLAY_ROMAJI  1
#ifndef BATTLETYPE_DISPLAY_MODE
// 表示モードを英語/ローマ字に設定
#define BATTLETYPE_DISPLAY_MODE BATTLETYPE_DISPLAY_ROMAJI
#endif

const BattleTypeTranslation battleTypeTranslations[] = {
  { "レギュラーマッチ",        "Regular Battle",    "Regular Match" },
  { "Xマッチ",                "X Battle",          "X Match" },
  { "バンカラマッチ チャレンジ", "Anarchy Challenge", "Bankara Challenge" },
  { "バンカラマッチ オープン",  "Anarchy Open",      "Bankara Open" }
};
const int battleTypeTranslationsCount = sizeof(battleTypeTranslations) / sizeof(BattleTypeTranslation);

// Rule translations
struct RuleTranslation {
  const char* japanese;
  const char* english;
  const char* romaji;
  const char* symbol;  // ルールの記号
};

#define RULE_DISPLAY_ENGLISH 0
#define RULE_DISPLAY_ROMAJI  1
#ifndef RULE_DISPLAY_MODE
// 表示モードを英語/ローマ字に設定
#define RULE_DISPLAY_MODE RULE_DISPLAY_ROMAJI
#endif

const RuleTranslation ruleTranslations[] = {
  { "ナワバリバトル", "Turf War",    "Nawabari", "" },
  { "ガチエリア",     "Splat Zones", "Area",     "[-] " },
  { "ガチヤグラ",     "Tower Control","Yagura",   "|^| " },
  { "ガチホコバトル", "Rainmaker",   "Hoko",     "{*} " },
  { "ガチアサリ",     "Clam Blitz",  "Asari",    "(+) " }
};
const int ruleTranslationsCount = sizeof(ruleTranslations) / sizeof(RuleTranslation);

// Stage translations (add romaji/short)
struct StageTranslation {
  const char* japanese;
  const char* english;
  const char* romaji;
};

#define STAGE_DISPLAY_ENGLISH 0
#define STAGE_DISPLAY_ROMAJI  1
#ifndef STAGE_DISPLAY_MODE
// 表示モードを英語/ローマ字に設定
#define STAGE_DISPLAY_MODE STAGE_DISPLAY_ROMAJI
#endif

const StageTranslation stageTranslations[] = {
  { "ユノハナ大渓谷", "Scorch Gorge",            "Yunohana" },
  { "ゴンズイ地区",   "Eeltail Alley",           "Gonzui" },
  { "ヤガラ市場",     "Hagglefish Market",       "Yagara" },
  { "マテガイ放水路", "Undertow Spillway",       "Mategai" },
  { "ナンプラー遺跡", "Um'ami Ruins",            "Nampla" },
  { "ナメロウ金属",   "Mincemeat Metalworks",    "Namero" },
  { "クサヤ温泉",     "Brinewater Springs",      "Kusaya" },
  { "タラポートショッピングパーク", "Barnacle & Dime", "Taraport" },
  { "ヒラメが丘団地", "Flounder Heights",        "Hirame" },
  { "マサバ海峡大橋", "Hammerhead Bridge",       "Masaba" },
  { "キンメダイ美術館", "Museum d'Alfonsino",    "Kinmedai" },
  { "マヒマヒリゾート＆スパ", "Mahi-Mahi Resort", "Mahimahi" },
  { "海女美術大学",   "Inkblot Art Academy",     "Amabi" },
  { "チョウザメ造船", "Sturgeon Shipyard",       "Chouzame" },
  { "ザトウマーケット", "Mako Mart",             "Zatou" },
  { "スメーシーワールド", "Wahoo World",          "Sume-shi" },
  { "コンブトラック", "Humpback Pump Track",     "Kombu" },
  { "マンタマリア号", "Manta Maria",             "Mantamaria" },
  { "タカアシ経済特区", "Crableg Capital",        "Takaashi" },
  { "オヒョウ海運",   "Shipshape Cargo Co.",     "Ohyou" },
  { "バイガイ亭",     "Bayside Bowl",            "Baigai" },
  { "ネギトロ炭鉱",   "Bluefin Depot",           "Negitoro" },
  { "カジキ空港",     "Marlin Airport",          "Kajiki" },
  { "リュウグウターミナル", "Dragon Palace Terminal", "Ryuuguu" }
};
const int stageTranslationsCount = sizeof(stageTranslations) / sizeof(StageTranslation);

// Data structures
struct BattleInfo {
  char rule[32];       // Rule name (already translated to English)
  char ruleSymbol[8];  // Rule symbol prefix
  char stage1[32];     // Stage 1 name (already translated to English)
  char stage2[32];     // Stage 2 name (already translated to English)
  char startTime[6];   // "HH:MM\0"
  char endTime[6];     // "HH:MM\0"
  bool isValid;        // Flag to indicate if data was successfully retrieved
};

// Current and next schedule for each battle type
BattleInfo regular, regularNext;
BattleInfo xMatch, xMatchNext;
BattleInfo bankaraChallenge, bankaraChallengeNext;
BattleInfo bankaraOpen, bankaraOpenNext;

char lastUpdate[16];
char currentDateTime[17];  // 現在の日時を保存する変数を追加

// Backlight control function
void setBacklight(uint8_t brightness) {
  ledcWrite(TFT_BL_PIN, brightness);
}

// Helper function to translate rule names from Japanese to selected display and get the symbol
void translateRuleInfo(const char* japaneseRule, char* outRule, char* ruleSymbol) {
  for (int i = 0; i < ruleTranslationsCount; i++) {
    if (strcmp(japaneseRule, ruleTranslations[i].japanese) == 0) {
      #if RULE_DISPLAY_MODE == RULE_DISPLAY_ROMAJI
        strcpy(outRule, ruleTranslations[i].romaji);
      #else
        strcpy(outRule, ruleTranslations[i].english);
      #endif
      strcpy(ruleSymbol, ruleTranslations[i].symbol);
      return;
    }
  }
  // Return original if no translation found and empty symbol
  strcpy(outRule, japaneseRule);
  ruleSymbol[0] = '\0';
}

// Helper function to translate stage names from Japanese to selected display
const char* translateStage(const char* japaneseStage) {
  for (int i = 0; i < stageTranslationsCount; i++) {
    if (strcmp(japaneseStage, stageTranslations[i].japanese) == 0) {
      #if STAGE_DISPLAY_MODE == STAGE_DISPLAY_ROMAJI
        return stageTranslations[i].romaji;
      #else
        return stageTranslations[i].english;
      #endif
    }
  }
  return japaneseStage;  // Return original if no translation found
}

// Helper function to translate battle type names
const char* translateBattleType(const char* battleType) {
  // Hardcoded mapping based on URL pattern to fixed battle types
  if (strstr(battleType, "regular") || strcmp(battleType, "Regular Battle") == 0 || strcmp(battleType, "Turf War") == 0) {
    #if BATTLETYPE_DISPLAY_MODE == BATTLETYPE_DISPLAY_ROMAJI
      return "Regular Match";
    #else
      return "Regular Battle";
    #endif
  } 
  else if (strstr(battleType, "x") || strcmp(battleType, "X Battle") == 0) {
    #if BATTLETYPE_DISPLAY_MODE == BATTLETYPE_DISPLAY_ROMAJI
      return "X Match";
    #else
      return "X Battle";
    #endif
  }
  else if (strstr(battleType, "bankara-challenge") || strcmp(battleType, "Anarchy Challenge") == 0) {
    #if BATTLETYPE_DISPLAY_MODE == BATTLETYPE_DISPLAY_ROMAJI
      return "Bankara Challenge";
    #else
      return "Anarchy Challenge";
    #endif
  }
  else if (strstr(battleType, "bankara-open") || strcmp(battleType, "Anarchy Open") == 0) {
    #if BATTLETYPE_DISPLAY_MODE == BATTLETYPE_DISPLAY_ROMAJI
      return "Bankara Open";
    #else
      return "Anarchy Open";
    #endif
  }
  
  // If we got here, it's an unrecognized type
  return battleType;
}

// Generic function to update battle data
void updateBattleData(const char* url, BattleInfo* battleInfo) {
  if (WiFi.status() != WL_CONNECTED) {
    battleInfo->isValid = false;
    return;
  }

  HTTPClient http;
  http.begin(url);

  int httpCode = http.GET();

  if (httpCode == HTTP_CODE_OK) {
    StaticJsonDocument<1024> doc;  // Increased buffer size
    DeserializationError error = deserializeJson(doc, http.getStream());

    if (error) {
      Serial.print("JSON parse error: ");
      Serial.println(error.c_str());
      strcpy(battleInfo->rule, "Error");
      battleInfo->ruleSymbol[0] = '\0';  // Empty symbol
      battleInfo->isValid = false;
      http.end();
      return;
    }

    // For regular match, hardcode the rule but with empty display text
    if (strstr(url, "regular")) {
      strcpy(battleInfo->rule, "");        // ナワバリの場合はルール名を空にする
      strcpy(battleInfo->ruleSymbol, "");  // シンボルも空に
    } else {
      // For other match types, get the rule and translate it with symbol
      const char* japaneseRule = doc["results"][0]["rule"]["name"];
      if (japaneseRule) {
        char englishRule[32];
        translateRuleInfo(japaneseRule, englishRule, battleInfo->ruleSymbol);
        strncpy(battleInfo->rule, englishRule, sizeof(battleInfo->rule) - 1);
      } else {
        strcpy(battleInfo->rule, "Unknown");
        battleInfo->ruleSymbol[0] = '\0';  // Empty symbol
      }
    }

    battleInfo->rule[sizeof(battleInfo->rule) - 1] = '\0';              // Ensure null termination
    battleInfo->ruleSymbol[sizeof(battleInfo->ruleSymbol) - 1] = '\0';  // Ensure null termination

    // Stage 1 - translate from Japanese to English
    const char* japaneseStage1 = doc["results"][0]["stages"][0]["name"];
    if (japaneseStage1) {
      const char* englishStage1 = translateStage(japaneseStage1);
      strncpy(battleInfo->stage1, englishStage1, sizeof(battleInfo->stage1) - 1);
      battleInfo->stage1[sizeof(battleInfo->stage1) - 1] = '\0';  // Ensure null termination
    } else {
      strcpy(battleInfo->stage1, "Unknown");
    }

    // Stage 2 - translate from Japanese to English
    const char* japaneseStage2 = doc["results"][0]["stages"][1]["name"];
    if (japaneseStage2) {
      const char* englishStage2 = translateStage(japaneseStage2);
      strncpy(battleInfo->stage2, englishStage2, sizeof(battleInfo->stage2) - 1);
      battleInfo->stage2[sizeof(battleInfo->stage2) - 1] = '\0';  // Ensure null termination
    } else {
      strcpy(battleInfo->stage2, "Unknown");
    }

    // Time
    const char* st = doc["results"][0]["start_time"];
    if (st && strlen(st) > 16) {
      strncpy(battleInfo->startTime, st + 11, 5);
      battleInfo->startTime[5] = '\0';
    } else {
      strcpy(battleInfo->startTime, "--:--");
    }

    const char* et = doc["results"][0]["end_time"];
    if (et && strlen(et) > 16) {
      strncpy(battleInfo->endTime, et + 11, 5);
      battleInfo->endTime[5] = '\0';
    } else {
      strcpy(battleInfo->endTime, "--:--");
    }

    // Debug output
    Serial.print("Rule: ");
    Serial.print(battleInfo->ruleSymbol);
    Serial.println(battleInfo->rule);
    Serial.print("Stage 1: ");
    Serial.println(battleInfo->stage1);
    Serial.print("Stage 2: ");
    Serial.println(battleInfo->stage2);
    Serial.print("Time: ");
    Serial.print(battleInfo->startTime);
    Serial.print(" - ");
    Serial.println(battleInfo->endTime);

    battleInfo->isValid = true;
  } else {
    Serial.print("HTTP Error: ");
    Serial.println(httpCode);
    strcpy(battleInfo->rule, "Error");
    battleInfo->ruleSymbol[0] = '\0';  // Empty symbol
    battleInfo->isValid = false;
  }

  http.end();
}

// Update all battle data
void updateAllBattleData() {
  Serial.println("Updating Regular Battle data...");
  updateBattleData(apiUrlRegular, &regular);
  delay(200);
  updateBattleData(apiUrlRegularNext, &regularNext);
  delay(200);

  Serial.println("Updating X Match data...");
  updateBattleData(apiUrlXMatch, &xMatch);
  delay(200);
  updateBattleData(apiUrlXMatchNext, &xMatchNext);
  delay(200);

  Serial.println("Updating Bankara Challenge data...");
  updateBattleData(apiUrlBankaraChallenge, &bankaraChallenge);
  delay(200);
  updateBattleData(apiUrlBankaraChallengeNext, &bankaraChallengeNext);
  delay(200);

  Serial.println("Updating Bankara Open data...");
  updateBattleData(apiUrlBankaraOpen, &bankaraOpen);
  delay(200);
  updateBattleData(apiUrlBankaraOpenNext, &bankaraOpenNext);
  delay(200);

  // Update the last update time
  time_t now;
  struct tm timeinfo;
  if (getLocalTime(&timeinfo)) {
    snprintf(lastUpdate, sizeof(lastUpdate), "%02d/%02d %02d:%02d",
             timeinfo.tm_mon + 1, timeinfo.tm_mday, timeinfo.tm_hour, timeinfo.tm_min);
  }
}

// 現在の日時を更新する関数
void updateCurrentDateTime() {
  struct tm timeinfo;
  if (getLocalTime(&timeinfo)) {
    snprintf(currentDateTime, sizeof(currentDateTime), "%04d/%02d/%02d %02d:%02d",
             timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday,
             timeinfo.tm_hour, timeinfo.tm_min);
  } else {
    strcpy(currentDateTime, "Time Error");
  }
}

// Draw a battle quadrant (update with proper display names)
void drawBattleQuadrant(int x, int y, const char* title, const BattleInfo* current, const BattleInfo* next, uint16_t titleColor) {
  // Title background with specific color for each battle type
  tft.fillRect(x, y, QUADRANT_WIDTH, 16, titleColor);

  // Title (通常サイズのテキスト)
  tft.setTextColor(TFT_BLACK);  // Black text on colored background for better contrast
  tft.setTextSize(1);           // 通常のサイズに戻す
  tft.setCursor(x + 4, y + 4);  // 位置調整
  tft.println(title);

  if (!current->isValid) {
    tft.setTextColor(TFT_RED);
    tft.setCursor(x + 4, y + 20);
    tft.println("Data Error");
    return;
  }

  // 通常のテキスト
  tft.setTextColor(TFT_WHITE);

  // 順序変更: 期間、ルール、ステージの順
  int yPos = 20;  // 開始位置

  // 1. 期間
  tft.setCursor(x + 4, y + yPos);
  tft.print(current->startTime);
  tft.print("-");
  tft.println(current->endTime);
  yPos += 10;

  // 2. ルール (シンボル付き) - ナワバリの場合は空なので表示しない
  if (strlen(current->rule) > 0 || strlen(current->ruleSymbol) > 0) {
    // ルールシンボルに応じた色を設定
    uint16_t symbolColor = TFT_WHITE;  // デフォルト色

    // ルールシンボルに基づいて色を決定
    if (strcmp(current->ruleSymbol, "[-] ") == 0) {
      symbolColor = COLOR_RULE_AREA;  // ガチエリア
    } else if (strcmp(current->ruleSymbol, "|^| ") == 0) {
      symbolColor = COLOR_RULE_YAGURA;  // ガチヤグラ
    } else if (strcmp(current->ruleSymbol, "{*} ") == 0) {
      symbolColor = COLOR_RULE_HOKO;  // ガチホコバトル
    } else if (strcmp(current->ruleSymbol, "(+) ") == 0) {
      symbolColor = COLOR_RULE_ASARI;  // ガチアサリ
    }

    // シンボルを色付きで表示
    tft.setCursor(x + 4, y + yPos);
    tft.setTextColor(symbolColor);
    tft.print(current->ruleSymbol);

    // ルール名は白色で表示
    tft.setTextColor(TFT_WHITE);
    tft.println(current->rule);
    yPos += 10;
  }

  // 3. ステージ名
  tft.setTextColor(TFT_LIGHTGREY);
  tft.setCursor(x + 4, y + yPos);
  tft.print("- ");

  // Print first 13 chars of stage1 name
  char shortenedStage[14];  // 13 chars + null terminator
  strncpy(shortenedStage, current->stage1, 13);
  shortenedStage[13] = '\0';
  tft.println(shortenedStage);
  yPos += 10;

  tft.setCursor(x + 4, y + yPos);
  tft.print("- ");

  // Print first 13 chars of stage2 name
  strncpy(shortenedStage, current->stage2, 13);
  shortenedStage[13] = '\0';
  tft.println(shortenedStage);
  yPos += 15;  // 次のセクションまでの間隔を少し広く

  // Next battle - "Next"文字は削除して、行間を詰める
  if (next->isValid) {
    tft.setTextColor(TFT_WHITE);

    // 1. 期間
    tft.setCursor(x + 4, y + yPos);
    tft.print(next->startTime);
    tft.print("-");
    tft.println(next->endTime);
    yPos += 10;

    // 2. ルール (シンボル付き) - ナワバリの場合は空なので表示しない
    if (strlen(next->rule) > 0 || strlen(next->ruleSymbol) > 0) {
      // ルールシンボルに応じた色を設定
      uint16_t symbolColor = TFT_WHITE;  // デフォルト色

      // ルールシンボルに基づいて色を決定
      if (strcmp(next->ruleSymbol, "[-] ") == 0) {
        symbolColor = COLOR_RULE_AREA;  // ガチエリア
      } else if (strcmp(next->ruleSymbol, "|^| ") == 0) {
        symbolColor = COLOR_RULE_YAGURA;  // ガチヤグラ
      } else if (strcmp(next->ruleSymbol, "{*} ") == 0) {
        symbolColor = COLOR_RULE_HOKO;  // ガチホコバトル
      } else if (strcmp(next->ruleSymbol, "(+) ") == 0) {
        symbolColor = COLOR_RULE_ASARI;  // ガチアサリ
      }

      // シンボルを色付きで表示
      tft.setCursor(x + 4, y + yPos);
      tft.setTextColor(symbolColor);
      tft.print(next->ruleSymbol);

      // ルール名は白色で表示
      tft.setTextColor(TFT_WHITE);
      tft.println(next->rule);
      yPos += 10;
    }

    // 3. ステージ名
    tft.setTextColor(TFT_LIGHTGREY);
    tft.setCursor(x + 4, y + yPos);
    tft.print("- ");

    // Print first 13 chars of next stage1 name
    strncpy(shortenedStage, next->stage1, 13);
    shortenedStage[13] = '\0';
    tft.println(shortenedStage);
    yPos += 10;

    tft.setCursor(x + 4, y + yPos);
    tft.print("- ");

    // Print first 13 chars of next stage2 name
    strncpy(shortenedStage, next->stage2, 13);
    shortenedStage[13] = '\0';
    tft.println(shortenedStage);
  }
}

// 下部情報（日時と最終更新時刻）を描画する関数を追加
void drawBottomInfo() {
  // 画面下部全体をクリア
  tft.fillRect(0, SCREEN_HEIGHT - 12, SCREEN_WIDTH, 12, TFT_BLACK);

  // 現在の日時を左下に表示
  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(1);
  tft.setCursor(4, SCREEN_HEIGHT - 12);
  tft.print(currentDateTime);

  // 最終更新時刻を右下に表示（余白を増やす）
  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(1);
  int textWidth = tft.textWidth(lastUpdate) + tft.textWidth("Updated: ");
  tft.setCursor(SCREEN_WIDTH - textWidth - 10, SCREEN_HEIGHT - 12);  // 余白をさらに増やす
  tft.print("Updated: ");
  tft.print(lastUpdate);

  // 必要な場合は中心線と交差する部分を再描画
  tft.drawLine(QUADRANT_WIDTH, SCREEN_HEIGHT - 12, QUADRANT_WIDTH, SCREEN_HEIGHT, TFT_WHITE);
}

// Update the display with all battle data
void updateDisplay() {
  tft.fillScreen(TFT_BLACK);

  // 先に4つの象限を描画 (バトル種別の表示方法を切り替え)
  drawBattleQuadrant(0, 0, translateBattleType("Regular Battle"), &regular, &regularNext, COLOR_REGULAR);
  drawBattleQuadrant(QUADRANT_WIDTH, 0, translateBattleType("X Battle"), &xMatch, &xMatchNext, COLOR_XMATCH);
  drawBattleQuadrant(0, QUADRANT_HEIGHT, translateBattleType("Anarchy Challenge"), &bankaraChallenge, &bankaraChallengeNext, COLOR_BANKARA_CHALLENGE);
  drawBattleQuadrant(QUADRANT_WIDTH, QUADRANT_HEIGHT, translateBattleType("Anarchy Open"), &bankaraOpen, &bankaraOpenNext, COLOR_BANKARA_OPEN);

  // 分割線を描画（太さと色を強調）
  tft.drawLine(QUADRANT_WIDTH, 0, QUADRANT_WIDTH, SCREEN_HEIGHT, TFT_WHITE);
  tft.drawLine(0, QUADRANT_HEIGHT, SCREEN_WIDTH, QUADRANT_HEIGHT, TFT_WHITE);

  // 下部情報（日時と最終更新時刻）を描画
  drawBottomInfo();
}

// Fetch all data and update the display
void updateAllData() {
  updateAllBattleData();
  updateCurrentDateTime();
  updateDisplay();
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("\n\nStarting...");
  Serial.println("ESP32-2432S028R Splatoon3 Schedule Display");

  // Backlight PWM setup
  ledcAttach(TFT_BL_PIN, 5000, 8);

  // TFT initialization
  tft.init();
  tft.setRotation(1);  // Landscape orientation

  // Gradually increase backlight brightness
  for (int i = 0; i < 255; i += 5) {
    setBacklight(i);
    delay(10);
  }

  // Initial screen display
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_GREEN);
  tft.setTextSize(2);
  tft.setCursor(40, 100);
  tft.println("Splatoon3");
  tft.setCursor(40, 140);
  tft.println("Schedule");
  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(1);
  tft.setCursor(70, 180);
  tft.println("Connecting...");

  // WiFi connection
  Serial.print("Connecting to WiFi...");
  WiFi.begin(ssid, password);
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi Connected");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
    tft.setTextColor(TFT_GREEN);
    tft.setCursor(70, 180);
    tft.println("Connected!  ");
  } else {
    Serial.println("\nWiFi Connection Failed");
    tft.setTextColor(TFT_RED);
    tft.setCursor(70, 180);
    tft.println("Failed!     ");
    delay(2000);
  }

  // NTP setup (for time retrieval)
  configTime(9 * 3600, 0, "ntp.nict.jp", "ntp.jst.mfeed.ad.jp");

  delay(1000);

  // Initial data fetch and display
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(1);
  tft.setCursor(50, 150);
  tft.println("Fetching data...");
  updateAllData();

  Serial.println("Setup complete");
}

void loop() {
  // 現在時刻
  static unsigned long lastStatusTime = 0;
  static unsigned long lastUpdateTime = 0;
  static unsigned long lastClockTime = 0;
  unsigned long currentTime = millis();

  // シリアル出力（10秒ごと）
  if (currentTime - lastStatusTime > 10000) {
    Serial.println("Running...");
    lastStatusTime = currentTime;
  }

  // 自動データ更新（5分ごと）
  if (currentTime - lastUpdateTime > 5 * 60 * 1000) {
    Serial.println("Performing scheduled update");
    updateAllData();
    lastUpdateTime = currentTime;
  }

  // 時計の更新（1分ごと）
  if (currentTime - lastClockTime > 60 * 1000) {
    updateCurrentDateTime();

    // 下部情報のみを更新（新しい関数を使用）
    drawBottomInfo();

    lastClockTime = currentTime;
  }

  delay(100);
}
