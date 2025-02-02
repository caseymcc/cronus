#include "cronus/terminal_ui.h"

#include "cronus/client_logic.h"
#include "cronus/logger.h"

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
    // Set up logging callback
    Logger::instance().setCallback([this](LogLevel level, const std::string &message)
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

}

void TerminalUI::initializeDirTree()
{
    m_dirTree=Container::Vertical({
        Button("Toggle Tree [F2]", [&] { m_showDirTree=!m_showDirTree; })
        });

    for(const auto &[isDir, name]:m_dirContents)
    {
        auto displayName=isDir?"📁 "+name:"📄 "+name;
        m_dirTree->Add(Button(displayName, [] {})); // Add dummy buttons
    }
}

Element TerminalUI::renderDirTree()
{
    std::vector<Element> treeElements;
    for(const auto &[isDir, name]:m_dirContents)
    {
        auto displayName=isDir?"📁 "+name:"📄 "+name;
        treeElements.push_back(text(displayName));
    }
    return vbox(std::move(treeElements))|border|size(WIDTH, LESS_THAN, 30);
}

Element TerminalUI::renderChatArea()
{
    std::vector<Element> chatElements;
    for(const auto &message:m_chatMessages)
    {
        chatElements.push_back(text(message)|border);
    }
    return vbox(std::move(chatElements))|border|flex;
}

Element TerminalUI::renderInputArea()
{
    return vbox({
        text("Input:")|bold,
        m_inputBox->Render()|border
        });
}

void TerminalUI::handleInput(ftxui::Event event)
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

Element TerminalUI::renderMainLayout()
{
    ftxui::Component chatAndInput=vbox(
        {
            renderChatArea(),
            renderInputArea()
        })|flex;

    if(m_showDirTree)
    {
        return hbox({
            chatAndInput|flex,
            renderDirTree()|flex_shrink // Add directory tree on the right
            });
    }

    chatAndInput|=CatchEvent(handleInput(Event::event));

    return chatAndInput; // If toolbar is hidden, only show chat and input
}



void TerminalUI::displayWelcome()
{
    auto welcome=text("Cronus Client")|bold|color(Color::Blue);
    render(welcome);
}

void TerminalUI::displayResponse(const std::string &provider, const std::string &response)
{
    auto header=text(provider+" Response:")|bold|color(Color::Green);
    auto content=text(response);
    auto element=vbox({
        header,
        content
        })|border;
    render(element);
}

void TerminalUI::displayError(const std::string &message)
{
    auto error=text("Error: "+message)|bold|color(Color::Red);
    render(error|border);
}

void TerminalUI::displayLog(const std::string &message)
{
    auto log=text(message)|color(Color::Yellow);
    render(log|border);
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

void TerminalUI::setupUI()
{
    // Create input box
    m_inputBox=Input(&m_input, "Enter your message");

    // Initialize directory tree
    initializeDirTree();

    // Create main container with input and directory tree
    auto container=Container::Horizontal({
        Container::Vertical({
            m_inputBox
        }),
        m_dirTree
        });

    // Add event handling
    container|=CatchEvent([this](Event event)
        {
            handleInput(event);
            return true;
        });

    // Create the renderer
    m_renderer=Renderer(container, [this]
        {
            return renderMainLayout();
        });
}

void TerminalUI::run()
{
    setupUI();
    m_screen.Loop(m_renderer);
}

} // namespace cronus
