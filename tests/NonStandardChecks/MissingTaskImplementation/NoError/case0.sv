class Driver;
  extern task drive(input logic [7:0] data);
  extern task reset();
endclass

task Driver::drive(input logic [7:0] data);
  $display("driving 0x%0h", data);
endtask

task Driver::reset();
  $display("reset");
endtask

module missing_task_impl_valid;
endmodule