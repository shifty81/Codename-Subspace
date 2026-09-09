#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
ENGINE="$ROOT/engine"
BUILD="$ENGINE/build"

echo "Codename: Subspace native C++ setup"
command -v cmake >/dev/null || { echo "cmake is required"; exit 1; }
command -v c++ >/dev/null || { echo "a C++17 compiler is required"; exit 1; }

cmake -S "$ENGINE" -B "$BUILD" -DCMAKE_BUILD_TYPE=Debug -DSUBSPACE_BUILD_TESTS=ON
cmake --build "$BUILD" -j
ctest --test-dir "$BUILD" --output-on-failure

echo "Native C++ setup/build/test complete."
