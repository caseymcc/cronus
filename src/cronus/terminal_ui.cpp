#include "cronus/terminal_ui.h"

#include "cronus/cronus.h"
#include "cronus/logger.h"

#include <ftxui/dom/elements.hpp>
#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <nlohmann/json.hpp>

#include <filesystem>
#include <chrono>

using namespace ftxui;
using json = nlohmann::json;

namespace cronus
{

TerminalUI::TerminalUI(Cronus &cronus) :
    m_cronus(cronus),
    m_screen(ScreenInteractive::Fullscreen())
{
    // Start the REST API if not already running
    if (!m_cronus.isApiRunning()) {
        m_cronus.startRestApi();
    }
    
    // Get the API base URL
    m_apiBaseUrl = m_cronus.getApiBaseUrl();
    m_apiClient = std::make_unique<httplib::Client>(m_apiBaseUrl.c_str());
    
    // Set up logging callback
    m_cronus.setLogCallback([this](LogLevel logLevel, const std::string &message)
        {
            addLog(logLevel, message);
        });
    
    // Fetch initial directory contents
    fetchDirectoryContents();
    
    // Start response polling
    startResponsePolling();
}

TerminalUI::~TerminalUI()
{
    stopResponsePolling();
}

void TerminalUI::addResponse(const std::string &provider, const std::string &response)
{
    if(provider=="streaming")
    {
        if(!m_isStreaming)
        {
            m_isStreaming=true;
            m_chatMessages.emplace_back(ChatType::Message, Role::Bot, response);
        }
        else
        {
            if(!m_chatMessages.empty() && m_chatMessages.back().role == Role::Bot)
                m_chatMessages.back().content+=response;
            else
                m_chatMessages.emplace_back(ChatType::Message, Role::Bot, response);
        }
    }
    else
    {
        m_isStreaming=false;
        m_chatMessages.emplace_back(ChatType::Message, Role::Bot, response);
    }
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
            return renderMainLayout()|flex_grow;
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
        std::string input=m_input;

        m_input.clear();
        m_screen.RequestAnimationFrame();

        m_chatMessages.emplace_back(ChatType::Message, Role::User, input);
        sendInputToApi(input);
    }
    else if(event==Event::F2)
    {
        m_showDirTree=!m_showDirTree;
        if (m_showDirTree) {
            fetchDirectoryContents();
        }
    }
    else if(event==Event::F3)
    {
        m_showDebug=!m_showDebug;
    }
    else if(event==Event::Character("Event::CtrlD")) // Ctrl-D
    {
        stopResponsePolling();
        m_screen.Exit();
    }
    else if(event==Event::Character("Event::CtrlC")) // Ctrl-C
    {
        // Clear input but don't exit
        m_input.clear();
        m_screen.RequestAnimationFrame();
    }
    else if(event.is_character())
    {
        if(!inputActive)
        {
            m_input.clear();
            inputActive=true;
        }
        m_input+=event.character();
        m_screen.RequestAnimationFrame();
    }
    else if(event==Event::Backspace&&!m_input.empty())
    {
        m_input.pop_back();
        m_screen.RequestAnimationFrame();
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
    return vbox(std::move(treeElements))|border|bgcolor(Color::Black)|clear_under|size(WIDTH, LESS_THAN, 60);
}

Element TerminalUI::renderChatArea()
{
    std::vector<Element> chatElements;
    for(const auto &entry:m_chatMessages)
    {
        if(entry.type==ChatType::Log)
        {
            if(entry.role==Role::Error)
                chatElements.push_back(text("Error: "+entry.content)|bold|color(Color::Red));
            else if(entry.role==Role::Warning)
                chatElements.push_back(text("Warning: "+entry.content)|bold|color(Color::Yellow));
            else if(entry.role==Role::Log)
                chatElements.push_back(text("Log: "+entry.content)|color(Color::Green));
            else
                chatElements.push_back(text(entry.content));
        }
        else
        {
            if(entry.role==Role::User)
                chatElements.push_back(paragraphAlignLeft(entry.content)|color(Color::GrayDark));
            else
                chatElements.push_back(paragraphAlignRight(entry.content)|color(Color::Blue));
        }
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

Element TerminalUI::renderDebugArea()
{
    return vbox({
        text("Debug Info:")|bold,
        text("Press F3 to close"),
        text("Application Status:")|color(Color::Green),
        text(" - Streaming: "+std::string(m_isStreaming?"Yes":"No")),
        text(" - Directory Tree: "+std::string(m_showDirTree?"Visible":"Hidden")),
        text(" - Message Count: "+std::to_string(m_chatMessages.size()))
        })|border|bgcolor(Color::Black)|size(HEIGHT, LESS_THAN, 60);
}

Element TerminalUI::renderMainLayout()
{
    auto mainContent=vbox({
            m_showDebug?renderDebugArea():text(""),
            renderChatArea()|flex,
            renderInputArea()|size(HEIGHT, EQUAL, 5)
        });

    if(m_showDirTree)
    {
        return dbox({
            renderDirTree(),
            mainContent
            });
    }

    return mainContent; // If toolbar is hidden, only show chat and input
}

void TerminalUI::fetchDirectoryContents()
{
    if (!m_apiClient) {
        addLog(LogLevel::Error, "API client not initialized");
        return;
    }
    
    auto res = m_apiClient->Get("/api/directory");
    if (res && res->status == 200) {
        try {
            auto jsonResponse = json::parse(res->body);
            m_dirContents.clear();
            
            for (const auto& item : jsonResponse["contents"]) {
                m_dirContents.emplace_back(
                    item["isDirectory"].get<bool>(),
                    item["name"].get<std::string>()
                );
            }
            
            // Reinitialize the directory tree with new contents
            initializeDirTree();
            m_screen.RequestAnimationFrame();
            
        } catch (const std::exception& e) {
            addLog(LogLevel::Error, std::string("Error parsing directory contents: ") + e.what());
        }
    } else {
        addLog(LogLevel::Error, "Failed to fetch directory contents from API");
    }
}

void TerminalUI::sendInputToApi(const std::string& input)
{
    if (!m_apiClient) {
        addLog(LogLevel::Error, "API client not initialized");
        return;
    }
    
    json requestBody = {
        {"input", input}
    };
    
    auto res = m_apiClient->Post("/api/input", requestBody.dump(), "application/json");
    if (!res || res->status != 200) {
        addLog(LogLevel::Error, "Failed to send input to API");
    }
}

void TerminalUI::startResponsePolling()
{
    m_pollingActive = true;
    m_pollingThread = std::thread([this]() {
        // This is a simplified polling mechanism
        // In a real implementation, you would use WebSockets or SSE for streaming responses
        while (m_pollingActive) {
            // Sleep to avoid hammering the API
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            
            // In a real implementation, you would poll an endpoint that returns new responses
            // For now, we're just simulating this since we don't have a real endpoint
            
            // This would be replaced with actual API calls in a real implementation
        }
    });
}

void TerminalUI::stopResponsePolling()
{
    m_pollingActive = false;
    if (m_pollingThread.joinable()) {
        m_pollingThread.join();
    }
}

} // namespace cronus
