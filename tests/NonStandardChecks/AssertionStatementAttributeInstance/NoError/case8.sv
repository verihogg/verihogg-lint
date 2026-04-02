module valid_04 (input logic clk, input logic en, input logic data);
    always @(posedge clk) begin
        cover property (en && data);
    end
endmodule