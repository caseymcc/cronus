#ifndef _cronus_terminal_ui_h_
#define _cronus_terminal_ui_h_

#include <string>
#include <iostream>

namespace cronus
{

class TerminalUI
{
public:
    void displayWelcome() const;
    void displayResponse(const std::string &provider, const std::string &response) const;
    void displayError(const std::string &message) const;
    std::string getUserInput() const;
};

} // namespace cronus

#endif//_cronus_terminal_ui_h_
