interface class my_interface;
  pure virtual function void do_something();
endclass

class my_class implements non_existing_interface;
  function void do_something();
    // Implementation
  endfunction
endclass
