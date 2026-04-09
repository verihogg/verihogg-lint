module array_incorrect;
    logic [3:0] signals [0:7];

    initial begin
        @(signals)
        $display("something changed");
    end
endmodule