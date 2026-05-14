module coverpoint_iff_valid;
  covergroup cg with function sample(bit [15:0] binary);
    cp_00 : coverpoint binary[15:13] iff (binary[1:0] == 2'b00);
    cp_01 : coverpoint binary[15:13] iff (binary[1:0] == 2'b01);
  endgroup
endmodule
