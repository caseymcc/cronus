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

}

void TerminalUI::initializeDirTree()
{
    m_dirTree=Container::Vertical({
        Button("Toggle Tree [F2]", [&] { m_showDirTree=!m_showDirTree; })
        });
}

// Create the directory tree element
Element TerminalUI::createDirTree() const {
    std::vector<Element> treeElements;
    for (const auto& [isDir, name] : m_directoryContents) {
        auto displayName = isDir ? "📁 " + name : "📄 " + name;
        treeElements.push_back(text(displayName));
    }
    return vbox(treeElements) | border | size(WIDTH, LESS_THAN, 30);
}

// Create the chat messages area
Element TerminalUI::createChatArea() const {
    std::vector<Element> chatElements;
    for (const auto& message : m_chatMessages) {
        chatElements.push_back(text(message) | border);
    }
    return vbox(chatElements) | border | flex;
}

// Create the input area
Element TerminalUI::createInputArea() const {
    return vbox({
        text("Input:") | bold,
        m_inputBox->Render() | border
    });
}

void initializeDirectoryTree() {
        for (const auto& [isDir, name] : m_directoryContents) {
            auto displayName = isDir ? "📁 " + name : "📄 " + name;
            m_dirTree->Add(Button(displayName, [] {})); // Add dummy buttons
        }
    }

    void initializeInputArea() {
        m_inputBox = Input(&m_userInput, "Type your message...");
    }

Element TerminalUI::createMainLayout(const Element &content) const
{
    auto chatAndInput=vbox(
        {
            createChatArea(),
            createInputArea()
        })|flex;

    if(m_showDirTree)
    {
        return hbox({
            chatAndInput|flex,
            createDirTree()|flex_shrink // Add directory tree on the right
            });
    }
    return chatAndInput; // If toolbar is hidden, only show chat and input
}

void TerminalUI::render(const Element &element)
{
    m_screen.Clear();
    Render(m_screen, createMainLayout(element));
    m_screen.Print();
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

void TerminalUI::setupUI()
{
    m_inputBox=Input(&m_input, "Enter your message");

    initializeDirTree();

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
    setupUI();

    m_screen.Loop(renderer);
}

} // namespace cronus
