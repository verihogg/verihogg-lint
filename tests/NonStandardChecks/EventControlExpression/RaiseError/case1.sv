interface my_interface;
    logic enable;
    logic [7:0] data;
    logic valid;
endinterface

module top_incorrect;
    my_interface cif_enable();

    initial begin
        @(cif_enable)
        $display("triggered");
    end
endmodule