#!/usr/bin/env bash
#
# run_tests.sh - configure and build the QtTest unit test suites
# (Backend/utest and Frontend/utest), using their own build/ directory so
# this never collides with a regular app build/ folder.
#
# This script only builds the tests - it does NOT run them. Once it
# finishes, run the tests with:
#
#   ctest --output-on-failure
#
# (from inside the build directory this script creates, e.g. `cd build-tests`).
#
# Usage:
#   ./run_tests.sh
#
set -euo pipefail

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$PROJECT_ROOT/build-tests"

mkdir -p "$BUILD_DIR"
cmake -S "$PROJECT_ROOT" -B "$BUILD_DIR" -DBUILD_TESTING=ON
cmake --build "$BUILD_DIR"

echo
echo "Build complete. Run the tests with:"
echo "  cd \"$BUILD_DIR\" && ctest --output-on-failure"