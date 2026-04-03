typedef logic [7:0] my_byte_t;

module incorrect_cast_2;
  my_byte_t a;
  int b;
  initial begin
    a = my_byte_t(b);
  end
endmodule