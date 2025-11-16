# COMPILER OPTIMIZATIONS MODULE
# Additional compiler-specific optimizations

# Platform-specific optimizations
if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    # macOS ARM64 optimizations
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "arm64")
        message(STATUS "Applying Apple Silicon optimizations")
        add_compile_options(-mcpu=apple-m1)
    endif()
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    # Linux x86_64 optimizations
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64")
        add_compile_options(-march=native)
    endif()
endif()

# Function to apply aggressive optimizations to a target
function(fp20_apply_aggressive_optimizations target)
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        if(NOT MSVC)
            target_compile_options(${target} PRIVATE
                -O3
                -ffast-math
                -funroll-loops
                -finline-functions
            )
        else()
            target_compile_options(${target} PRIVATE
                /O2
                /Ob2
                /Oi
                /Ot
            )
        endif()
        message(STATUS "Aggressive optimizations applied to ${target}")
    endif()
endfunction()
