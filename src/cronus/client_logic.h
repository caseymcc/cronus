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

    TerminalUI &ui_;
};

} // namespace cronus

#endif//_cronus_client_logic_h_
