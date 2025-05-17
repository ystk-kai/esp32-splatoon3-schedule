// ESP32NetworkService.h
// ESP32 WiFi and HTTP implementation of NetworkService

#ifndef ESP32_NETWORK_SERVICE_H
#define ESP32_NETWORK_SERVICE_H

#include <WiFi.h>
#include <HTTPClient.h>
#include "../application/NetworkService.h"

namespace Infrastructure
{

    // ESP32 WiFi implementation of NetworkService
    class ESP32NetworkService : public Application::NetworkService
    {
    public:
        // Connect to WiFi network
        bool connect(const char *ssid, const char *password) override;

        // Check if currently connected to WiFi
        bool isConnected() override;

        // Get current IP address as string
        String getIPAddress() override;

        // Perform HTTP GET request and return response body
        String httpGet(const char *url) override;

        // Configure time service (NTP)
        void configureTimeService() override;

        // Get current local time formatted as YYYY/MM/DD HH:MM
        bool getCurrentDateTime(char *buffer, size_t bufferSize) override;

        // Get current local time formatted as MM/DD HH:MM
        bool getLastUpdateTime(char *buffer, size_t bufferSize) override;

    private:
        // Helper method to get local time struct
        bool getLocalTime(struct tm &timeinfo);
    };

} // namespace Infrastructure

#endif // ESP32_NETWORK_SERVICE_H