cmake_minimum_required(VERSION 3.16)
project(mero VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Build options
option(MERO_BUILD_SHARED "Build shared library" ON)
option(MERO_BUILD_STATIC "Build static library" ON)
option(MERO_BUILD_CLI "Build command-line tool" ON)
option(MERO_BUILD_TESTS "Build tests" OFF)

# Compiler flags
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    add_compile_options(-Wall -Wextra -O2)
    if(NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
        add_compile_options(-DNDEBUG)
    endif()
elseif(MSVC)
    add_compile_options(/W3 /O2)
endif()

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/src/engine_cpp)

# C Core sources
set(MERO_C_SOURCES
    src/core_c/arena.c
    src/core_c/string_pool.c
    src/core_c/hash_map.c
    src/core_c/error.c
)

# C++ Engine sources
set(MERO_CXX_SOURCES
    src/engine_cpp/lexer.cpp
    src/engine_cpp/parser.cpp
    src/engine_cpp/semantic_analyzer.cpp
    src/engine_cpp/ir_builder.cpp
    src/engine_cpp/optimizer.cpp
    src/engine_cpp/codegen_luau.cpp
    src/engine_cpp/type_checker.cpp
    src/engine_cpp/compiler.cpp
    src/engine_cpp/diagnostics.cpp
    src/engine_cpp/ast_printer.cpp
    src/engine_cpp/builtin_mapper.cpp
)

# Shared library
if(MERO_BUILD_SHARED)
    add_library(mero SHARED ${MERO_C_SOURCES} ${MERO_CXX_SOURCES})
    target_include_directories(mero PUBLIC
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    )
    set_target_properties(mero PROPERTIES
        VERSION ${PROJECT_VERSION}
        SOVERSION 1
    )
endif()

# Static library
if(MERO_BUILD_STATIC)
    add_library(mero_static STATIC ${MERO_C_SOURCES} ${MERO_CXX_SOURCES})
    target_include_directories(mero_static PUBLIC
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    )
    set_target_properties(mero_static PROPERTIES OUTPUT_NAME mero_static)
endif()

# CLI executable
if(MERO_BUILD_CLI)
    add_executable(meroc src/engine_cpp/main.cpp)
    if(MERO_BUILD_STATIC)
        target_link_libraries(meroc PRIVATE mero_static)
    else()
        target_link_libraries(meroc PRIVATE mero)
    endif()
endif()

# Installation
install(TARGETS mero mero_static meroc
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
)
install(DIRECTORY include/mero DESTINATION include)
