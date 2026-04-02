module bad_system_func_args;

  typedef logic [7:0] byte_t;
  typedef struct packed {
    logic [3:0] addr;
    logic [3:0] data;
  } packet_t;

  byte_t   my_byte;
  packet_t my_packet;
  int      my_int;
  logic    my_logic;
  string   type_str;
  integer  i;

  initial begin

    type_str = $typename(my_byte, my_packet);
    $display("Type: %s", type_str);
  end

endmodule