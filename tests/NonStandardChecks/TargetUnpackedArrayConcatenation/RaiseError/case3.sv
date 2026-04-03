module incorrect1;
  int a[2];
  int b[2];

  initial begin
    {a, b} = '{1,2,3,4};
  end
endmodule