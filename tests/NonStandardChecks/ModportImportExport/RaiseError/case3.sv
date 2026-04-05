interface my_bus (input logic clk, input logic rst_n);
    logic [7:0] data;
    logic       valid;
    logic       ready;

    modport master (
        output data,
        output valid,
        import ready
    );

endinterface

module producer (my_bus.master bus);
    always_ff @(posedge bus.clk) begin
        bus.data  <= 8'hAB;
        bus.valid <= 1'b1;
    end
endmodule