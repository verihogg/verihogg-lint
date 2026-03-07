class Transaction;
  bit [31:0] data;
  task delay();
endclass

task Transaction::delay();
  #50;
  $display("Time = %0t, Data = %0d", $time, data);
endtask
