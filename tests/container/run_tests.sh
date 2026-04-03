#!/bin/sh
set -u

BINARY="/nix/var/nix/profiles/default/bin/verihogg-lint"
FIXTURES_DIR="/verihogg-test-fixtures"

PASS=0
FAIL=0
ERRORS=""

if [ ! -f "$BINARY" ] && [ ! -L "$BINARY" ]; then
    echo "FATAL: binary not found at $BINARY"
    exit 1
fi

for rule_dir in "$FIXTURES_DIR"/*/; do
    rule_name=$(basename "$rule_dir")

    [ "$rule_name" = "container" ] && continue

    no_error_dir="$rule_dir/NoError"
    if [ -d "$no_error_dir" ]; then
        for sv_file in "$no_error_dir"/case*.sv; do
            [ -f "$sv_file" ] || continue

            "$BINARY" "$sv_file" > /tmp/lint_out.txt 2>&1
            exit_code=$?

            if [ "$exit_code" -eq 0 ]; then
                PASS=$((PASS + 1))
                printf "  PASS [exit 0]  %s\n" "$sv_file"
            else
                FAIL=$((FAIL + 1))
                output=$(cat /tmp/lint_out.txt)
                printf "  FAIL [exit %s, expected 0]  %s\n%s\n" \
                    "$exit_code" "$sv_file" "$output"
                ERRORS="${ERRORS}\nFAIL NoError: $sv_file (exit $exit_code)"
            fi
        done
    fi

    raise_error_dir="$rule_dir/RaiseError"
    if [ -d "$raise_error_dir" ]; then
        for sv_file in "$raise_error_dir"/case*.sv; do
            [ -f "$sv_file" ] || continue

            "$BINARY" "$sv_file" > /tmp/lint_out.txt 2>&1
            exit_code=$?

            if [ "$exit_code" -ne 0 ]; then
                PASS=$((PASS + 1))
                printf "  PASS [exit %s] %s\n" "$exit_code" "$sv_file"
            else
                FAIL=$((FAIL + 1))
                printf "  FAIL [exit 0, expected non-zero]  %s\n" "$sv_file"
                ERRORS="${ERRORS}\nFAIL RaiseError: $sv_file (exit 0)"
            fi
        done
    fi
done

echo ""
echo "========================================"
echo "  Results: Passed=$PASS  Failed=$FAIL"
echo "========================================"

if [ "$FAIL" -gt 0 ]; then
    echo ""
    echo "Failed tests:"
    printf "%b\n" "$ERRORS"
    exit 1
fi

exit 0