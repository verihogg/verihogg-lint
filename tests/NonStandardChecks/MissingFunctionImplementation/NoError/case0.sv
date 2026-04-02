class Alu;
  extern function int add(input int a, b);
  extern function int sub(input int a, b);
endclass

function int Alu::add(input int a, b);
  return a + b;
endfunction

function int Alu::sub(input int a, b);
  return a - b;
endfunction

module missing_func_impl_valid;
endmodule