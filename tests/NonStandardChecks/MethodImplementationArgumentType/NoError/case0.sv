typedef bit my_type;

class foo;
  extern function bit boo(bit arg);
endclass

function bit foo::boo(bit arg);
  return ~arg;
endfunction
