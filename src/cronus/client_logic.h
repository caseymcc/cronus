#ifndef _cronus_client_logic_h_
#define _cronus_client_logic_h_

#include "llm_hermes/hermes.h"
#include "cronus/terminal_ui.h"
#include <filesystem>

namespace cronus
{

class ClientLogic
{
public:
    using LogCallback = std::function<void(const std::string&)>;
    using ResponseCallback = std::function<void(const std::string&, const std::string&)>;
    using ErrorCallback = std::function<void(const std::string&)>;

    explicit ClientLogic();
    ~ClientLogic();
    
    std::future<int> processInput(const std::string &input);
    std::vector<std::pair<bool, std::string>> getCurrentDirectoryContents() const;

    // Callback setters
    void setLogCallback(LogCallback callback) { m_logCallback = callback; }
    void setResponseCallback(ResponseCallback callback) { m_responseCallback = callback; }
    void setErrorCallback(ErrorCallback callback) { m_errorCallback = callback; }

private:
    int processCompletion(const std::string &input);
    void log(const std::string& message) const;
    void handleResponse(const std::string& provider, const std::string& response) const;
    void handleError(const std::string& error) const;

    std::filesystem::path m_currentPath;
    std::unique_ptr<TaskSystem> m_taskSystem;

    // Callbacks
    LogCallback m_logCallback;
    ResponseCallback m_responseCallback;
    ErrorCallback m_errorCallback;
};

} // namespace cronus

#endif//_cronus_client_logic_h_
