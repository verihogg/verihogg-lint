typedef struct packed {
  logic [3:0] cmd;
  logic [31:0] addr;
} type_req_s;

module consumer (
  input logic [3:0] cmd_i,
  input logic [31:0] addr_i
);
endmodule

module top;
  type_req_s queue;

  consumer u_consumer (
    .cmd_i (queue.cmd),
    .addr_i(queue.addr)
  );
endmodule
