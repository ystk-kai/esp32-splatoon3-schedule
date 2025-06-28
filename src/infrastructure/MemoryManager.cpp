// MemoryManager.cpp
// メモリ管理と監視のためのユーティリティ実装

#include "MemoryManager.h"

namespace Infrastructure
{
    // 静的変数の初期化
    size_t MemoryManager::lastFreeHeap = 0;
    size_t MemoryManager::minFreeHeapEver = SIZE_MAX;
    unsigned long MemoryManager::lastMemoryCheck = 0;
    unsigned long MemoryManager::lastCleanupTime = 0;
    size_t MemoryManager::memoryLeakThreshold = 5000; // 5KB
    int MemoryManager::consecutiveLowMemoryCount = 0;

    void MemoryManager::logMemoryUsage(const char *operation)
    {
        size_t freeHeap, minFreeHeap, maxAllocHeap;
        getMemoryInfo(freeHeap, minFreeHeap, maxAllocHeap);

        Serial.print("Memory [");
        Serial.print(operation);
        Serial.print("] - Free: ");
        Serial.print(freeHeap);
        Serial.print(" bytes, Min Free: ");
        Serial.print(minFreeHeap);
        Serial.print(" bytes, Max Alloc: ");
        Serial.print(maxAllocHeap);
        Serial.println(" bytes");

        // メモリ使用量の変化を記録
        if (lastFreeHeap > 0)
        {
            int change = (int)freeHeap - (int)lastFreeHeap;
            if (change != 0)
            {
                Serial.print("Memory change: ");
                Serial.print(change > 0 ? "+" : "");
                Serial.print(change);
                Serial.println(" bytes");

                // メモリリークの検出
                if (change < -(int)memoryLeakThreshold)
                {
                    Serial.println("WARNING: Potential memory leak detected!");
                }
            }
        }

        lastFreeHeap = freeHeap;
        lastMemoryCheck = millis();

        // メモリ不足の警告
        if (isLowMemory())
        {
            consecutiveLowMemoryCount++;
            Serial.print("WARNING: Low memory detected! (Count: ");
            Serial.print(consecutiveLowMemoryCount);
            Serial.println(")");

            if (consecutiveLowMemoryCount >= 3)
            {
                Serial.println("CRITICAL: Persistent low memory - consider restart");
            }
        }
        else
        {
            consecutiveLowMemoryCount = 0;
        }
    }

    bool MemoryManager::isLowMemory()
    {
        size_t freeHeap = ESP.getFreeHeap();
        return freeHeap < 10000; // 10KB未満の場合
    }

    void MemoryManager::getMemoryInfo(size_t &freeHeap, size_t &minFreeHeap, size_t &maxAllocHeap)
    {
        freeHeap = ESP.getFreeHeap();
        minFreeHeap = ESP.getMinFreeHeap();
        maxAllocHeap = ESP.getMaxAllocHeap();

        // 最小メモリ使用量を更新
        if (freeHeap < minFreeHeapEver)
        {
            minFreeHeapEver = freeHeap;
        }
    }

    void MemoryManager::forceGarbageCollection()
    {
        // ESP32では明示的なガベージコレクションはありませんが、
        // メモリ使用量をチェックして警告を出す
        if (isLowMemory())
        {
            Serial.println("Low memory detected - consider restarting if problems occur");
        }

        // メモリ使用量をログ
        logMemoryUsage("After GC attempt");
    }

    void MemoryManager::resetMemoryStats()
    {
        lastFreeHeap = 0;
        minFreeHeapEver = SIZE_MAX;
        lastMemoryCheck = 0;
        lastCleanupTime = 0;
        consecutiveLowMemoryCount = 0;
        Serial.println("Memory statistics reset");
    }

    void MemoryManager::performPeriodicCleanup()
    {
        unsigned long currentTime = millis();

        // 5分ごとにクリーンアップを実行
        if (currentTime - lastCleanupTime >= 300000) // 5分
        {
            Serial.println("Performing periodic memory cleanup...");

            // メモリ使用量をチェック
            size_t freeHeap = ESP.getFreeHeap();

            // メモリ使用量が少ない場合は警告
            if (freeHeap < 15000) // 15KB未満
            {
                Serial.println("WARNING: Low memory during cleanup");
            }

            // メモリ使用量の詳細をログ
            logMemoryUsage("Periodic cleanup");

            lastCleanupTime = currentTime;
        }
    }

    void MemoryManager::analyzeMemoryTrend()
    {
        // メモリ使用量の傾向を分析
        if (lastFreeHeap > 0)
        {
            size_t currentFreeHeap = ESP.getFreeHeap();
            size_t totalMemoryLoss = minFreeHeapEver - currentFreeHeap;

            if (totalMemoryLoss > 50000) // 50KB以上のメモリ損失
            {
                Serial.print("Memory trend analysis: Total memory loss: ");
                Serial.print(totalMemoryLoss);
                Serial.println(" bytes");
                Serial.println("Consider implementing memory optimization");
            }
        }
    }
}

// namespace Infrastructure