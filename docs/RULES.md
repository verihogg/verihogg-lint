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
  
- `DUPLICATE_EVENT`  
   Duplicate event #, already declared at line # file #

- `ILLEGAL_EVENT`  
   The following events must be of a singular data type:#

- `DUPLICATE_ENUM_LITERAL`  
     Duplicate enumeration literal #, already declared at line # file #

- `DUPLICATE_COVER_CROSS`  
    Duplicate cover cross #, already declared at line # file #

- `DUPLICATE_COVERGROUP`  
    Duplicate covergroup #, already declared at line # file #

- `DUPLICATE_COVERPOINT`  
   Duplicate coverpoint #, already declared at line # file #
  
