## verihogg-lint

**verihogg-lint** is a SystemVerilog linter built on top of [Surelog](https://github.com/alainmarcel/Surelog).  
The project is intended for static code analysis and checking compliance with coding rules and standards.

---

## Implemented rules

- `FATAL_SYSTEM_TASK_FIRST_ARGUMENT`  
  Expecting 0, 1 or 2 as first argument to '$fatal' system task

- `CLASS_VARIABLE_LIFETIME`  
  'automatic' lifetime for class variable not allowed

- `IMPLICIT_DATA_TYPE_IN_DECLARATION`  
  Expecting net type (e.g. wire) or 'var' keyword before implicit data type

- `PARAMETER_DYNAMIC_ARRAY`  
  Fixed size required for parameter dimension

- `HIERARCHICAL_INTERFACE_IDENTIFIER`  
  Hierarchical interface identifier not allowed

- `PROTOTYPE_RETURN_DATA_TYPE`  
  Expecting return data type or void for function prototype

- `DPI_DECLARATION_STRING`  
  Expecting "DPI" or "DPI-C"

- `REPETITION_IN_SEQUENCE`  
  Goto repeat '[->' and non-consecutive repeat '[=' operators not allowed

- `COVERPOINT_EXPRESSION_TYPE`  
  Coverpoint expression should be of an integral data type

- `COVERGROUP_EXPRESSION`  
  Expecting constant expression or non-ref covergroup argument

- `CONCATENATION_MULTIPLIER`  
  Expecting constant expression as concatenation multiplier

- `PARAMETER_OVERRIDE`  
  Expecting parentheses around parameter override

- `MULTIPLE_DOT_STAR_CONNECTIONS`  
  Dot start port connection '.*' cannot appear more than once in the port list

- `SELECT_IN_EVENT_CONTROL`  
  Select in event control not allowed

- `EMPTY_ASSIGNMENT_PATTERN`  
  Empty assignment pattern ‘{} not allowed

- `MISSING_FOR_LOOP_INITIALIZATION`  
  ‘for’ loop variable initialization required

- `MISSING_FOR_LOOP_CONDITION`  
  ‘for’ loop conditional expression required

- `MISSING_FOR_LOOP_STEP`  
  ‘for’ loop step required

- `FOREACH_LOOP_CONDITION`  
  Multidimensional array select not allowed in foreach loop condition

- `SELECT_IN_WEIGHT`  
  Select in weight specification not allowed

- `ASSIGNMENT_PATTERN`  
  Expecting assignment pattern '{…} instead of concatenation

- `ASSIGNMENT_PATTERN_CONTEXT`  
  Assignment pattern not allowed outside assignment-like context (could not determine data type)

- `SCALAR_ASSIGNMENT_PATTERN`  
  Variable of 1-bit scalar type # not allowed as target of assignment pattern

- `TARGET_UNPACKED_ARRAY_CONCATENATION`  
  Unpacked array concatenation not allowed as target expression

- `INSIDE_OPERATOR`  
  'inside' operator in constant expression not allowed

- `INSIDE_OPERATOR_RANGE`  
  Expecting curly braces {} around 'inside' operator range

- `TYPE_CASTING`  
  Expecting tick before type casting expression
  
- `CIRCULAR_INHERITANCE`  
  Class # extends itself
  
- `DUPLICATE_CLASS`  
  Duplicate class #, already declared

- `DUPLICATE_CONSTRUCTOR`  
  Duplicate constructor #, already declared
    
- `EXTEND_CLASS`  
  Extending non existing class #

- `EXTERN_CONSTRAINT_UNDECLARED`  
  Outer class constraint # was not declared extern inside class #

- `EXTERN_FUNCTION_UNDECLARED`  
  Outer class function # was not declared extern inside class #

- `EXTERN_TASK_UNDECLARED`  
  Outer class task # was not declared extern inside class #

- `EXTEND_INTERFACE_CLASS`  
  Extending interface class # by non-interface class not allowed

- `IMPLEMENT_CLASS`  
  Implementing non-interface class # by class not allowed
    
- `IMPLEMENT_INTERFACE_CLASS`  
  Implementing non existing interface class #

- `TIME_VALUE`  
  Unexpected white space between number and time value

- `MULTIPLE_BINS`  
  Specification of multiple bins dimension not allowed

- `ASSERTION_STATEMENT_ATTRIBUTE_INSTANCE`  
  Expecting attribute instance after block identifier # for procedural assertion statement

- `SYSTEM_FUNCTION_ARGUMENTS`  
  Maximum number of arguments for # system function is #

- `WILDCARD_EQUALITY_OPERATOR`  
  Expecting wildcard operator '==?' instead of '=?='

- `WILDCARD_INEQUALITY_OPERATOR`  
  Expecting wildcard operator '!=?' instead of '!?='

- `EXPONENT_FORMAT_TIME_VALUE`  
  Unexpected exponent format for time value

- `NOF_PARAMETER_OVERRIDES`  
  Expected # parameter overrides, found #module; endmodule

- `MISSING_FUNCTION_IMPLEMENTATION`  
  extern function is not implemented

- `MISSING_TASK_IMPLEMENTATION`  
  extern task is not implemented

- `FUNCTION_IMPLEMENTATION_SCOPE`  
  extern function implemented outside of its class scoped

- `TASK_IMPLEMENTATION_SCOPE`  
  extern task implemented outside of its class scope

- `CONSTRAINT_IMPLEMENTATION_SCOPE`  
  extern conxtraint implemented outside of its class scope
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
