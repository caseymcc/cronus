#ifndef _cronus_terminal_ui_h_
#define _cronus_terminal_ui_h_

#include "cronus/cronus.h"

#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/screen.hpp>
#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <httplib.h>
#include <nlohmann/json.hpp>

#include <string>
#include <vector>
#include <thread>
#include <atomic>
#include <mutex>

namespace cronus
{

enum class ChatType
{
    Message,
    Completion,
    Log
};

enum class Role
{
    User,
    Bot,
    Error,
    Warning,
    Log
};

struct ChatMessage
{
    ChatMessage(ChatType type, Role role, const std::string &content) :
        type(type),
        role(role),
        content(content)
    {
    }

    ChatType type;
    Role role;

    std::string content;
};

class TerminalUI
{
public:
    TerminalUI();
    explicit TerminalUI(class Cronus &cronus);
    ~TerminalUI();

    void updateDirectoryTree(const std::vector<std::pair<bool, std::string>> &contents);

    void run();

private:
    void addResponse(const std::string &provider, const std::string &response);
    void addLog(LogLevel logLevel, const std::string &message);

    void handleInput(ftxui::Event event);

    // Helper methods
    void initializeDirTree();
    void setupUI();
    void fetchDirectoryContents();
    void sendInputToApi(const std::string& input);
    void startResponsePolling();
    void stopResponsePolling();

    ftxui::Element renderDirTree();
    ftxui::Element renderDebugArea();
    ftxui::Element renderChatArea();
    ftxui::Element renderInputArea();
    ftxui::Element renderMainLayout();

    Cronus &m_cronus;
    ftxui::ScreenInteractive m_screen;
    std::string m_apiBaseUrl;
    std::unique_ptr<httplib::Client> m_apiClient;

    // Directory tree state
    ftxui::Component m_dirTree;
    bool m_showDirTree{ false };
    std::vector<std::pair<bool, std::string>> m_dirContents;

    // Input handling
    std::string m_input;
    bool inputActive{ false };
    ftxui::Component m_inputBox;

    // Chat messages
    std::vector<ChatMessage> m_chatMessages;
    bool m_isStreaming{false};
    bool m_showDebug{false};
    ftxui::Component m_renderer;
    
    // Response polling
    std::thread m_pollingThread;
    std::atomic<bool> m_pollingActive{false};
    std::mutex m_chatMutex;
};

} // namespace cronus

#endif//_cronus_terminal_ui_h_
