#include "cronus/terminal_ui.h"

#include "cronus/client_logic.h"

#include <ftxui/dom/elements.hpp>
#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>

#include <filesystem>

using namespace ftxui;

namespace cronus
{

TerminalUI::TerminalUI(ClientLogic &logic) :
    m_logic(logic),
    m_screen(ScreenInteractive::TerminalOutput())
{
    // Set up callbacks
    m_logic.setLogCallback([this](const std::string &message)
        {
            displayLog(message);
        });

    m_logic.setResponseCallback([this](const std::string &provider, const std::string &response)
        {
            displayResponse(provider, response);
        });

    m_logic.setErrorCallback([this](const std::string &error)
        {
            displayError(error);
        });

    initializeDirTree();
    setupInput();
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

void TerminalUI::setupInput()
{
    m_inputBox=Input(&m_input, "Enter your message");

    auto container=Container::Horizontal({
        m_inputBox,
        m_dirTree
        });

    container|=CatchEvent([this](Event event)
        {
            handleInput(event);
            return true;
        });

    auto renderer=Renderer(container, [this]
        {
            auto inputElement=vbox({
                text("Enter your message:")|bold,
                m_inputBox->Render()|border
                });
            return createMainLayout(inputElement);
        });

    m_screen.Loop(renderer);
}

void TerminalUI::handleInput(Event event)
{
    if(event==Event::Return&&!m_input.empty())
    {
        m_logic.processInput(m_input);
        m_input.clear();
    }
    else if(event==Event::F2)
    {
        m_showDirTree=!m_showDirTree;
    }
    else if(event==Event::Character('q'))
    {
        m_screen.Exit();
    }
}

void TerminalUI::run()
{
    displayWelcome();
    setupInput();
}

} // namespace cronus
