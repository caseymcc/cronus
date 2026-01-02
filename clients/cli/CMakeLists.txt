find_package(Threads REQUIRED)
find_package(httplib CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(OpenSSL REQUIRED)

add_executable(cronus-cli main.cpp)

target_link_libraries(cronus-cli 
    PRIVATE 
        httplib::httplib 
        nlohmann_json::nlohmann_json 
        Threads::Threads
        OpenSSL::SSL
        OpenSSL::Crypto
)

# Enable C++17
target_compile_features(cronus-cli PRIVATE cxx_std_17)
