module test2;
  typedef struct {
    logic a;
    logic b;
  } my_struct_t;

  my_struct_t s;

  initial begin
    s = '{}; 
  end
endmodule