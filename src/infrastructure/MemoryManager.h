// MemoryManager.h
// メモリ管理と監視のためのユーティリティ

#ifndef MEMORY_MANAGER_H
#define MEMORY_MANAGER_H

#include <Arduino.h>

namespace Infrastructure
{
    class MemoryManager
    {
    public:
        // メモリ使用量をログ出力
        static void logMemoryUsage(const char *operation);

        // メモリ不足の警告をチェック
        static bool isLowMemory();

        // メモリ使用量の詳細情報を取得
        static void getMemoryInfo(size_t &freeHeap, size_t &minFreeHeap, size_t &maxAllocHeap);

        // メモリの強制ガベージコレクション（可能な場合）
        static void forceGarbageCollection();

        // メモリ使用量の統計情報をリセット
        static void resetMemoryStats();

        // 定期的なメモリクリーンアップ
        static void performPeriodicCleanup();

        // メモリ使用量の傾向を分析
        static void analyzeMemoryTrend();

    private:
        static size_t lastFreeHeap;
        static size_t minFreeHeapEver;
        static unsigned long lastMemoryCheck;
        static unsigned long lastCleanupTime;
        static size_t memoryLeakThreshold;
        static int consecutiveLowMemoryCount;
    };
}

#endif // MEMORY_MANAGER_H