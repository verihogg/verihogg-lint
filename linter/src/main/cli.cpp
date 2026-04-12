#include "main/cli.h"

#include <array>
#include <cstring>
#include <filesystem>
#include <gsl/span>
#include <iostream>
#include <string>

#include "main/rule_dispatcher.h"

namespace {

struct RuleInfo {
  const char* id;
  const char* description;
};
}  // namespace

namespace cli {

constexpr size_t CONFIG_FILE_ARG_LEN = 13;

static constexpr std::array kRules = std::to_array<RuleInfo>({
    {.id = "FATAL_SYSTEM_TASK_FIRST_ARGUMENT",
     .description =
         "Expecting 0, 1 or 2 as first argument to '$fatal' system task"},
    {.id = "CLASS_VARIABLE_LIFETIME",
     .description = "'automatic' lifetime for class variable not allowed"},
    {.id = "IMPLICIT_DATA_TYPE_IN_DECLARATION",
     .description =
         "Expecting net type (e.g. wire) or 'var' before implicit data type"},
    {.id = "PARAMETER_DYNAMIC_ARRAY",
     .description = "Fixed size required for parameter dimension"},
    {.id = "HIERARCHICAL_INTERFACE_IDENTIFIER",
     .description = "Hierarchical interface identifier not allowed"},
    {.id = "PROTOTYPE_RETURN_DATA_TYPE",
     .description =
         "Expecting return data type or void for function prototype"},
    {.id = "DPI_DECLARATION_STRING",
     .description = R"(Expecting "DPI" or "DPI-C")"},
    {.id = "REPETITION_IN_SEQUENCE",
     .description = "Goto '[-> and non-consecutive '[= operators not allowed"},
    {.id = "COVERPOINT_EXPRESSION_TYPE",
     .description = "Coverpoint expression should be of an integral data type"},
    {.id = "COVERGROUP_EXPRESSION",
     .description =
         "Expecting constant expression or non-ref covergroup argument"},
    {.id = "CONCATENATION_MULTIPLIER",
     .description =
         "Expecting constant expression as concatenation multiplier"},
    {.id = "PARAMETER_OVERRIDE",
     .description = "Expecting parentheses around parameter override"},
    {.id = "MULTIPLE_DOT_STAR_CONNECTIONS",
     .description = "'.*' cannot appear more than once in the port list"},
    {.id = "SELECT_IN_EVENT_CONTROL",
     .description = "Select in event control not allowed"},
    {.id = "EMPTY_ASSIGNMENT_PATTERN",
     .description = "Empty assignment pattern '{}' not allowed"},
    {.id = "MISSING_FOR_LOOP_INITIALIZATION",
     .description = "'for' loop variable initialization required"},
    {.id = "MISSING_FOR_LOOP_CONDITION",
     .description = "'for' loop conditional expression required"},
    {.id = "MISSING_FOR_LOOP_STEP", .description = "'for' loop step required"},
    {.id = "FOREACH_LOOP_CONDITION",
     .description =
         "Multidimensional array select not allowed in foreach loop condition"},
    {.id = "SELECT_IN_WEIGHT",
     .description = "Select in weight specification not allowed"},
    {.id = "ASSIGNMENT_PATTERN",
     .description =
         "Expecting assignment pattern '{...}' instead of concatenation"},
    {.id = "ASSIGNMENT_PATTERN_CONTEXT",
     .description =
         "Assignment pattern not allowed outside assignment-like context"},
    {.id = "SCALAR_ASSIGNMENT_PATTERN",
     .description = "Variable of 1-bit scalar type not allowed as assignment "
                    "pattern target"},
    {.id = "TARGET_UNPACKED_ARRAY_CONCATENATION",
     .description =
         "Unpacked array concatenation not allowed as target expression"},
    {.id = "INSIDE_OPERATOR",
     .description = "'inside' operator in constant expression not allowed"},
    {.id = "INSIDE_OPERATOR_RANGE",
     .description = "Expecting curly braces {} around 'inside' operator range"},
    {.id = "TYPE_CASTING",
     .description = "Expecting tick before type casting expression"},
    {.id = "TIME_VALUE",
     .description = "Unexpected white space between number and time value"},
    {.id = "MULTIPLE_BINS",
     .description = "Specification of multiple bins dimension not allowed"},
    {.id = "ASSERTION_STATEMENT_ATTRIBUTE_INSTANCE",
     .description =
         "Expecting attribute instance after block identifier for assertion"},
    {.id = "SYSTEM_FUNCTION_ARGUMENTS",
     .description = "Maximum number of arguments for system function exceeded"},
    {.id = "WILDCARD_EQUALITY_OPERATOR",
     .description = "Expecting wildcard operator '==?' instead of '=?='"},
    {.id = "WILDCARD_INEQUALITY_OPERATOR",
     .description = "Expecting wildcard operator '!=?' instead of '!?='"},
    {.id = "EXPONENT_FORMAT_TIME_VALUE",
     .description = "Unexpected exponent format for time value"},
    {.id = "NOF_PARAMETER_OVERRIDES",
     .description = "Expected # parameter overrides, found #module; endmodule"},
    {.id = "MISSING_FUNCTION_IMPLEMENTATION",
     .description = "extern function is not implemented"},
    {.id = "MISSING_TASK_IMPLEMENTATION",
     .description = "extern task is not implemented"},
    {.id = "TASK_IMPLEMENTATION_SCOPE",
     .description = "extern task implemented outside of its class scope"},
    {.id = "CONSTRAINT_IMPLEMENTATION_SCOPE",
     .description = "extern constraint implemented outside of its class scope"},
    {.id = "FUNCTION_IMPLEMENTATION_SCOPE",
     .description = "extern function implemented outside of its class scope"},
    {.id = "MODPORT_IMPORT_EXPORT_PORT",
     .description = "еxpected method name instead of interface signal name"},
    {.id = "EVENT_CONTROL_EXPRESSION",
     .description = "еxpected singular data type for event control expression "
                    "instead of type"},
    {.id = "METHOD_OVERRIDE_ARGUMENT_NAME",
     .description = "argument name of method does not match of override"},
    {.id = "FUNCTION_IMPLEMENTATION_RETURN_TYPE",
     .description =
         "return type of function must be the same as prototype return type"},

    {.id = "EXTEND_CLASS", .description = "extending non existing class"},
    {.id = "DUPLICATE_CONSTRUCTOR",
     .description = "duplicate constructor already declared"},
    {.id = "DUPLICATE_CLASS",
     .description = "duplicate class, already declared"},

    {.id = "EXTERN_CONSTRAINT_UNDECLARED",
     .description = "outer class constraint was not declared extern"},
    {.id = "EXTERN_FUNCTION_UNDECLARED",
     .description = "outer class function was not declared extern"},
    {.id = "EXTERN_TASK_UNDECLARED",
     .description = "outer class task was not declared extern"},
    {.id = "EXTEND_INTERFACE_CLASS",
     .description =
         "extending interface class by non-interface class not allowed"},
    {.id = "IMPLEMENT_CLASS",
     .description = "implementing non-interface class by class not allowed"},
    {.id = "IMPLEMENT_INTERFACE_CLASS",
     .description = "implementing non existing interface class"},
    {.id = "CIRCULAR_INHERITANCE", .description = "class extends itself"},
});

static constexpr auto kRuleCount = kRules.size();

auto ParseArgs(gsl::span<const char*> args) -> Options {
  Options opts;

  opts.surelog_args.push_back(args[0]);

  const std::filesystem::path configFileName = DefaultConfigFileName;
  opts.config_file = std::filesystem::current_path() / configFileName;

  for (const auto arg : args.subspan(1)) {
    if (std::strcmp(arg, "--dump-config") == 0) {
      opts.dump_config = true;
      return opts;
    } else if (std::strcmp(arg, "--help") == 0 || std::strcmp(arg, "-h") == 0) {
      opts.show_help = true;
    } else if (std::strcmp(arg, "--version") == 0) {
      opts.show_version = true;
    } else if (std::strcmp(arg, "--list-rules") == 0) {
      opts.show_rules = true;
    } else if (std::strcmp(arg, "--surelog-help") == 0) {
      opts.show_surelog_help = true;
    } else if (std::strncmp(arg, "--config-file", CONFIG_FILE_ARG_LEN) == 0) {
      const std::string strArg{arg};
      const std::string config_file =
          strArg.substr(CONFIG_FILE_ARG_LEN + 1, strArg.size());
      opts.config_file = std::filesystem::path{config_file};
    } else {
      opts.surelog_args.push_back(arg);
    }
  }

  return opts;
}

void PrintVersion() { std::cout << "verihogg-lint " << kVersion << "\n"; }

void PrintHelp(const char* programName) {
  // clang-format off
  std::cout
    << "Usage: " << programName << " [OPTIONS] <file.sv> [<file.sv>...]\n"
    << "       " << programName << " [OPTIONS] -f <filelist>\n"
    << "\n"
    << "A SystemVerilog linter with static analysis rules built on Surelog.\n"
    << "\n"
    << "OPTIONS:\n"
    << "  -h, --help          Show this help and exit\n"
    << "  --version           Show version and exit\n"
    << "  --list-rules        List all available lint rules\n"
    << "  --surelog-help      Show full Surelog parser/elaboration options\n"
    << "\n"
    << "INPUT:\n"
    << "  <file>.sv           SystemVerilog source file\n"
    << "  -f <file>           Filelist (sources, includes, defines)\n"
    << "  +incdir+<dir>       Add include directory (repeatable)\n"
    << "  +define+<name>[=<value>]\n"
    << "                      Define a preprocessor macro (repeatable)\n"
    << "  -nobuiltin          Skip built-in SV classes (recommended)\n"
    << "\n"
    << "FILELIST FORMAT (.f file):\n"
    << "  Lines starting with // or # are comments.\n"
    << "  Example:\n"
    << "    +incdir+rtl/include\n"
    << "    +define+SYNTHESIS\n"
    << "    rtl/pkg/common_pkg.sv\n"
    << "    rtl/core/top.sv\n"
    << "\n"
    << "EXAMPLES:\n"
    << "  " << programName << " file.sv -nobuiltin\n"
    << "  " << programName << " -f files.f -nobuiltin\n"
    << "  " << programName << " --list-rules\n"
    << "\n"
    << "All other flags are forwarded to Surelog (parser/elaboration).\n"
    << "Run '" << programName << " --surelog-help' for the full list.\n";
  // clang-format on
}

void PrintRules() {
  std::cout << "Available lint rules (" << kRuleCount << "):\n";

  for (const auto& rule : kRules) {
    std::cout << "\n  " << rule.id << "\n"
              << "      " << rule.description << "\n";
  }
}

}  // namespace cli
