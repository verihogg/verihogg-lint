#!/usr/bin/env sh
set -u

BINARY="/nix/var/nix/profiles/default/bin/verihogg-lint"
FIXTURES_DIR="/verihogg-test-fixtures"

UVM_FIXTURE="/verihogg-test-uvm/smoke.sv"

PASS=0
FAIL=0
ERRORS=""

if [ ! -f "$BINARY" ] && [ ! -L "$BINARY" ]; then
    echo "FATAL: binary not found at $BINARY"
    exit 1
fi

echo "UVM:"

"$BINARY" --uvm-version > /tmp/lint_out.txt 2>&1
uvm_ver_exit=$?
if [ "$uvm_ver_exit" -eq 0 ]; then
    PASS=$((PASS + 1))
    printf "  PASS [exit 0]  --uvm-version: %s\n" "$(cat /tmp/lint_out.txt)"
else
    FAIL=$((FAIL + 1))
    printf "  FAIL [exit %s, expected 0]  --uvm-version\n%s\n" "$uvm_ver_exit" "$(cat /tmp/lint_out.txt)"
    ERRORS="${ERRORS}\nFAIL: --uvm-version (exit $uvm_ver_exit)"
fi

if [ -f "$UVM_FIXTURE" ]; then
    "$BINARY" --uvm -nobuiltin "$UVM_FIXTURE" > /tmp/lint_out.txt 2>&1
    uvm_with_exit=$?
    if [ "$uvm_with_exit" -eq 0 ]; then
        PASS=$((PASS + 1))
        printf "  PASS [exit 0]  --uvm resolves built-in library (%s)\n" "$UVM_FIXTURE"
    else
        FAIL=$((FAIL + 1))
        printf "  FAIL [exit %s, expected 0]  --uvm %s\n%s\n" "$uvm_with_exit" "$UVM_FIXTURE" "$(cat /tmp/lint_out.txt)"
        ERRORS="${ERRORS}\nFAIL: --uvm flag (exit $uvm_with_exit)"
    fi

    "$BINARY" -nobuiltin "$UVM_FIXTURE" > /tmp/lint_out.txt 2>&1
    uvm_without_exit=$?
    if [ "$uvm_without_exit" -ne 0 ]; then
        PASS=$((PASS + 1))
        printf "  PASS [exit non-0]  UVM fixture correctly fails without --uvm\n"
    else
        FAIL=$((FAIL + 1))
        printf "  FAIL [exit 0, expected non-0]  UVM fixture should fail without --uvm\n"
        ERRORS="${ERRORS}\nFAIL: UVM fixture unexpectedly passed without --uvm"
    fi
fi

echo ""
echo "Lint rules:"

for rule_dir in "$FIXTURES_DIR"/*/; do
    rule_name=$(basename "$rule_dir")

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

    fix_dir="$rule_dir/Fix"
    if [ -d "$fix_dir" ]; then
        for sv_file in "$fix_dir"/case*.sv; do
            [ -f "$sv_file" ] || continue

            stem=$(basename "$sv_file" .sv)
            case "$stem" in *.fixed) continue ;; esac

            expected_file="${sv_file%.sv}.fixed.sv"
            [ -f "$expected_file" ] || continue

            tmp_file="/tmp/verihogg_fix_${rule_name}_${stem}.sv"
            cp "$sv_file" "$tmp_file"

            "$BINARY" --fix "$tmp_file" > /tmp/lint_out.txt 2>&1

            if [ "$(cat "$tmp_file")" = "$(cat "$expected_file")" ]; then
                PASS=$((PASS + 1))
                printf "  PASS [fix]  %s\n" "$sv_file"
            else
                FAIL=$((FAIL + 1))
                printf "  FAIL [fix mismatch]  %s\n" "$sv_file"
                ERRORS="${ERRORS}\nFAIL Fix: $sv_file (output mismatch)"
            fi

            rm -f "$tmp_file"
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