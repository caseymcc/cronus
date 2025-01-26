#ifndef _cronus_client_logic_h_
#define _cronus_client_logic_h_

#include "llm_hermes/hermes.h"
#include "cronus/terminal_ui.h"

namespace cronus
{

class ClientLogic
{
public:
    explicit ClientLogic(TerminalUI &ui);
    int run();

private:
    int processCompletion(const std::string &input);
    void updateDirectoryTree();
    std::vector<std::pair<bool, std::string>> getCurrentDirectoryContents() const;

    TerminalUI &m_ui;
    std::filesystem::path m_currentPath;
};

} // namespace cronus

#endif//_cronus_client_logic_h_
