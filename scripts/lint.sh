#!/usr/bin/env bash

set -euo pipefail

readonly CLANG_TIDY_BIN="clang-tidy"
readonly BUILD_DIR="${1:-build}"

if ! command -v "${CLANG_TIDY_BIN}" >/dev/null 2>&1; then
  echo "error: ${CLANG_TIDY_BIN} is required but was not found" >&2
  exit 1
fi

if [[ ! -f "${BUILD_DIR}/compile_commands.json" ]]; then
  echo "error: ${BUILD_DIR}/compile_commands.json was not found; run CMake configure first" >&2
  exit 1
fi

find linter/src \
  -type f \
  -name '*.cpp' \
  -print0 | xargs -P$(nproc) -0 -n 1 "${CLANG_TIDY_BIN}" -p "${BUILD_DIR}" --quiet
