cmake_minimum_required(VERSION 3.15)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(cronus_src
    src/main.cpp
)

# Add executable
add_executable(cronus
    ${cronus_src}
)

target_link_libraries(cronus PRIVATE
    llm_hermes
)