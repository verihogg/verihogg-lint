module invalid_03 (input logic clk, input logic req, input logic ack);
    always @(posedge clk) begin
        cov_req_ack: cover property (req |-> ack);
    end
endmodule