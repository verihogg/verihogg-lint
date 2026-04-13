typedef bit my_type;

class foo;
  extern function bit boo(bit arg);
endclass

function bit foo::boo(my_type arg);
  return arg;
endfunction
