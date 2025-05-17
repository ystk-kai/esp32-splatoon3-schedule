#include "ESP32NetworkService.h"
#include <time.h>

namespace Infrastructure
{

    bool ESP32NetworkService::connect(const char *ssid, const char *password)
    {
        WiFi.begin(ssid, password);

        // Wait for connection with timeout
        int attempts = 0;
        while (WiFi.status() != WL_CONNECTED && attempts < 20)
        {
            delay(500);
            Serial.print(".");
            attempts++;
        }

        if (WiFi.status() == WL_CONNECTED)
        {
            Serial.println("\nWiFi Connected");
            Serial.print("IP Address: ");
            Serial.println(WiFi.localIP());
            return true;
        }
        else
        {
            Serial.println("\nWiFi Connection Failed");
            return false;
        }
    }

    bool ESP32NetworkService::isConnected()
    {
        return WiFi.status() == WL_CONNECTED;
    }

    String ESP32NetworkService::getIPAddress()
    {
        return WiFi.localIP().toString();
    }

    String ESP32NetworkService::httpGet(const char *url)
    {
        if (!isConnected())
        {
            return "";
        }

        HTTPClient http;
        http.begin(url);

        int httpCode = http.GET();

        if (httpCode == HTTP_CODE_OK)
        {
            String payload = http.getString();
            http.end();
            return payload;
        }
        else
        {
            Serial.print("HTTP Error: ");
            Serial.println(httpCode);
            http.end();
            return "";
        }
    }

    void ESP32NetworkService::configureTimeService()
    {
        // Configure time service with Japan timezone (UTC+9)
        configTime(9 * 3600, 0, "ntp.nict.jp", "ntp.jst.mfeed.ad.jp");

        // Wait for time to be set
        delay(1000);
    }

    bool ESP32NetworkService::getLocalTime(struct tm &timeinfo)
    {
        time_t now;
        time(&now);
        return localtime_r(&now, &timeinfo) != nullptr;
    }

    bool ESP32NetworkService::getCurrentDateTime(char *buffer, size_t bufferSize)
    {
        struct tm timeinfo;

        if (getLocalTime(timeinfo))
        {
            snprintf(buffer, bufferSize, "%04d/%02d/%02d %02d:%02d",
                     timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday,
                     timeinfo.tm_hour, timeinfo.tm_min);
            return true;
        }
        else
        {
            strncpy(buffer, "Time Error", bufferSize - 1);
            buffer[bufferSize - 1] = '\0';
            return false;
        }
    }

    bool ESP32NetworkService::getLastUpdateTime(char *buffer, size_t bufferSize)
    {
        struct tm timeinfo;

        if (getLocalTime(timeinfo))
        {
            snprintf(buffer, bufferSize, "%02d/%02d %02d:%02d",
                     timeinfo.tm_mon + 1, timeinfo.tm_mday, timeinfo.tm_hour, timeinfo.tm_min);
            return true;
        }
        else
        {
            strncpy(buffer, "Error", bufferSize - 1);
            buffer[bufferSize - 1] = '\0';
            return false;
        }
    }

} // namespace Infrastructure