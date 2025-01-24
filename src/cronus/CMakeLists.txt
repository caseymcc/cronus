cmake_minimum_required(VERSION 3.15)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(cronus_src
    ./main.cpp
    ./client_logic.h
    ./client_logic.cpp
    ./terminal_ui.h
    ./terminal_ui.cpp
)

# Add executable
add_executable(cronus
    ${cronus_src}
)

target_include_directories(cronus 
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/..
)
target_link_libraries(cronus PRIVATE
    llm_hermes
)