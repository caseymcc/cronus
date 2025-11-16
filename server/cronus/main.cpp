#include "cronus/cronus.h"
#include "cronus/config.h"
#include "cronus/webServer.h"

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
        <<"  -w, --web                   Use web UI (browser-based) instead of terminal UI\n"
        <<"  -p, --port <port>           Set port for web UI (default: 3000)\n"
        <<"  -h, --help                  Show this help message\n";
}

int main(int argc, char *argv[])
{
    std::string resourcePath;
    bool useWebUI=false;
    int webPort=3000;

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
    }

    // Register signal handler for Ctrl+C
    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);

    // Load configuration
    cronus::Config::instance().load(resourcePath);

    cronusApp.run(resourcePath);

    // Create and start the web server if requested
    std::unique_ptr<cronus::WebServer> webServer;
    if(useWebUI)
    {
        std::cout<<"Starting Cronus with web UI on port "<<webPort<<std::endl;
        std::cout<<"Open your browser and navigate to http://localhost:"<<webPort<<std::endl;

        webServer = std::make_unique<cronus::WebServer>(cronusApp, webPort);
        if(!webServer->start())
        {
            std::cerr<<"Failed to start web server"<<std::endl;
            cronusApp.stop();
            return 1;
        }
    }
    else
    {
        std::cout<<"Starting Cronus in terminal mode"<<std::endl;
    }

    // Keep the application running until the user presses Ctrl+C
    std::cout<<"Press Ctrl+C to stop Cronus..."<<std::endl;
    
    cronusApp.waitForComplete();
    
    // Stop the web server if it was started
    if(webServer)
    {
        webServer->stop();
    }

    // Stop the Cronus application
    
    
    return 0;
}
