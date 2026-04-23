module concat_multiplier_sysfunc;
  logic [7:0] sig;
  logic [3:0] vec;

  logic [63:0] a = {$bits(sig){1'b0}};
  logic [31:0] b = {$size(vec){1'b1}};
  logic [63:0] c = {$bits(sig)-1{1'b0}};
endmodule
