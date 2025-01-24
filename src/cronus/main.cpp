#include "cronus/terminal_ui.h"
#include "cronus/client_logic.h"

#include "cronus/config.h"

int main()
{
    // Load configuration
    cronus::Config::instance().load();

    cronus::TerminalUI ui;
    cronus::ClientLogic logic(ui);
    return logic.run();
}
