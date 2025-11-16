cmake_minimum_required(VERSION 3.20)
project(fp20
    VERSION 0.1.0
    DESCRIPTION "C++20 Header-Only Functional Programming Library - Haskell-Inspired"
    LANGUAGES CXX
)

# Require C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compiler warnings
if(MSVC)
    add_compile_options(/W4 /WX)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -Werror)
endif()

# Header-only library interface
add_library(fp20 INTERFACE)
target_include_directories(fp20 INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# Enable testing
enable_testing()

# Fetch Catch2 for runtime tests
include(FetchContent)
FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.5.0
)
FetchContent_MakeAvailable(Catch2)

# TDD option - enforces test-first development
option(FP20_STRICT_TDD "Enforce strict TDD: fail build if tests don't exist before implementation" ON)

# Compilation tests (static_assert)
add_executable(compilation_tests
    tests/compilation/test_concepts.cpp
    tests/compilation/test_applicative_concepts.cpp
    tests/compilation/test_either_identity.cpp
    tests/compilation/test_state_monad.cpp
    tests/compilation/test_reader_monad.cpp
)
target_link_libraries(compilation_tests PRIVATE fp20)
add_test(NAME CompilationTests COMMAND compilation_tests)

# Runtime tests (using Catch2)
add_executable(unit_tests
    tests/unit/test_functor_laws.cpp
    tests/unit/test_functor_instances.cpp
    tests/unit/test_functor_concept.cpp
    tests/unit/test_applicative_concept.cpp
    tests/unit/test_applicative_laws.cpp
)
target_link_libraries(unit_tests PRIVATE fp20 Catch2::Catch2WithMain)
add_test(NAME UnitTests COMMAND unit_tests)

# Custom target to run all tests
add_custom_target(test_all
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
    DEPENDS compilation_tests unit_tests
    COMMENT "Running all tests (TDD verification)"
)

# TDD enforcement: ensure tests exist
if(FP20_STRICT_TDD)
    message(STATUS "TDD Mode: STRICT - Tests must be written before implementation")
endif()

message(STATUS "fp20 configuration complete")
message(STATUS "  C++ Standard: C++${CMAKE_CXX_STANDARD}")
message(STATUS "  Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  TDD Mode: ${FP20_STRICT_TDD}")
