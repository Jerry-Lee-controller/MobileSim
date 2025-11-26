#!/usr/bin/env bash
# Simple build helper that works regardless of the current working directory.
set -euo pipefail
SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
SRC_FILE="$SCRIPT_DIR/src/main.cpp"
OUTPUT="$SCRIPT_DIR/flightsim"

if [ ! -f "$SRC_FILE" ]; then
  echo "[error] Could not find $SRC_FILE. Make sure the repository is intact." >&2
  exit 1
fi

echo "[info] Building flightsim from $SRC_FILE" >&2
g++ -std=c++17 "$SRC_FILE" -o "$OUTPUT"
echo "[done] Built $OUTPUT" >&2
