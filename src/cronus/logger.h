#ifndef _cronus_logger_h_
#define _cronus_logger_h_

#include <functional>
#include <string>
#include <memory>

namespace cronus
{

enum class LogLevel {
    DEBUG,
    INFO,
    WARNING,
    ERROR
};

class Logger {
public:
    using LogCallback = std::function<void(LogLevel, const std::string&)>;

    static Logger& instance();

    void setCallback(LogCallback callback);
    void resetCallback();

    void debug(const std::string& message);
    void info(const std::string& message);
    void warning(const std::string& message);
    void error(const std::string& message);

private:
    Logger();
    void log(LogLevel level, const std::string& message);

    LogCallback m_callback;
};

// Convenience functions
inline void logDebug(const std::string& message) { Logger::instance().debug(message); }
inline void logInfo(const std::string& message) { Logger::instance().info(message); }
inline void logWarning(const std::string& message) { Logger::instance().warning(message); }
inline void logError(const std::string& message) { Logger::instance().error(message); }

} // namespace cronus

#endif//_cronus_logger_h_
