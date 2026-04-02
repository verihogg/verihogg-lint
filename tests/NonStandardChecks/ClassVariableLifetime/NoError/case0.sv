class Packet;
  int size;
  static int count;
  logic [7:0] data;

  function new(int s);
    size = s;
    count++;
  endfunction
endclass

module class_var_valid;
endmodule
