// Legal: a real variable declared via typedef. Surelog attaches a phantom net
// to the declaration, but it is a variable (net type 0), not a real net.
typedef real my_real_t;

module top;
  my_real_t x;
endmodule