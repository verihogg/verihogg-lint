class env;
  function void write(string msg);
  endfunction

  task display();
  endtask

  task run();
    write("hello");
    display();
  endtask
endclass