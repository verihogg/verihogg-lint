package p1;

  class packet;
    int data;

    task print();
      $display("data=%0d", data);
    endtask
  endclass

endpackage


module top;
  import p1::*;

  task packet::print();
    $display("packet data=%0d", data);
  endtask

  initial begin
    packet p = new();
    p.data = 42;
    p.print();
  end
endmodule