#ifndef _cronus_terminal_ui_h_
#define _cronus_terminal_ui_h_

#include <string>
#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/screen.hpp>
#include <ftxui/component/component.hpp>

namespace cronus
{

class TerminalUI
{
public:
    TerminalUI();
    void displayWelcome() const;
    void displayResponse(const std::string &provider, const std::string &response) const;
    void displayError(const std::string &message) const;
    void run();
    std::function<void(const std::string&)> onInput;

private:
    ftxui::ScreenInteractive m_screen;
    void render(const ftxui::Element& element);
    
    // Directory tree state
    bool m_showDirTree{true};
    ftxui::Component m_dirTree;
    std::vector<std::pair<bool, std::string>> m_dirContents;
    
    // Input handling
    std::string m_input;
    ftxui::Component m_inputBox;
    void handleInput(ftxui::Event event);
    void setupInput();
    
    // Helper methods
    void initializeDirTree();
    ftxui::Element createDirTree() const;
    
public:
    void updateDirectoryTree(const std::vector<std::pair<bool, std::string>>& contents);
    ftxui::Element createMainLayout(const ftxui::Element& content) const;
};

} // namespace cronus

#endif//_cronus_terminal_ui_h_
