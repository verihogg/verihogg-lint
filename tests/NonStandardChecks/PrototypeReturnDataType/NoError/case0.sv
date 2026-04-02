class Calculator;
  extern function int    add(input int a, b);
  extern function void   reset();
  extern function logic  is_ready();
endclass

function int Calculator::add(input int a, b);
  return a + b;
endfunction

function void Calculator::reset();
endfunction

function logic Calculator::is_ready();
  return 1;
endfunction

module proto_return_valid;
endmodule