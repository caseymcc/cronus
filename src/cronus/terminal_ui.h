#ifndef _cronus_terminal_ui_h_
#define _cronus_terminal_ui_h_

#include "client_logic.h"

#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/screen.hpp>
#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>

#include <string>
#include <vector>

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
    explicit TerminalUI(class ClientLogic &logic);

    void updateDirectoryTree(const std::vector<std::pair<bool, std::string>> &contents);

    void run();

private:
    void addResponse(const std::string &provider, const std::string &response);
    void addLog(LogLevel logLevel, const std::string &message);

    void handleInput(ftxui::Event event);

    // Helper methods
    void initializeDirTree();
    void setupUI();

    ftxui::Element renderDirTree();
    ftxui::Element renderChatArea();
    ftxui::Element renderInputArea();
    ftxui::Element renderMainLayout();

    ClientLogic &m_logic;
    ftxui::ScreenInteractive m_screen;

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
    ftxui::Component m_renderer;
};

} // namespace cronus

#endif//_cronus_terminal_ui_h_
