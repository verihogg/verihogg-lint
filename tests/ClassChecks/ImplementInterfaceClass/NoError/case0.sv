interface class my_interface;
  pure virtual function void do_something();
endclass

class my_class implements my_interface;
  function void do_something();
    // Implementation
  endfunction
endclass
