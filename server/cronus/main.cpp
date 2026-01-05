#include "cronus/cronus.h"
#include "cronus/config.h"
//#include "cronus/webServer.h"
#include "cronus/modeDetector.h"
#include "cronus/directoryInitializer.h"
#include "cronus/startupNotifier.h"

#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <csignal>

cronus::Cronus cronusApp;

// Signal handler for Ctrl+C
void signalHandler(int signal)
{
    if (signal == SIGINT || signal == SIGTERM)
    {
        std::cout << "\nReceived termination signal. Shutting down..." << std::endl;
        cronusApp.stop();
    }
}

void printUsage(const char *programName)
{
    std::cout<<"Usage: "<<programName<<" [options]\n"
        <<"Options:\n"
        <<"  -r, --resource-dir <path>   Set custom resource directory path\n"
        <<"  -w, --web                   Enable web UI (disabled by default)\n"
        <<"  -p, --port <port>           Set port for web UI (default: 9000)\n"
        <<"  --mode <mode>               Force operational mode: single-agent or multi-agent\n"
        <<"  --init                      Initialize .cronus/ directory and exit\n"
        <<"  --detect-mode               Detect and display operational mode, then exit\n"
        <<"  -h, --help                  Show this help message\n"
        <<"\n"
        <<"By default, Cronus runs with JSON-RPC server only (no web UI).\n"
        <<"Use --web flag to enable the browser-based web interface.\n";
}

int main(int argc, char *argv[])
{
    std::string resourcePath;
    bool useWebUI=false;
    int webPort=9000;
    std::optional<cronus::OperationalMode> forcedMode;
    bool initOnly=false;
    bool detectModeOnly=false;

    // Parse command line arguments
    for(int i=1; i<argc; ++i)
    {
        std::string arg=argv[i];
        if(arg=="-h"||arg=="--help")
        {
            printUsage(argv[0]);
            return 0;
        }
        else if(arg=="-r"||arg=="--resource-dir")
        {
            if(i+1<argc)
            {
                resourcePath=argv[++i];
            }
            else
            {
                std::cerr<<"Error: --resource-dir requires a path argument\n";
                return 1;
            }
        }
        else if(arg=="-w"||arg=="--web")
        {
            useWebUI=true;
        }
        else if(arg=="-p"||arg=="--port")
        {
            if(i+1<argc)
            {
                try
                {
                    webPort=std::stoi(argv[++i]);
                }
                catch(const std::exception &e)
                {
                    std::cerr<<"Error: --port requires a valid port number\n";
                    return 1;
                }
            }
            else
            {
                std::cerr<<"Error: --port requires a number argument\n";
                return 1;
            }
        }
        else if(arg=="--mode")
        {
            if(i+1<argc)
            {
                std::string modeStr=argv[++i];
                auto mode=cronus::stringToOperationalMode(modeStr);
                if(mode.has_value())
                {
                    forcedMode=mode.value();
                }
                else
                {
                    std::cerr<<"Error: Invalid mode '"<<modeStr<<"'. Use 'single-agent' or 'multi-agent'\n";
                    return 1;
                }
            }
            else
            {
                std::cerr<<"Error: --mode requires a mode argument\n";
                return 1;
            }
        }
        else if(arg=="--init")
        {
            initOnly=true;
        }
        else if(arg=="--detect-mode")
        {
            detectModeOnly=true;
        }
    }

    // Handle --detect-mode
    if(detectModeOnly)
    {
        auto workingDir=std::filesystem::current_path();
        auto detectionResult=cronus::ModeDetector::detect(workingDir);
        
        std::cout<<"Mode Detection Results:\n";
        std::cout<<"  Working Directory: "<<workingDir.string()<<"\n";
        std::cout<<"  Detected Mode: "<<cronus::operationalModeToString(detectionResult.mode)<<"\n";
        std::cout<<"  Reason: "<<detectionResult.detectionReason<<"\n";
        std::cout<<"  Has Existing Config: "<<(detectionResult.hasExistingConfig?"Yes":"No")<<"\n";
        std::cout<<"  Has Version Control: "<<(detectionResult.hasVersionControl?"Yes":"No")<<"\n";
        std::cout<<"  Is Empty Directory: "<<(detectionResult.isEmptyDirectory?"Yes":"No")<<"\n";
        std::cout<<"  Has Multi-Agent Marker: "<<(detectionResult.hasMultiAgentMarker?"Yes":"No")<<"\n";
        
        return 0;
    }

    // Handle --init
    if(initOnly)
    {
        auto workingDir=std::filesystem::current_path();
        cronus::OperationalMode mode;
        
        if(forcedMode.has_value())
        {
            mode=forcedMode.value();
            std::cout<<"Initializing in "<<cronus::operationalModeToString(mode)<<" mode (forced)\n";
        }
        else
        {
            auto detectionResult=cronus::ModeDetector::detect(workingDir);
            mode=detectionResult.mode;
            std::cout<<"Initializing in "<<cronus::operationalModeToString(mode)<<" mode (detected)\n";
            std::cout<<"Detection reason: "<<detectionResult.detectionReason<<"\n";
        }
        
        auto initResult=cronus::DirectoryInitializer::initialize(workingDir, mode);
        
        if(initResult.success)
        {
            std::cout<<"✓ "<<initResult.message<<"\n";
            std::cout<<"  Configuration directory: "<<initResult.cronusDir.string()<<"\n";
            return 0;
        }
        else
        {
            std::cerr<<"✗ "<<initResult.message<<"\n";
            return 1;
        }
    }

    // Register signal handler for Ctrl+C
    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);

    // Load configuration (this will auto-detect mode and initialize if needed)
    cronus::Config::instance().load(resourcePath);
    
    // Override mode if forced via command line
    if(forcedMode.has_value())
    {
        cronus::Config::instance().setOperationalMode(forcedMode.value());
        std::cout<<"Operational mode forced to: "<<cronus::operationalModeToString(forcedMode.value())<<"\n";
    }
    
    std::cout<<"Running in "<<cronus::operationalModeToString(cronus::Config::instance().getOperationalMode())<<" mode\n";

    cronusApp.run(resourcePath);

    // Start the web server if requested
    std::cout<<"Starting Cronus on port "<<webPort<<std::endl;

    cronusApp.startWebServer(webPort);

    // Send startup notification to let clients know the server is ready
    std::string serverUrl = "http://localhost:" + std::to_string(webPort);
    cronus::StartupNotifier::notify(
        cronus::Config::instance().getNotificationHost(),
        cronus::Config::instance().getNotificationPort(),
        serverUrl
    );
    
    // Keep the application running until the user presses Ctrl+C
    std::cout<<"Press Ctrl+C to stop Cronus..."<<std::endl;
    
    cronusApp.waitForComplete();
    
    // Stop the Cronus application (this will also stop the web server if running)
    cronusApp.stop();
    
    
    return 0;
}
