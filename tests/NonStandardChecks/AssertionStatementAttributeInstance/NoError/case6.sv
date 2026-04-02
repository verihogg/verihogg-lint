module valid_03 (input logic clk, input logic a, input logic b);
    cover property (@(posedge clk) a |-> b);
    cover property (@(posedge clk) b |-> a);
endmodule