// Under module
module top;
   class Scheduler;
      bit busy;
      task wait_for_ready();
      task set_busy();
   endclass

   task Scheduler::wait_for_ready();
      wait(busy == 1'b0);
   endtask

   task Scheduler::set_busy();
      busy = 1'b1;
   endtask
endmodule; // top

