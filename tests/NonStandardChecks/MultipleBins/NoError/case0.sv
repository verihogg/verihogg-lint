module multiple_bins_valid;
  logic [1:0] mode;

  covergroup cg @(posedge mode[0]);
    cp_mode: coverpoint mode {
      bins idle    = {2'b00};
      bins active  = {2'b01};
      bins stall   = {2'b10};
      bins error   = {2'b11};
    }
  endgroup

  cg cg_inst = new();
endmodule
