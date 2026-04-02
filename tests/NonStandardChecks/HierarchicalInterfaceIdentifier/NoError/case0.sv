interface MyBus;
  logic       valid;
  logic [7:0] data;
endinterface

module consumer (MyBus bus);
  always_ff @(posedge bus.valid)
    $display("data = %0h", bus.data);
endmodule

module hierarchical_iface_valid;
  MyBus b();
  consumer u_consumer(.bus(b));
endmodule
