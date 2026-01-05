#include "cronus/startupNotifier.h"
#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <nlohmann/json.hpp>

namespace cronus
{

bool StartupNotifier::notify(const std::string& notificationHost, int notificationPort, const std::string& serverUrl)
{
    try
    {
        // Create UDP socket
        int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
        if (sockfd < 0)
        {
            std::cerr << "Failed to create UDP socket for startup notification" << std::endl;
            return false;
        }

        // Prepare the notification message as JSON
        nlohmann::json notification;
        notification["type"] = "server_startup";
        notification["serverUrl"] = serverUrl;
        notification["timestamp"] = std::time(nullptr);
        
        std::string message = notification.dump();

        // Setup server address
        struct sockaddr_in serverAddr;
        memset(&serverAddr, 0, sizeof(serverAddr));
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(notificationPort);
        
        // Convert hostname to IP address
        if (inet_pton(AF_INET, notificationHost.c_str(), &serverAddr.sin_addr) <= 0)
        {
            // If not a valid IP, try localhost
            if (notificationHost == "localhost" || notificationHost == "127.0.0.1")
            {
                serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
            }
            else
            {
                std::cerr << "Invalid notification host address: " << notificationHost << std::endl;
                close(sockfd);
                return false;
            }
        }

        // Send the notification
        ssize_t sentBytes = sendto(sockfd, message.c_str(), message.length(), 0,
                                   (struct sockaddr*)&serverAddr, sizeof(serverAddr));

        close(sockfd);

        if (sentBytes < 0)
        {
            std::cerr << "Failed to send startup notification" << std::endl;
            return false;
        }

        std::cout << "Startup notification sent to " << notificationHost << ":" 
                  << notificationPort << " (server: " << serverUrl << ")" << std::endl;
        return true;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Exception while sending startup notification: " << e.what() << std::endl;
        return false;
    }
}

} // namespace cronus
