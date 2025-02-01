#ifndef _cronus_terminal_ui_h_
#define _cronus_terminal_ui_h_

#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/screen.hpp>
#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>

#include <string>

namespace cronus
{

class TerminalUI
{
public:
    TerminalUI();
    explicit TerminalUI(class ClientLogic& logic);

    void run();

    void updateDirectoryTree(const std::vector<std::pair<bool, std::string>>& contents);
    ftxui::Element createMainLayout(const ftxui::Element& content) const;

private:
    void displayWelcome();
    void displayResponse(const std::string &provider, const std::string &response);
    void displayError(const std::string &message);
    void displayLog(const std::string &message);

    void handleInput(ftxui::Event event);
    void setupUI();
    
    // Helper methods
    void initializeDirTree();
    ftxui::Element createDirTree() const;
    void render(const ftxui::Element& element);

    ClientLogic& m_logic;
    ftxui::ScreenInteractive m_screen;
        
    // Directory tree state
    ftxui::Component m_dirTree;
    bool m_showDirTree{true};
    std::vector<std::pair<bool, std::string>> m_dirContents;    
    
    // Input handling
    std::string m_input;
    ftxui::Component m_inputBox;
    
    
public:
    
};

} // namespace cronus

#endif//_cronus_terminal_ui_h_
