module user_type_cast_example;

  typedef int my_int_t;

  my_int_t a;
  int b;

  initial begin
    a = 42;

    b = my_int_t(a);  

    $display("b = %0d", b);
  end

endmodule