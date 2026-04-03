module correct_select_in_expr;
  reg [3:0] a, b;

  always @(posedge clk) begin
    a <= b[3:0]; // select здесь, но не в event control
  end
endmodule