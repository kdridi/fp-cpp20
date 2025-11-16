cmake_minimum_required(VERSION 3.20)
project(segy VERSION 1.0.0 LANGUAGES CXX)

# ============================================================================
# COMPILER REQUIREMENTS
# ============================================================================

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ============================================================================
# OPTIMIZATION FLAGS (ULTRA INSTINCT MODE)
# ============================================================================

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(
        -Wall -Wextra -Wpedantic -Werror
        -Wconversion
        # -Wsign-conversion # Too strict for binary format handling
    )

    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        add_compile_options(-g -O0)
    else()
        add_compile_options(-O3 -DNDEBUG)
    endif()
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    add_compile_options(
        /W4 /WX
        $<$<CONFIG:Debug>:/Od>
        $<$<CONFIG:Release>:/O2>
    )
endif()

# ============================================================================
# SANITIZERS (UB AWARENESS)
# ============================================================================

option(ENABLE_SANITIZERS "Enable address and undefined behavior sanitizers" OFF)

if(ENABLE_SANITIZERS)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        add_compile_options(-fsanitize=address,undefined)
        add_link_options(-fsanitize=address,undefined)
    endif()
endif()

# ============================================================================
# LIBRARY (Header-only)
# ============================================================================

add_library(segy INTERFACE)
target_include_directories(segy INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# ============================================================================
# TESTS
# ============================================================================

option(BUILD_TESTS "Build tests" ON)

if(BUILD_TESTS)
    enable_testing()

    # Unit tests
    add_executable(test_endian tests/unit/test_endian.cpp)
    target_link_libraries(test_endian PRIVATE segy)
    add_test(NAME test_endian COMMAND test_endian)

    add_executable(test_headers tests/unit/test_headers.cpp)
    target_link_libraries(test_headers PRIVATE segy)
    add_test(NAME test_headers COMMAND test_headers)

    add_executable(test_trace tests/unit/test_trace.cpp)
    target_link_libraries(test_trace PRIVATE segy)
    add_test(NAME test_trace COMMAND test_trace)

    add_executable(test_reader_writer tests/unit/test_reader_writer.cpp)
    target_link_libraries(test_reader_writer PRIVATE segy)
    add_test(NAME test_reader_writer COMMAND test_reader_writer)

    # Compilation tests (static_assert)
    add_executable(test_concepts tests/compilation/test_concepts.cpp)
    target_link_libraries(test_concepts PRIVATE segy)
    add_test(NAME test_concepts COMMAND test_concepts)
endif()

# ============================================================================
# EXAMPLES
# ============================================================================

option(BUILD_EXAMPLES "Build examples" ON)

if(BUILD_EXAMPLES)
    add_executable(simple_reader examples/simple_reader.cpp)
    target_link_libraries(simple_reader PRIVATE segy)

    add_executable(simple_writer examples/simple_writer.cpp)
    target_link_libraries(simple_writer PRIVATE segy)
endif()

# ============================================================================
# INSTALLATION
# ============================================================================

install(DIRECTORY include/segy DESTINATION include)
install(TARGETS segy EXPORT segyTargets)

install(EXPORT segyTargets
    FILE segyTargets.cmake
    NAMESPACE segy::
    DESTINATION lib/cmake/segy
)

# ============================================================================
# BUILD INFO
# ============================================================================

message(STATUS "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━")
message(STATUS "SEG-Y Library Configuration")
message(STATUS "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━")
message(STATUS "C++ Standard:     C++20")
message(STATUS "Build Type:       ${CMAKE_BUILD_TYPE}")
message(STATUS "Compiler:         ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "Build Tests:      ${BUILD_TESTS}")
message(STATUS "Build Examples:   ${BUILD_EXAMPLES}")
message(STATUS "Sanitizers:       ${ENABLE_SANITIZERS}")
message(STATUS "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━")
