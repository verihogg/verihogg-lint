module incorrect_multi_select;
  reg [3:0] a, b;

  always @(a[1] or b) begin
    a <= a + b;
  end
endmodule