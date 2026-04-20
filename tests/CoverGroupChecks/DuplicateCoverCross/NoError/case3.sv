module top;
    int a, b;
    bit en;
    covergroup cg @(posedge clk);
        cp1: coverpoint a;
        cp2: coverpoint b;
        cr1: cross cp1, cp2 iff(en);
        cr2: cross cp1, cp2;
    endgroup
endmodule
