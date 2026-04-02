module test_foreach_struct_correct;
  typedef struct { int a; int b; } mystruct_t;
  mystruct_t arr[10];

  initial begin
    foreach (arr[i]) begin
      arr[i].a = i;
      arr[i].b = i*2;
    end
  end
endmodule