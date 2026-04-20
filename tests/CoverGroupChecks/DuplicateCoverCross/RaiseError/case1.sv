module top;
    int a, b, c;
    covergroup cg;
        cp_a: coverpoint a;
        cp_b: coverpoint b;
        cp_c: coverpoint c;
        my_cross: cross cp_a, cp_b;
        my_cross: cross cp_a, cp_c; 
    endgroup
endmodule
