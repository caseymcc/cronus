cmake_minimum_required(VERSION 3.15)
project(llm_hermes VERSION 0.1.0 LANGUAGES CXX)

# Find dependencies
find_package(cpr CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

set(llm_hermes_src
    ./hermes.h
    ./hermes.cpp
    ./models_manager.h
    ./models_manager.cpp
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
add_library(llm_hermes
    ${llm_hermes_src}
)

target_include_directories(llm_hermes 
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_link_libraries(llm_hermes PUBLIC
    cpr::cpr
    nlohmann_json::nlohmann_json
)
