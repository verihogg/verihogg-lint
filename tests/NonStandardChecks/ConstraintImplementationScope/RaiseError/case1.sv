package p2;

  class transaction;
    rand bit [7:0] addr;

    constraint c_addr {
      addr inside {[8'h10:8'h1F]};
    }
  endclass

endpackage


module top;
  import p2::*;

  constraint transaction::c_addr {
    addr != 8'h00;
  }

  initial begin
    transaction t = new();
    if (!t.randomize())
      $error("randomize failed");
  end
endmodule