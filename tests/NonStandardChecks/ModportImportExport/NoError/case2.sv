class foo;
    int value;
endclass

interface bar (input logic clk);
    foo field;
    logic [7:0] data;

    modport mp1 (
        input clk,
        output data
    );

endinterface

interface bar_v2 (input logic clk);
    logic [7:0] data;
    logic       valid;

    task automatic write_data(input logic [7:0] val);
        data  = val;
        valid = 1'b1;
    endtask

    function automatic logic [7:0] read_data();
        return data;
    endfunction

    // ✓ export/import только с task/function
    modport mp1 (
        input  clk,
        output data,
        output valid,
        export write_data,
        import read_data
    );

endinterface