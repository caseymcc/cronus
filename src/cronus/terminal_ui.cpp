#include "cronus/terminal_ui.h"
#include <ftxui/dom/elements.hpp>
#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>

using namespace ftxui;

namespace cronus
{

TerminalUI::TerminalUI() : m_screen(Screen::Create(Dimension::Full(), Dimension::Fixed(1))) {}

void TerminalUI::render(const Element &element) const
{
    m_screen.Clear();
    Render(m_screen, element);
    m_screen.Print();
}

void TerminalUI::displayWelcome() const
{
    auto welcome=text("Cronus Client")|bold|color(Color::Blue);
    render(welcome);
}

void TerminalUI::displayResponse(const std::string &provider, const std::string &response) const
{
    auto header=text(provider+" Response:")|bold|color(Color::Green);
    auto content=text(response);
    auto element=vbox({
        header,
        content
        })|border;
    render(element);
}

void TerminalUI::displayError(const std::string &message) const
{
    auto error=text("Error: "+message)|bold|color(Color::Red);
    render(error|border);
}

std::string TerminalUI::getUserInput() const
{
    std::string input;
    auto screen=ScreenInteractive::TerminalOutput();

    Component inputBox=Input(&input, "Enter your message");
    auto renderer=Renderer(inputBox, [&]
        {
            return vbox({
                text("Enter your message:")|bold,
                inputBox->Render()|border
                });
        });

    auto enterPressed=false;
    inputBox|=CatchEvent([&](Event event)
        {
            if(event==Event::Return)
            {
                enterPressed=true;
                screen.ExitLoopClosure()();
                return true;
            }
            return false;
        });

    screen.Loop(renderer);
    return input;
}

} // namespace cronus
