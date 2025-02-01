#include "cronus/logger.h"
#include <iostream>
#include <iomanip>
#include <ctime>

namespace cronus
{

Logger& Logger::instance()
{
    static Logger instance;
    return instance;
}

Logger::Logger()
{
    // Set default callback to write to stderr
    m_callback = [](LogLevel level, const std::string& message) {
        auto now = std::time(nullptr);
        auto tm = *std::localtime(&now);
        
        std::string levelStr;
        switch(level) {
            case LogLevel::DEBUG: levelStr = "DEBUG"; break;
            case LogLevel::INFO: levelStr = "INFO"; break;
            case LogLevel::WARNING: levelStr = "WARNING"; break;
            case LogLevel::ERROR: levelStr = "ERROR"; break;
        }

        std::cerr << std::put_time(&tm, "%Y-%m-%d %H:%M:%S") 
                  << " [" << levelStr << "] " 
                  << message << std::endl;
    };
}

void Logger::setCallback(LogCallback callback)
{
    m_callback = std::move(callback);
}

void Logger::resetCallback()
{
    Logger fresh;
    m_callback = fresh.m_callback;
}

void Logger::log(LogLevel level, const std::string& message)
{
    if (m_callback) {
        m_callback(level, message);
    }
}

void Logger::debug(const std::string& message)
{
    log(LogLevel::DEBUG, message);
}

void Logger::info(const std::string& message)
{
    log(LogLevel::INFO, message);
}

void Logger::warning(const std::string& message)
{
    log(LogLevel::WARNING, message);
}

void Logger::error(const std::string& message)
{
    log(LogLevel::ERROR, message);
}

} // namespace cronus
