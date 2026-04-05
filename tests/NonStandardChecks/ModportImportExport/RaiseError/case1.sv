class foo;
    int data;
    function int get_data();
        return data;
    endfunction
endclass

interface bar (input logic clk);
    foo field;

    modport mp1 (export field);

endinterface

module top;
    logic clk;
    bar dut_if (.clk(clk));
endmodule