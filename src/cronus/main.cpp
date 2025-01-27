#include "cronus/terminal_ui.h"
#include "cronus/client_logic.h"

#include "cronus/config.h"

int main()
{
    // Load configuration
    cronus::Config::instance().load();

    cronus::ClientLogic logic;
    logic.run();
    
    cronus::TerminalUI ui(logic);
    ui.run();
    
    logic.stop();
}
