cmake_minimum_required(VERSION 3.15)
project(cronus_llm VERSION 0.1.0 LANGUAGES CXX)

# Find dependencies
find_package(cpr CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

# Add library
add_library(cronus_llm
    src/llm.cpp
)

target_include_directories(cronus_llm 
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(cronus_llm PUBLIC
    cpr::cpr
    nlohmann_json::nlohmann_json
)
