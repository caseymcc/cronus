#include <iostream>
#include <string>
#include <thread>
#include <atomic>
#include <chrono>
#include <mutex>

#define CPPHTTPLIB_OPENSSL_SUPPORT
#include <httplib.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

class CronusClient {
public:
    CronusClient(const std::string& host, int port)
        : m_host(host), m_port(port) {}

    ~CronusClient() {
        m_running = false;
        if (m_listenerThread.joinable()) {
            m_listenerThread.join();
        }
    }

    bool connect() {
        m_cli = std::make_unique<httplib::Client>(m_host, m_port);
        m_cli->set_connection_timeout(5); 
        
        // Simple health check
        auto res = m_cli->Get("/api/health");
        if (!res || res->status != 200) {
            std::cerr << "Failed to connect to Cronus server at " << m_host << ":" << m_port << std::endl;
            return false;
        }

        std::cout << "Connected to Cronus server!" << std::endl;
        
        // Start listening for SSE events
        m_running = true;
        m_listenerThread = std::thread(&CronusClient::listenForEvents, this);
        
        return true;
    }

    void run() {
        std::cout << "Type your message and press Enter. Type 'exit' to quit." << std::endl;
        
        while (m_running) {
            std::cout << "> " << std::flush;
            
            std::string input;
            std::getline(std::cin, input);
            
            if (input == "exit" || input == "quit") {
                m_running = false;
                break;
            }
            
            if (input.empty()) continue;
            
            sendInput(input);
        }
    }

private:
    void sendInput(const std::string& input) {
        json payload = {
            {"input", input}
        };
        
        auto res = m_cli->Post("/api/input", payload.dump(), "application/json");
        if (!res || res->status != 200) {
            std::cerr << "Error sending input: " << (res ? std::to_string(res->status) : "Connection failed") << std::endl;
        }
    }

    void listenForEvents() {
        httplib::Client cli(m_host, m_port);
        cli.set_keep_alive(true);
        cli.set_read_timeout(300); // 5 minutes

        while (m_running) {
            cli.Get("/api/events", [&](const char* data, size_t data_length) {
                if (!m_running) return false;
                
                std::string chunk(data, data_length);
                processStreamData(chunk);
                
                return true;
            });
            
            if (m_running) {
                // connection lost, retry
                std::this_thread::sleep_for(std::chrono::seconds(2));
            }
        }
    }
    
    void processStreamData(const std::string& data) {
         // Simple SSE parser
         std::stringstream ss(data);
         std::string line;
         while (std::getline(ss, line)) {
             if (line.rfind("data: ", 0) == 0) {
                 std::string jsonStr = line.substr(6);
                 try {
                     auto j = json::parse(jsonStr);
                     
                     if (j.contains("type")) {
                         std::string type = j["type"];
                         
                         if (type == "agent_response") {
                             std::string provider = j.value("provider", "unknown");
                             std::string content = j.value("content", "");
                             
                             std::cout << "\r\n[" << provider << "]: " << content << "\n> " << std::flush;
                         } else if (type == "directory_update") {
                            // ignore for now in CLI
                         } else if (type == "directory_init") {
                            // ignore
                         }
                     }
                 } catch (const std::exception& e) {
                     // ignore parse errors
                 }
             }
         }
    }

    std::string m_host;
    int m_port;
    std::unique_ptr<httplib::Client> m_cli;
    std::atomic<bool> m_running{false};
    std::thread m_listenerThread;
};

int main(int argc, char* argv[]) {
    std::string host = "localhost";
    int port = 9000;

    if (argc > 1) host = argv[1];
    if (argc > 2) port = std::stoi(argv[2]);

    CronusClient client(host, port);
    if (client.connect()) {
        client.run();
    }

    return 0;
}
