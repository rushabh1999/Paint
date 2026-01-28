#!/bin/bash
# Comprehensive test runner for Configuration Drift Detector

set -e

# Get script directory
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

echo "=================================="
echo "Configuration Drift Detector Tests"
echo "=================================="
echo ""

# Colors for output
GREEN='\033[0;32m'
RED='\033[0;31m'
NC='\033[0m' # No Color

# Track results
TESTS_PASSED=0
TESTS_FAILED=0

# Function to run a test
run_test() {
    local test_name="$1"
    local test_command="$2"
    
    echo "Running: $test_name"
    if eval "$test_command" > /tmp/test_output.log 2>&1; then
        echo -e "${GREEN}✓ PASS${NC}: $test_name"
        TESTS_PASSED=$((TESTS_PASSED + 1))
    else
        echo -e "${RED}✗ FAIL${NC}: $test_name"
        cat /tmp/test_output.log
        TESTS_FAILED=$((TESTS_FAILED + 1))
    fi
    echo ""
}

# Clean previous builds
echo "Cleaning previous builds..."
(cd backend && make clean > /dev/null 2>&1)
echo ""

# Build backend
run_test "Backend Build" "(cd backend && make)"

# Run backend tests
run_test "Backend Unit Tests" "(cd tests/backend && make clean && make run)"

# Run Python tests
run_test "Python CLI Tests" "python3 tests/cli/test_drift_cli.py"

# Test CLI help
run_test "CLI Help Command" "python3 cli/drift_cli.py --help"

# Test backend executable exists
run_test "Backend Executable" "test -x backend/drift_detector"

# Print summary
echo "=================================="
echo "Test Summary"
echo "=================================="
echo -e "Tests Passed: ${GREEN}$TESTS_PASSED${NC}"
echo -e "Tests Failed: ${RED}$TESTS_FAILED${NC}"
echo ""

if [ $TESTS_FAILED -eq 0 ]; then
    echo -e "${GREEN}All tests passed!${NC}"
    exit 0
else
    echo -e "${RED}Some tests failed!${NC}"
    exit 1
fi
