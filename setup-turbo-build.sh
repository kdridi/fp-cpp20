#!/bin/bash

# CMAKE WIZARD - TURBO BUILD SETUP SCRIPT

set -e

echo "========================================"
echo "CMAKE WIZARD - TURBO BUILD SETUP"
echo "========================================"

# Check for ccache
if ! command -v ccache &> /dev/null; then
    echo "Installing ccache for compilation caching..."
    if command -v brew &> /dev/null; then
        brew install ccache
    else
        echo "WARNING: Homebrew not found. Please install ccache manually."
    fi
else
    echo "ccache: FOUND"
fi

# Check for ninja
if ! command -v ninja &> /dev/null; then
    echo "Installing Ninja build system..."
    if command -v brew &> /dev/null; then
        brew install ninja
    else
        echo "WARNING: Homebrew not found. Please install ninja manually."
    fi
else
    echo "ninja: FOUND"
fi

# Clean previous build
echo "Cleaning previous build directory..."
rm -rf build
mkdir -p build

# Configure with Ninja
echo "Configuring project with Ninja generator..."
cd build
cmake -G Ninja -DCMAKE_BUILD_TYPE=Release ..

echo "========================================"
echo "SETUP COMPLETE!"
echo "========================================"
echo ""
echo "USAGE:"
echo "  cd build"
echo "  ninja                    # Build all"
echo "  ninja test_all          # Build and run all tests"
echo "  ctest --parallel 8      # Run tests in parallel"
echo ""
echo "INCREMENTAL BUILD TEST:"
echo "  touch ../tests/unit/test_functor_laws.cpp"
echo "  time ninja              # Should be < 2 seconds!"
echo "========================================"
