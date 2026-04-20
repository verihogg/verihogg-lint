module top;
    int a, b;
    covergroup cg;
        cp_a: coverpoint a;
        cp_b: coverpoint b;
        cp_c: coverpoint a { bins low = {[0:127]}; } 
    endgroup
endmodule
