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
docker run --rm -v "$(pwd)":/data -w /data ghcr.io/verihogg/verihogg-lint:latest lint *.sv -nobuiltin

# All .sv files recursively
docker run --rm -v "$(pwd)":/data -w /data ghcr.io/verihogg/verihogg-lint:latest lint **/*.sv -nobuiltin

# Specific directory
docker run --rm -v "$(pwd)":/data -w /data ghcr.io/verihogg/verihogg-lint:latest lint /path/to/project/**/*.sv -nobuiltin
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
./result/bin/lint /path/to/your_file/my_sv_code.sv -nobuiltin
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
./build/bin/lint -f files.f -nobuiltin
```

#### Alternative: Multiple files or wildcards

You can also pass multiple files or use wildcards:

```bash
# Multiple files
./build/bin/lint file1.sv file2.sv file3.sv -nobuiltin

# All .sv files in current directory
./build/bin/lint *.sv -nobuiltin

# All .sv files recursively
./build/bin/lint **/*.sv -nobuiltin

# Specific directory
./build/bin/lint /path/to/project/**/*.sv -nobuiltin
```

You can run `./build/bin/lint --help` to see all available options.

## Testing

You can launch local tests, which are build using GTest framework

To run tests:
```bash
ctest --output-on-failure --test-dir build
```
