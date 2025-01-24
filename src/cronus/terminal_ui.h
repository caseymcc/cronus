#ifndef _cronus_terminal_ui_h_
#define _cronus_terminal_ui_h_

#include <string>
#include <iostream>

namespace cronus
{

class TerminalUI
{
public:
    void display_welcome() const;
    void display_response(const std::string &provider, const std::string &response) const;
    void display_error(const std::string &message) const;
    std::string get_user_input() const;
};

} // namespace cronus

#endif//_cronus_terminal_ui_h_
