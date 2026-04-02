module assertion_attr_valid;
  logic clk, req, ack;

  always @(posedge clk) begin
    (* full_case *) assert (req == ack)
      else $error("req/ack mismatch");
  end
endmodule
