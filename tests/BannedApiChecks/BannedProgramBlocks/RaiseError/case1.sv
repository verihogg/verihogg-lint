program automatic my_program(input logic clk);
  initial begin
    @(posedge clk);
  end
endprogram

module top;
  logic clk;
  my_program p(.clk(clk));
endmodule
