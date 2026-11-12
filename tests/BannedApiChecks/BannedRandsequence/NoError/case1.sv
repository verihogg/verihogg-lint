module a(input logic clk, input logic req, input logic ack);
  sequence req_ack;
    req ##1 ack;
  endsequence

  assert property (@(posedge clk) req_ack);
endmodule
