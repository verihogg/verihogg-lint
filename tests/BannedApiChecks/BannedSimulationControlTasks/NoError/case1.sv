class env;
  function void stop();
  endfunction

  task finish();
  endtask

  task run();
    stop();
    finish();
  endtask
endclass
