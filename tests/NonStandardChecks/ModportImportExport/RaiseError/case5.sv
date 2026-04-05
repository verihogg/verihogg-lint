interface control_if (input logic clk);
    logic [3:0] opcode;
    logic [7:0] operand_a;
    logic [7:0] operand_b;

    modport ctrl_port (
        input  clk,
        export opcode,   
        export operand_a,   
        export operand_b  
    );

endinterface