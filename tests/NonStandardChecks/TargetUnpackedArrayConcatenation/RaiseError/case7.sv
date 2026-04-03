module incorrect2;
  int a[2];
  int b;

  initial begin
    {a, b} = '{1,2,3};
  end
endmodule