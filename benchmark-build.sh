#!/bin/bash

# CMAKE WIZARD - BUILD PERFORMANCE BENCHMARK

set -e

echo "========================================"
echo "CMAKE WIZARD - BUILD BENCHMARK"
echo "========================================"
echo ""

# Colors
GREEN='\033[0;32m'
BLUE='\033[0;34m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Get number of cores
CORES=$(sysctl -n hw.ncpu)
echo "System: macOS with $CORES cores"
echo ""

# Test 1: No-change build
echo -e "${BLUE}[TEST 1]${NC} No-change build (should be instant)..."
cd build
RESULT=$(time ninja 2>&1 | grep "no work to do")
cd ..
if [ -n "$RESULT" ]; then
    echo -e "${GREEN}PASS${NC} - No work to do (instant)"
else
    echo -e "${YELLOW}WARN${NC} - Work was done (unexpected)"
fi
echo ""

# Test 2: Single file incremental build
echo -e "${BLUE}[TEST 2]${NC} Incremental build (1 file change)..."
touch tests/unit/test_functor_laws.cpp
cd build
START=$(date +%s.%N)
ninja > /dev/null 2>&1
END=$(date +%s.%N)
ELAPSED=$(echo "$END - $START" | bc)
cd ..
echo -e "Time: ${YELLOW}${ELAPSED}s${NC}"
if (( $(echo "$ELAPSED < 2.0" | bc -l) )); then
    echo -e "${GREEN}PASS${NC} - Build < 2 seconds (OBJECTIVE MET!)"
else
    echo -e "${YELLOW}WARN${NC} - Build >= 2 seconds (still acceptable)"
fi
echo ""

# Test 3: Multiple files incremental build
echo -e "${BLUE}[TEST 3]${NC} Incremental build (3 files change)..."
touch tests/unit/test_functor_laws.cpp
touch tests/unit/test_applicative_laws.cpp
touch tests/compilation/test_concepts.cpp
cd build
START=$(date +%s.%N)
ninja > /dev/null 2>&1
END=$(date +%s.%N)
ELAPSED=$(echo "$END - $START" | bc)
cd ..
echo -e "Time: ${YELLOW}${ELAPSED}s${NC}"
if (( $(echo "$ELAPSED < 5.0" | bc -l) )); then
    echo -e "${GREEN}PASS${NC} - Build < 5 seconds"
else
    echo -e "${YELLOW}WARN${NC} - Build >= 5 seconds"
fi
echo ""

# Test 4: Test execution
echo -e "${BLUE}[TEST 4]${NC} Test execution time..."
cd build
START=$(date +%s.%N)
ctest --output-on-failure > /dev/null 2>&1 || true
END=$(date +%s.%N)
ELAPSED=$(echo "$END - $START" | bc)
cd ..
echo -e "Time: ${YELLOW}${ELAPSED}s${NC}"
echo ""

# Configuration summary
echo "========================================"
echo "CONFIGURATION SUMMARY"
echo "========================================"
cd build
echo "Generator: $(grep CMAKE_GENERATOR CMakeCache.txt | cut -d'=' -f2)"
echo "Build Type: $(grep CMAKE_BUILD_TYPE CMakeCache.txt | grep -v "//" | cut -d'=' -f2)"
echo "Parallel Jobs: $(grep CMAKE_BUILD_PARALLEL_LEVEL CMakeCache.txt | grep -v "//" | cut -d'=' -f2)"

CCACHE=$(grep CMAKE_CXX_COMPILER_LAUNCHER CMakeCache.txt | grep -v "//" | cut -d'=' -f2)
if [ -n "$CCACHE" ]; then
    echo "CCache: ENABLED ($CCACHE)"
else
    echo "CCache: NOT INSTALLED (install with: brew install ccache)"
fi

LTO=$(grep CMAKE_INTERPROCEDURAL_OPTIMIZATION CMakeCache.txt | grep -v "//" | grep "TRUE" || echo "")
if [ -n "$LTO" ]; then
    echo "LTO: ENABLED"
else
    echo "LTO: DISABLED"
fi
cd ..

echo ""
echo "========================================"
echo -e "${GREEN}BENCHMARK COMPLETE!${NC}"
echo "========================================"
echo ""
echo "RECOMMENDATIONS:"
if [ -z "$CCACHE" ]; then
    echo "  - Install ccache for 90% faster rebuilds: brew install ccache"
fi
echo "  - Use 'ninja' instead of 'make' for best performance"
echo "  - Incremental builds should be < 2 seconds"
echo ""
echo "See BUILD_OPTIMIZATION_REPORT.md for details"
