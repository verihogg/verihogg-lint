module top;
    int a, b, c;
    covergroup cg;
        cp_a: coverpoint a;
        cp_b: coverpoint b;
        cp_c: coverpoint c;
        cr1: cross cp_a, cp_b;
        cr2: cross cp_a, cp_b, cp_c; 
    endgroup
endmodule
