#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD="$ROOT/engine/build"
cmake -S "$ROOT/engine" -B "$BUILD" -DCMAKE_BUILD_TYPE=Debug -DSUBSPACE_BUILD_TESTS=ON
cmake --build "$BUILD" --target subspace_engine subspace_game subspace_tests subspace_milestone_tests subspace_production_tests -j
ctest --test-dir "$BUILD" --output-on-failure
echo "Native ship/refinement build and tests passed."
