cmake_minimum_required(VERSION 3.15)
project(hermes VERSION 0.1.0 LANGUAGES CXX)

# Find dependencies
find_package(cpr CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

set(hermes_src
    ./hermes.h
    ./hermes.cpp
    ./model_manager.h
    ./model_manager.cpp
    ./providers/base_llm.h
    ./providers/base_llm.cpp
    ./providers/openai_llm.h
    ./providers/openai_llm.cpp
    ./providers/anthropic_llm.h
    ./providers/anthropic_llm.cpp
    ./providers/deepseek_llm.h
    ./providers/deepseek_llm.cpp
)

# Add library
add_library(hermes
    ${hermes_src}
)

target_include_directories(hermes 
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_link_libraries(hermes PUBLIC
    cpr::cpr
    nlohmann_json::nlohmann_json
)
