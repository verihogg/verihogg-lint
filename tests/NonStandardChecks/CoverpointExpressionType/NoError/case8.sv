module correct_port(input logic clk, input logic [3:0] p);
    covergroup cg @(posedge clk);
        cp_port_ok: coverpoint p;
    endgroup

    cg cg_inst = new();
endmodule