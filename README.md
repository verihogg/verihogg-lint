## verihogg-lint

**verihogg-lint** is a SystemVerilog linter built on top of [Surelog](https://github.com/alainmarcel/Surelog).  
The project is intended for static code analysis and checking compliance with coding rules and standards.

---

## Project structure

- `linter/src` – implementation of linter rules.  
- `linter/include` – linter header files.  
- `build.nix` / `shell.nix` – Nix definitions for build and development shell.  

---

## Usage

### Recommended: via prebuilt Docker image

You can use the prebuilt image published to GitHub Container Registry:

#### Single file

```bash
cd /path/to/sv/file.sv
docker run --rm -v "$(pwd)":/data -w /data ghcr.io/verihogg/verihogg-lint:latest verihogg-lint file.sv -nobuiltin
```

#### Using filelist (.f)

The recommended way to lint entire projects is using a filelist.

Example `files.f`:

```
+incdir+rtl/include
+define+SYNTHESIS
rtl/pkg/common_pkg.sv
rtl/interfaces/bus_if.sv
rtl/core/top.sv
rtl/core/alu.sv
rtl/core/regfile.sv
```

Run with Docker:

```bash
cd /path/to/project
docker run --rm -v "$(pwd)":/data -w /data ghcr.io/verihogg/verihogg-lint:latest verihogg-lint -f files.f -nobuiltin
```

The filelist format supports:
- Include directories: `+incdir+<path>`
- Defines: `+define+<name>` or `+define+<name>=<value>`
- File paths: relative or absolute paths to SystemVerilog files

#### Alternative: Multiple files or wildcards

You can also pass multiple files or use wildcards:

```bash
# Multiple files
cd /path/to/project
docker run --rm -v "$(pwd)":/data -w /data ghcr.io/verihogg/verihogg-lint:latest verihogg-lint file1.sv file2.sv file3.sv -nobuiltin

# All .sv files in current directory
docker run --rm -v "$(pwd)":/data -w /data ghcr.io/verihogg/verihogg-lint:latest verihogg-lint *.sv -nobuiltin

# All .sv files recursively
docker run --rm -v "$(pwd)":/data -w /data ghcr.io/verihogg/verihogg-lint:latest verihogg-lint **/*.sv -nobuiltin

# Specific directory
docker run --rm -v "$(pwd)":/data -w /data ghcr.io/verihogg/verihogg-lint:latest verihogg-lint /path/to/project/**/*.sv -nobuiltin
```

---

## Build and run locally

### 1. Build from source (Nix)

Use Nix to build the project and all required dependencies in a reproducible environment.
This command compiles the project and creates a `result` symlink that points to the
built output in the Nix store.

```bash
nix-build
```

### 2. Run from console (local build)

#### Single file

To lint a single SystemVerilog file:

```bash
./result/bin/verihogg-lint /path/to/your_file/my_sv_code.sv -nobuiltin
```

#### Using filelist (.f)

The recommended way to lint entire projects is using a filelist:

Example `files.f`:

```
+incdir+rtl/include
+define+SYNTHESIS
rtl/pkg/common_pkg.sv
rtl/interfaces/bus_if.sv
rtl/core/top.sv
rtl/core/alu.sv
rtl/core/regfile.sv
```

Run with filelist:

```bash
./build/bin/verihogg-lint -f files.f -nobuiltin
```

#### Alternative: Multiple files or wildcards

You can also pass multiple files or use wildcards:

```bash
# Multiple files
./build/bin/verihogg-lint file1.sv file2.sv file3.sv -nobuiltin

# All .sv files in current directory
./build/bin/verihogg-lint *.sv -nobuiltin

# All .sv files recursively
./build/bin/verihogg-lint **/*.sv -nobuiltin

# Specific directory
./build/bin/verihogg-lint /path/to/project/**/*.sv -nobuiltin
```

You can run `./build/bin/verihogg-lint --help` to see all available options.

---

## Autofix

Several rules support automatic fix suggestions. The linter can apply them directly or export them for later review.

### Fixable rules

| Rule | Fix |
|------|-----|
| `CLASS_VARIABLE_LIFETIME` | Removes `automatic` keyword |
| `TIME_VALUE` | Removes whitespace between number and time unit |
| `EXPONENT_FORMAT_TIME_VALUE` | Converts exponent notation to plain integer (e.g. `1.0e6` → `1000000`) |
| `WILDCARD_EQUALITY_OPERATOR` | Replaces `=?=` with `==?` |
| `WILDCARD_INEQUALITY_OPERATOR` | Replaces `!?=` with `!=?` |
| `TYPE_CASTING` | Inserts missing `'` before cast expression |
| `ASSIGNMENT_PATTERN` | Replaces concatenation with assignment pattern `'{...}` |
| `MULTIPLE_DOT_STAR_CONNECTIONS` | Removes duplicate `.*` port connections |
| `VOID_CAST_OF_VOID_FUNCTION` | Removes `void'(...)` wrapper from void function calls |
| `METHOD_OVERRIDE_ARGUMENT_NAME` | Renames mismatched argument names to match the prototype |

### Autofix flags

```
--fix                         Apply all fixable suggestions in-place
--fix-dry-run                 Print a diff to stdout without modifying files
--show-suggestions            Print fix hints with before/after snippets
--export-fixes=<file>         Export fixes to a YAML file without applying
--apply-fixes-from=<file>     Apply fixes from a previously exported YAML file
--backup-suffix=<suffix>      Save original files before --fix (e.g. --backup-suffix=.bak)
```

### Examples

Apply fixes directly:
```bash
verihogg-lint --fix file.sv -nobuiltin
```

Preview changes without modifying files:
```bash
verihogg-lint --fix-dry-run file.sv -nobuiltin
```

Export fixes for review, then apply separately:
```bash
verihogg-lint --export-fixes=fixes.yaml file.sv -nobuiltin
verihogg-lint --apply-fixes-from=fixes.yaml
```

Apply fixes with backup:
```bash
verihogg-lint --fix --backup-suffix=.bak file.sv -nobuiltin
```

---

## Testing

You can launch local tests, which are build using GTest framework

To run tests:
```bash
ctest --output-on-failure --test-dir build
```
