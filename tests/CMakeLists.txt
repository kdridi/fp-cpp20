# Test suite configuration
add_library(test_main STATIC test_main.cpp)
target_link_libraries(test_main PUBLIC Catch2::Catch2WithMain fp20)

# Helper function to add tests following TDD principles
function(add_fp20_test test_name)
    add_executable(${test_name} ${ARGN})
    target_link_libraries(${test_name} PRIVATE test_main fp20 Catch2::Catch2)
    add_test(NAME ${test_name} COMMAND ${test_name})

    # TDD enforcement: Check that test file exists before implementation
    if(FP20_STRICT_TDD)
        set_tests_properties(${test_name} PROPERTIES LABELS "TDD")
    endif()
endfunction()

# Unit tests - Following TDD, tests are written FIRST
add_fp20_test(test_functor_concept unit/test_functor_concept.cpp)
add_fp20_test(test_functor_laws unit/test_functor_laws.cpp)
add_fp20_test(test_functor_instances unit/test_functor_instances.cpp)

# Property-based tests for law verification
add_fp20_test(test_functor_properties properties/test_functor_properties.cpp)

# Compilation tests (static_assert based)
add_fp20_test(test_compile_time_functor compilation/test_compile_time_functor.cpp)

# Future tests (currently commented out until we reach these epics)
# add_fp20_test(test_applicative_concept unit/test_applicative_concept.cpp)
# add_fp20_test(test_monad_concept unit/test_monad_concept.cpp)