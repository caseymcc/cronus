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
    std::string getUserInput() const;

private:
    mutable ftxui::Screen m_screen;
    void render(const ftxui::Element& element) const;
    
    // Directory tree state
    bool m_showDirTree{true};
    std::filesystem::path m_currentPath;
    ftxui::Component m_dirTree;
    
    // Helper methods
    void initializeDirTree();
    ftxui::Element createDirTree() const;
    ftxui::Element createMainLayout(const ftxui::Element& content) const;
};

} // namespace cronus

#endif//_cronus_terminal_ui_h_
