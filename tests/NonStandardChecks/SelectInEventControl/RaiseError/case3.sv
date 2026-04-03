module incorrect_select_index;
  reg [3:0] a;

  always @(a[1]) begin
    a <= a + 1;
  end
endmodule