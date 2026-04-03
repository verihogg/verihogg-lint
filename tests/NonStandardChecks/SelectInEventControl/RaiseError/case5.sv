module incorrect_constant_select;
  reg [7:0] mem [0:15];

  always @(mem[3]) begin 
    mem[3] <= mem[3] + 1;
  end
endmodule