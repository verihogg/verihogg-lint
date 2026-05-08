class Packet;
  automatic int size;
  logic [7:0] data;

  function new(int s);
    size = s;
  endfunction
endclass

module class_var_bad;
endmodule
