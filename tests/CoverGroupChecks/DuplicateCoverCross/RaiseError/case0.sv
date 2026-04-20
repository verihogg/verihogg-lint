module top;
    int a, b;
    bit en;
    covergroup cg @(posedge clk);
        cp1: coverpoint a;
        cp2: coverpoint b;
        cr: cross cp1, cp2;
        cr: cross cp1, cp2 iff(en);
    endgroup
endmodule
