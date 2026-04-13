package types_pkg;
  typedef bit my_type;
endpackage

class foo;
  import types_pkg::my_type;

  extern function bit boo(my_type arg);
endclass

function bit foo::boo(types_pkg::my_type arg);
  return arg;
endfunction