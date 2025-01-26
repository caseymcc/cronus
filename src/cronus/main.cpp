#include "cronus/terminal_ui.h"
#include "cronus/client_logic.h"

#include "cronus/config.h"

int main()
{
    // Load configuration
    cronus::Config::instance().load();

    cronus::TerminalUI ui;
    cronus::ClientLogic logic(ui);
    
    logic.start();
    
    while (true) {
        std::string userInput = ui.getUserInput();
        auto future = logic.processInput(userInput);
        
        if (future.get() != 0) {
            return 1;
        }
    }
}
