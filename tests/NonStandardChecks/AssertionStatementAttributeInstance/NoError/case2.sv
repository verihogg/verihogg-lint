module valid_01 (input logic clk, input logic a, input logic b);
    (* cover_attribute = "functional" *)
    cov_with_attr: cover property (
        @(posedge clk) a |-> b
    );
endmodule