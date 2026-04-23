module array_correct;
    logic [3:0] signals [0:7];

    logic [31:0] bus_flat;
    assign bus_flat = {signals[7], signals[6], signals[5], signals[4],
                       signals[3], signals[2], signals[1], signals[0]};

    initial begin
        @(bus_flat)
        $display("bus_flat changed: %0h", bus_flat);
    end
endmodule
