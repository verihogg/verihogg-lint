module correct_constant_select;
  reg [7:0] mem [0:15];
  reg [7:0] val;

  always @(posedge clk) begin
    val <= mem[3]; // константный select допустим внутри тела
  end
endmodule