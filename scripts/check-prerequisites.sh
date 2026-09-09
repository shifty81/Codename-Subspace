#!/usr/bin/env bash
set -euo pipefail
echo "Codename: Subspace native C++ prerequisites"
command -v cmake >/dev/null || { echo "[FAIL] cmake missing"; exit 1; }
command -v c++ >/dev/null || { echo "[FAIL] C++ compiler missing"; exit 1; }
command -v git >/dev/null && echo "[PASS] git" || echo "[WARN] git missing"
echo "[PASS] cmake"
echo "[PASS] C++ compiler"
