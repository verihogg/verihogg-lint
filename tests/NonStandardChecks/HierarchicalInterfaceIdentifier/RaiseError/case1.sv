interface MyBus;
  logic valid;
  logic [7:0] data;
endinterface

module sub_sub;
  MyBus b();
endmodule

module sub;
  sub_sub ss();
endmodule

module consumer (MyBus bus);
  always_ff @(posedge bus.valid)
    $display("data = %0h", bus.data);
endmodule

module hierarchical_iface_bad;
  sub u_sub();

  consumer u_consumer(
    .bus(u_sub.ss.b)
  );
endmodule