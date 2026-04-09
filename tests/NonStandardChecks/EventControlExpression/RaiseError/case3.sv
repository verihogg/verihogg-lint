typedef struct {
    logic       valid;
    logic [7:0] data;
    logic       ready;
} bus_t;

module struct_incorrect;
    bus_t bus;

    initial begin
        bus.valid = 0;
        bus.data  = 8'h00;
        bus.ready = 0;

        @(bus)
        $display("bus changed: data = %0h", bus.data);
    end
endmodule