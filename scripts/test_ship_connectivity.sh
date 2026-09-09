#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD="$ROOT/engine/build"
cmake -S "$ROOT/engine" -B "$BUILD" -DCMAKE_BUILD_TYPE=Debug -DSUBSPACE_BUILD_TESTS=ON
cmake --build "$BUILD" --target subspace_tests -j
"$BUILD/subspace_tests"
echo "Native ship connectivity/system tests passed."
