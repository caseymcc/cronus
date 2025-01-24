#include "cronus/terminal_ui.h"
#include "cronus/client_logic.h"

int main() {
    cronus::TerminalUI ui;
    cronus::ClientLogic logic(ui);
    return logic.run();
}
