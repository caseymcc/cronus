#include "cronus/terminal_ui.h"

namespace cronus
{

void TerminalUI::displayWelcome() const
{
    std::cout<<"Cronus Client"<<std::endl;
}

void TerminalUI::displayResponse(const std::string &provider, const std::string &response) const
{
    std::cout<<provider<<" Response: "<<response<<std::endl;
}

void TerminalUI::displayError(const std::string &message) const
{
    std::cerr<<"Error: "<<message<<std::endl;
}

std::string TerminalUI::getUserInput() const
{
    std::cout<<"Enter your message: ";
    std::string input;
    std::getline(std::cin, input);
    return input;
}

} // namespace cronus
