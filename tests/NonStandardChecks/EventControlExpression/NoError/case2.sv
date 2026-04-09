typedef struct {
    logic       valid;
    logic [7:0] data;
    logic       ready;
} bus_t;

module struct_correct;
    bus_t bus;

    initial begin
        @(bus.valid)
        $display("valid changed");
    end

    always_comb begin
        $display("bus changed: data = %0h", bus.data);
    end

    event bus_changed;

    always_ff @(posedge bus.valid or posedge bus.ready) begin
        -> bus_changed;
    end

    initial begin
        @(bus_changed)
        $display("bus event fired");
    end
endmodule