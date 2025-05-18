// AppInitializationService.h
// アプリケーション初期化サービスのインターフェース

#ifndef APP_INITIALIZATION_SERVICE_H
#define APP_INITIALIZATION_SERVICE_H

#include <Arduino.h>

namespace Application
{
    // アプリケーション初期化サービスのインターフェース
    class AppInitializationService
    {
    public:
        virtual ~AppInitializationService() = default;

        // アプリケーションの初期化を試みる
        virtual bool tryInitializeApp() = 0;

        // セットアップフェーズの処理
        virtual void performSetup() = 0;

        // 定期実行されるロジック
        virtual void performLoop(unsigned long currentMillis) = 0;
    };
}

#endif // APP_INITIALIZATION_SERVICE_H