class foo;
  typedef bit my_type;

  extern function bit boo(my_type arg);
  extern function bit goo(my_type arg);
endclass

function bit foo::boo(foo::my_type arg);
  return arg;
endfunction

function bit foo::goo(foo::my_type arg);
  return ~arg;
endfunction