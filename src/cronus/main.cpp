#include "cronus/terminal_ui.h"
#include "cronus/cronus.h"
#include "cronus/config.h"

#include <iostream>

void printUsage(const char* programName) {
    std::cout << "Usage: " << programName << " [options]\n"
              << "Options:\n"
              << "  -r, --resource-dir <path>   Set custom resource directory path\n"
              << "  -h, --help                  Show this help message\n";
}

int main(int argc, char* argv[])
{
    std::string resourcePath;

    // Parse command line arguments
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-h" || arg == "--help") {
            printUsage(argv[0]);
            return 0;
        } else if (arg == "-r" || arg == "--resource-dir") {
            if (i + 1 < argc) {
                resourcePath = argv[++i];
            } else {
                std::cerr << "Error: --resource-dir requires a path argument\n";
                return 1;
            }
        }
    }

    // Load configuration
    cronus::Config::instance().load(resourcePath);

    cronus::Cronus cronusApps;
    cronusApps.run();
    
    cronus::TerminalUI ui(cronusApps);
    ui.run();
    
    cronusApps.stop();
}
