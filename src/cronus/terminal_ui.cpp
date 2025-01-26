#include "cronus/terminal_ui.h"
#include <ftxui/dom/elements.hpp>
#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <filesystem>

using namespace ftxui;

namespace cronus
{

TerminalUI::TerminalUI() :
    m_screen(Screen::Create(Dimension::Full(), Dimension::Full()))
{
    initializeDirTree();
}

void TerminalUI::initializeDirTree()
{
    m_dirTree=Container::Vertical({
        Button("Toggle Tree [F2]", [&] { m_showDirTree=!m_showDirTree; })
        });
}

Element TerminalUI::createDirTree() const
{
    std::vector<Element> tree;

    for(const auto &[isDir, name]:m_dirContents)
    {
        auto displayName=isDir?"📁 "+name:"📄 "+name;
        tree.push_back(text(displayName));
    }

    return vbox(tree)|border|size(WIDTH, LESS_THAN, 30);
}

void TerminalUI::updateDirectoryTree(const std::vector<std::pair<bool, std::string>> &contents)
{
    m_dirContents=contents;
}

Element TerminalUI::createMainLayout(const Element &content) const
{
    std::vector<Element> layout;
    layout.push_back(content);

    if(m_showDirTree)
    {
        layout.push_back(createDirTree());
    }

    return hbox(layout);
}

void TerminalUI::render(const Element &element) const
{
    m_screen.Clear();
    Render(m_screen, createMainLayout(element));
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

std::string TerminalUI::getUserInput()
{
    std::string input;
    auto screen=ScreenInteractive::TerminalOutput();

    Component inputBox=Input(&input, "Enter your message");

    // Combine input box with directory tree
    auto container=Container::Horizontal({
        inputBox,
        m_dirTree
        });

    auto renderer=Renderer(container, [&]
        {
            auto inputElement=vbox({
                text("Enter your message:")|bold,
                inputBox->Render()|border
                });

            return createMainLayout(inputElement);
        });

    // Handle both Enter and F2 keys
    container|=CatchEvent([&screen, this](Event event)
        {
            if(event==Event::Return)
            {
                screen.ExitLoopClosure()();
                return true;
            }
            if(event==Event::F2)
            {
                m_showDirTree = !m_showDirTree;
                return true;
            }
            return false;
        });

    screen.Loop(renderer);
    return input;
}

} // namespace cronus
