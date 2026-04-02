class Packet;
  rand logic [7:0] data;
  extern constraint data_range;
endclass

constraint Packet::data_range {
  data inside {[8'h01 : 8'hFE]};
}

module constraint_scope_valid;
endmodule
