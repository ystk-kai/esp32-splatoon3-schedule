// NetworkService.h
// Service interface for network operations

#ifndef NETWORK_SERVICE_H
#define NETWORK_SERVICE_H

#include <Arduino.h>

namespace Application
{

    // Service interface for network operations
    class NetworkService
    {
    public:
        virtual ~NetworkService() = default;

        // Connect to WiFi network
        virtual bool connect(const char *ssid, const char *password) = 0;

        // Check if currently connected to WiFi
        virtual bool isConnected() = 0;

        // Get current IP address as string
        virtual String getIPAddress() = 0;

        // Perform HTTP GET request and return response body
        virtual String httpGet(const char *url) = 0;

        // Configure time service (NTP)
        virtual void configureTimeService() = 0;

        // Get current local time formatted as YYYY/MM/DD HH:MM
        virtual bool getCurrentDateTime(char *buffer, size_t bufferSize) = 0;

        // Get current local time formatted as MM/DD HH:MM
        virtual bool getLastUpdateTime(char *buffer, size_t bufferSize) = 0;
    };

} // namespace Application

#endif // NETWORK_SERVICE_H