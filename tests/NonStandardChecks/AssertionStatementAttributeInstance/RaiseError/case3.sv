module invalid_01 (input logic clk, input logic a, input logic b);
    cov_no_attr: cover property (
        @(posedge clk) a |-> b
    );
endmodule