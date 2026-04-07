#!/usr/bin/env bash

set -euo pipefail

ctest \
    --output-on-failure \
    --test-dir build
