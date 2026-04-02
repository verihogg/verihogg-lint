module invalid_02 (input logic clk, input logic a, input logic b, input logic c);
    cov_first: cover property (@(posedge clk) a |-> b);
    cov_second: cover property (@(posedge clk) b |-> c);
endmodule