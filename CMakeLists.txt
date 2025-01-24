cmake_minimum_required(VERSION 3.15)
project(cronus VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Find dependencies
find_package(cpr CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

# Add library
add_library(cronus_lib
    src/llm.cpp
)

target_include_directories(cronus_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(cronus_lib PUBLIC
    cpr::cpr
    nlohmann_json::nlohmann_json
)

# Add executable
add_executable(${PROJECT_NAME} 
    src/main.cpp
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    cronus_lib
)
