class Transaction;
  bit [31:0] data;
  // External declaration
  extern task delay();
endclass

// Outside the class body
task Transaction::delay();
  #50;
  $display("Time = %0t, Data = %0d", $time, data);
endtask
