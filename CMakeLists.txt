cmake_minimum_required(VERSION 3.20)
project(fp20
    VERSION 0.1.0
    DESCRIPTION "C++20 Header-Only Functional Programming Library - Haskell-Inspired"
    LANGUAGES CXX
)

# ============================================================================
# PERFORMANCE OPTIMIZATION SETTINGS - CMAKE WIZARD EDITION
# ============================================================================

# Force Ninja generator if available
set(CMAKE_GENERATOR "Ninja" CACHE INTERNAL "")

# Parallel compilation with all available cores
include(ProcessorCount)
ProcessorCount(N)
if(NOT N EQUAL 0)
    set(CMAKE_BUILD_PARALLEL_LEVEL ${N} CACHE STRING "Number of parallel build jobs" FORCE)
    message(STATUS "Parallel Build: Using ${N} cores")
endif()

# CCache support (if available)
find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
    set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_PROGRAM}" CACHE STRING "")
    message(STATUS "CCache: ENABLED (${CCACHE_PROGRAM})")
else()
    message(STATUS "CCache: NOT FOUND - Install with: brew install ccache")
endif()

# Default to Release mode for maximum performance
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
    message(STATUS "Build Type: Release (default - optimized)")
endif()

# Interprocedural optimization (LTO) for Release builds
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    include(CheckIPOSupported)
    check_ipo_supported(RESULT IPO_SUPPORTED OUTPUT IPO_ERROR)
    if(IPO_SUPPORTED)
        set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
        message(STATUS "LTO: ENABLED (Link-Time Optimization)")
    else()
        message(STATUS "LTO: NOT SUPPORTED - ${IPO_ERROR}")
    endif()
endif()

# ============================================================================
# C++20 CONFIGURATION
# ============================================================================

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compiler warnings
if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Make warnings errors optional (disabled by default for faster development)
option(FP20_WARNINGS_AS_ERRORS "Treat warnings as errors" OFF)
if(FP20_WARNINGS_AS_ERRORS)
    if(MSVC)
        add_compile_options(/WX)
    else()
        add_compile_options(-Werror)
    endif()
endif()

# ============================================================================
# HEADER-ONLY LIBRARY INTERFACE
# ============================================================================

add_library(fp20 INTERFACE)
target_include_directories(fp20 INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# ============================================================================
# TESTING FRAMEWORK
# ============================================================================

enable_testing()

# Fetch Catch2 for runtime tests (with caching)
include(FetchContent)
set(FETCHCONTENT_QUIET FALSE)
FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.5.0
    GIT_SHALLOW TRUE  # Shallow clone for faster download
)
FetchContent_MakeAvailable(Catch2)

# TDD option - enforces test-first development
option(FP20_STRICT_TDD "Enforce strict TDD: fail build if tests don't exist before implementation" ON)

# ============================================================================
# AUTO-DISCOVERY MODULE
# ============================================================================

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/AutoDiscovery.cmake)

# Auto-discover all tests
fp20_auto_discover_tests()

# ============================================================================
# PRECOMPILED HEADERS - TURBO BOOST
# ============================================================================

# Apply PCH to all test targets
if(TARGET compilation_tests)
    fp20_setup_pch(compilation_tests)
    message(STATUS "PCH: Applied to compilation_tests")
endif()

if(TARGET unit_tests)
    fp20_setup_pch(unit_tests)
    message(STATUS "PCH: Applied to unit_tests")
endif()

if(TARGET runtime_tests)
    fp20_setup_pch(runtime_tests)
    message(STATUS "PCH: Applied to runtime_tests")
endif()

if(TARGET property_tests)
    fp20_setup_pch(property_tests)
    message(STATUS "PCH: Applied to property_tests")
endif()

# ============================================================================
# CUSTOM TARGETS
# ============================================================================

# Custom target to run all tests
add_custom_target(test_all
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure --parallel ${N}
    DEPENDS
        $<TARGET_NAME_IF_EXISTS:compilation_tests>
        $<TARGET_NAME_IF_EXISTS:unit_tests>
        $<TARGET_NAME_IF_EXISTS:runtime_tests>
        $<TARGET_NAME_IF_EXISTS:property_tests>
    COMMENT "Running all tests in parallel (TDD verification)"
)

# ============================================================================
# BUILD SUMMARY
# ============================================================================

message(STATUS "========================================")
message(STATUS "fp++20 BUILD CONFIGURATION")
message(STATUS "========================================")
message(STATUS "C++ Standard: C++${CMAKE_CXX_STANDARD}")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Generator: ${CMAKE_GENERATOR}")
message(STATUS "TDD Mode: ${FP20_STRICT_TDD}")
message(STATUS "Parallel Jobs: ${CMAKE_BUILD_PARALLEL_LEVEL}")
if(CCACHE_PROGRAM)
    message(STATUS "CCache: ${CCACHE_PROGRAM}")
endif()
if(CMAKE_INTERPROCEDURAL_OPTIMIZATION)
    message(STATUS "LTO: Enabled")
endif()
message(STATUS "========================================")
message(STATUS "READY TO BUILD AT LIGHT SPEED!")
message(STATUS "========================================")
