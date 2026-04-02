module test_concat_wrong;
  logic [3:0] a, b;
  logic [7:0] x;

  initial begin
    x = {a : '0, b : '1};
  end
endmodule