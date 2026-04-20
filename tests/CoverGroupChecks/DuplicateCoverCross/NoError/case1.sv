module top;
    int x, y;
    covergroup cg1;
        cp1: coverpoint x;
        cp2: coverpoint y;
        cr: cross cp1, cp2;
    endgroup

    covergroup cg2;
        cp_a: coverpoint x;
        cp_b: coverpoint y;
        cr: cross cp_a, cp_b;
    endgroup
endmodule
