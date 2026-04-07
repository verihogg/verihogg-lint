#!/usr/bin/env bash

set -euo pipefail

readonly CLANG_FORMAT_BIN="clang-format"

if ! command -v "${CLANG_FORMAT_BIN}" >/dev/null 2>&1; then
  echo "error: ${CLANG_FORMAT_BIN} is required but was not found" >&2
  exit 1
fi

find linter tests \
  -type f \
  \( -name '*.cpp' -o -name '*.h' \) \
  -print0 | xargs -0 "${CLANG_FORMAT_BIN}" --dry-run --Werror
