# AUTO-DISCOVERY CMAKE MODULE - NINJA MODE ACTIVATED
# Automatically discovers and registers tests without manual configuration

# Function to auto-discover and create test executables
function(fp20_auto_discover_tests)
    message(STATUS "========================================")
    message(STATUS "AUTO-DISCOVERY: Scanning for tests...")
    message(STATUS "========================================")

    # Discover compilation tests (static_assert only)
    file(GLOB_RECURSE COMPILATION_TEST_FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/tests/compilation/*.cpp"
    )

    # Filter out Catch2-based tests and RED phase tests (intentionally failing)
    list(FILTER COMPILATION_TEST_FILES EXCLUDE REGEX ".*test_compile_time_functor\\.cpp$")
    list(FILTER COMPILATION_TEST_FILES EXCLUDE REGEX ".*test_monad_concept\\.cpp$")

    # Discover unit tests
    file(GLOB_RECURSE UNIT_TEST_FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/tests/unit/*.cpp"
    )

    # Discover runtime tests
    file(GLOB_RECURSE RUNTIME_TEST_FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/tests/runtime/*.cpp"
    )

    # Discover property tests
    file(GLOB_RECURSE PROPERTY_TEST_FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/tests/properties/*.cpp"
    )

    # Filter out test_main.cpp files (used as common main)
    list(FILTER UNIT_TEST_FILES EXCLUDE REGEX ".*test_main\\.cpp$")
    list(FILTER RUNTIME_TEST_FILES EXCLUDE REGEX ".*test_main\\.cpp$")
    list(FILTER PROPERTY_TEST_FILES EXCLUDE REGEX ".*test_main\\.cpp$")

    # Count tests
    list(LENGTH COMPILATION_TEST_FILES COMP_COUNT)
    list(LENGTH UNIT_TEST_FILES UNIT_COUNT)
    list(LENGTH RUNTIME_TEST_FILES RUNTIME_COUNT)
    list(LENGTH PROPERTY_TEST_FILES PROP_COUNT)

    message(STATUS "Found ${COMP_COUNT} compilation tests")
    message(STATUS "Found ${UNIT_COUNT} unit tests")
    message(STATUS "Found ${RUNTIME_COUNT} runtime tests")
    message(STATUS "Found ${PROP_COUNT} property tests")

    # Create compilation tests executable
    if(COMPILATION_TEST_FILES)
        add_executable(compilation_tests ${COMPILATION_TEST_FILES})
        target_link_libraries(compilation_tests PRIVATE fp20)
        # Apply fast-math only to our code (not dependencies)
        if(CMAKE_BUILD_TYPE STREQUAL "Release" AND NOT MSVC)
            target_compile_options(compilation_tests PRIVATE -ffast-math -O3)
        endif()
        # Disable unused variable warnings for compilation tests (they test type system)
        if(NOT MSVC)
            target_compile_options(compilation_tests PRIVATE -Wno-unused-variable -Wno-unused-but-set-variable)
        endif()
        add_test(NAME CompilationTests COMMAND compilation_tests)
        message(STATUS "Created: compilation_tests executable")
    endif()

    # Create unit tests executable
    if(UNIT_TEST_FILES)
        add_executable(unit_tests ${UNIT_TEST_FILES})
        target_link_libraries(unit_tests PRIVATE fp20 Catch2::Catch2WithMain)
        # Apply fast-math only to our code (not dependencies)
        if(CMAKE_BUILD_TYPE STREQUAL "Release" AND NOT MSVC)
            target_compile_options(unit_tests PRIVATE -ffast-math -O3)
        endif()
        # Disable unused warnings for test code (focuses on behavior, not production code)
        if(NOT MSVC)
            target_compile_options(unit_tests PRIVATE -Wno-unused-variable -Wno-unused-parameter)
        endif()
        add_test(NAME UnitTests COMMAND unit_tests)
        message(STATUS "Created: unit_tests executable")
    endif()

    # Create runtime tests executable
    if(RUNTIME_TEST_FILES)
        add_executable(runtime_tests ${RUNTIME_TEST_FILES})
        target_link_libraries(runtime_tests PRIVATE fp20 Catch2::Catch2WithMain)
        # Apply fast-math only to our code (not dependencies)
        if(CMAKE_BUILD_TYPE STREQUAL "Release" AND NOT MSVC)
            target_compile_options(runtime_tests PRIVATE -ffast-math -O3)
        endif()
        # Disable unused warnings for test code
        if(NOT MSVC)
            target_compile_options(runtime_tests PRIVATE -Wno-unused-variable -Wno-unused-parameter)
        endif()
        add_test(NAME RuntimeTests COMMAND runtime_tests)
        message(STATUS "Created: runtime_tests executable")
    endif()

    # Create property tests executable
    if(PROPERTY_TEST_FILES)
        add_executable(property_tests ${PROPERTY_TEST_FILES})
        target_link_libraries(property_tests PRIVATE fp20 Catch2::Catch2WithMain)
        # Apply fast-math only to our code (not dependencies)
        if(CMAKE_BUILD_TYPE STREQUAL "Release" AND NOT MSVC)
            target_compile_options(property_tests PRIVATE -ffast-math -O3)
        endif()
        # Disable unused warnings for test code
        if(NOT MSVC)
            target_compile_options(property_tests PRIVATE -Wno-unused-variable -Wno-unused-parameter)
        endif()
        add_test(NAME PropertyTests COMMAND property_tests)
        message(STATUS "Created: property_tests executable")
    endif()

    message(STATUS "========================================")
    message(STATUS "AUTO-DISCOVERY: Complete!")
    message(STATUS "========================================")
endfunction()

# Function to setup precompiled headers for a target
function(fp20_setup_pch target)
    target_precompile_headers(${target} PRIVATE
        # Standard library headers commonly used in FP
        <vector>
        <optional>
        <functional>
        <memory>
        <string>
        <variant>
        <type_traits>
        <concepts>
        <utility>
        <tuple>
        <algorithm>
        <ranges>
        <iostream>
        <stdexcept>
        <cassert>
    )
endfunction()
