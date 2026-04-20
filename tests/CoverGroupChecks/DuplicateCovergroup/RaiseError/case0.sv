module top;
    int a;
    covergroup cg;
        cp: coverpoint a;
    endgroup

    covergroup cg;
        cp2: coverpoint a;
    endgroup
endmodule
