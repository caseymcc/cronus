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
    m_logic.setResponseCallback([this](const std::string &provider, const std::string &response)
        {
            addResponse(provider, response);
        });

    m_logic.setLogCallback([this](LogLevel logLevel, const std::string &message)
        {
            addLog(logLevel, message);
        });

}

void TerminalUI::addResponse(const std::string &provider, const std::string &response)
{
    m_chatMessages.emplace_back(ChatType::Message, Role::Bot, response);
    m_screen.RequestAnimationFrame();
}

void TerminalUI::addLog(LogLevel logLevel, const std::string &message)
{
    Role role=Role::Log;

    switch(logLevel)
    {
        case LogLevel::Debug:
            role=Role::Log;
            break;
        case LogLevel::Info:
            role=Role::Log;
            break;
        case LogLevel::Warning:
            role=Role::Warning;
            break;
        case LogLevel::Error:
            role=Role::Error;
            break;
    }

    m_chatMessages.emplace_back(ChatType::Log, role, message);
    m_screen.RequestAnimationFrame();
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
            return renderMainLayout() | flex | size(WIDTH, EQUAL, 100) | size(HEIGHT, EQUAL, 100);
        });
}

void TerminalUI::run()
{
    setupUI();
    m_screen.Loop(m_renderer);
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

Element TerminalUI::renderDirTree()
{
    std::vector<Element> treeElements;
    for(const auto &[isDir, name]:m_dirContents)
    {
        auto displayName=isDir?"📁 "+name:"📄 "+name;
        treeElements.push_back(text(displayName));
    }
    return vbox(std::move(treeElements))|border|size(WIDTH, GREATER_THAN, 30);
}

Element TerminalUI::renderChatArea()
{
    std::vector<Element> chatElements;
    for(const auto &entry:m_chatMessages)
    {
        if(entry.type == ChatType::Log)
        {
            if(entry.role==Role::Error)
                chatElements.push_back(text("Error: "+entry.content)|bold|color(Color::Red));
            else if(entry.role==Role::Warning)
                chatElements.push_back(text("Warning: "+entry.content)|bold|color(Color::Yellow));
            else if(entry.role==Role::Log)
                chatElements.push_back(text("Log: "+entry.content)|bold|color(Color::Green));
            else
                chatElements.push_back(text(entry.content)|border);
        }
        else
            chatElements.push_back(text(entry.content)|border);
    }
    return vbox(std::move(chatElements))|border|flex|size(HEIGHT, GREATER_THAN, 10);
}

Element TerminalUI::renderInputArea()
{
    return vbox({
        text("Input:")|bold,
        m_inputBox->Render()|border
        });
}

Element TerminalUI::renderMainLayout()
{
    auto chatAndInput = vbox({
            renderChatArea() | flex_grow,
            renderInputArea() | size(HEIGHT, EQUAL, 5)
        });

    if(m_showDirTree)
    {
        return dbox({
            renderDirTree() | size(WIDTH, LESS_THAN, 60),
            chatAndInput
        });
    }

    return chatAndInput; // If toolbar is hidden, only show chat and input
}

} // namespace cronus
