module array_correct;
    logic [3:0] signals [0:7];

    initial begin
        @(signals[0][0]) // бит 0 элемента 0
        $display("signals[0][0] changed");
    end

    initial begin
        @(signals[0] or signals[1] or signals[2])
        $display("one of first 3 elements changed");
    end

    logic [31:0] bus_flat;
    assign bus_flat = {signals[7], signals[6], signals[5], signals[4],
                       signals[3], signals[2], signals[1], signals[0]};

    initial begin
        @(bus_flat) // packed vector — это singular type
        $display("bus_flat changed: %0h", bus_flat);
    end
endmodule