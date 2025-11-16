include(FetchContent)

# Function to fetch and build Tree-sitter parsers
function(add_tree_sitter_parser NAME REPO TAG)
    FetchContent_Declare(
        tree-sitter_${NAME}
        GIT_REPOSITORY ${REPO}
        GIT_TAG ${TAG}
    )
    FetchContent_MakeAvailable(tree-sitter_${NAME})

    add_library(tree-sitter_${NAME} STATIC
        ${tree-sitter_${NAME}_SOURCE_DIR}/src/parser.c
    )

    # Some parsers have additional sources (scanner.c or scanner.cc)
    if (EXISTS "${tree-sitter_${NAME}_SOURCE_DIR}/src/scanner.c")
        target_sources(tree-sitter_${NAME} PRIVATE ${tree-sitter_${NAME}_SOURCE_DIR}/src/scanner.c)
    elseif (EXISTS "${tree-sitter_${NAME}_SOURCE_DIR}/src/scanner.cc")
        target_sources(tree-sitter_${NAME} PRIVATE ${tree-sitter_${NAME}_SOURCE_DIR}/src/scanner.cc)
        set_property(TARGET tree-sitter_${NAME} PROPERTY CXX_STANDARD 17) # Ensure C++17 for scanner.cc
    endif()

    target_include_directories(tree-sitter_${NAME} PUBLIC ${tree-sitter_${NAME}_SOURCE_DIR}/src)

    # Install the parser library into the project's build directory
    install(TARGETS tree-sitter_${NAME} DESTINATION ${CMAKE_BINARY_DIR}/lib)
    install(DIRECTORY ${tree-sitter_${NAME}_SOURCE_DIR}/src/ DESTINATION ${CMAKE_BINARY_DIR}/include/tree-sitter-${NAME})
endfunction()