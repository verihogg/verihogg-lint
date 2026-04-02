module invalid_04 (input logic clk, input logic a, input logic b);
    always @(posedge clk) begin
        cov_wrong_attr_pos: (* synthesis_keep = 1 *) cover property (a |-> b);
    end
endmodule