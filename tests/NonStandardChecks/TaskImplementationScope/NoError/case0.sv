class Driver;
  extern task drive(input logic [7:0] data);
endclass

task Driver::drive(input logic [7:0] data);
  $display("driving 0x%0h", data);
endtask

module task_scope_valid;
endmodule
