interface my_interface;
    logic enable;
    logic [7:0] data;
    logic valid;
endinterface

module top_correct;
    my_interface cif_enable();

    initial begin
        @(cif_enable.enable)
        $display("triggered on enable");

        @(posedge cif_enable.enable)
        $display("triggered on posedge enable");

        @(cif_enable.valid)
        $display("triggered on valid");
    end
endmodule