module coverpoint_valid;
  logic [7:0] data;
  logic [1:0] state;

  covergroup cg @(posedge data[0]);
    cp_data:  coverpoint data;
    cp_state: coverpoint state;
  endgroup

  cg cg_inst = new();
endmodule
