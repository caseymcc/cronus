#ifndef _cronus_startupNotifier_h_
#define _cronus_startupNotifier_h_

#include <string>

namespace cronus
{

/**
 * @brief Sends a startup notification to notify clients that the server is ready
 */
class StartupNotifier
{
public:
    /**
     * @brief Send a startup notification via UDP
     * @param notificationHost Host to send notification to (usually localhost)
     * @param notificationPort Port to send notification to (default: 8999)
     * @param serverUrl URL where the server is accessible (e.g., http://localhost:9000)
     * @return true if notification was sent successfully, false otherwise
     */
    static bool notify(const std::string& notificationHost, int notificationPort, const std::string& serverUrl);

private:
    StartupNotifier() = default;
};

} // namespace cronus

#endif//_cronus_startupNotifier_h_
