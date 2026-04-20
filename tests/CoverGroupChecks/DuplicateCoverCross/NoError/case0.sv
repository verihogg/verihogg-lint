module mod1;
    int a, b;
    covergroup cg @(posedge clk);
        cp_a: coverpoint a;
        cp_b: coverpoint b;
        cr: cross cp_a, cp_b;
    endgroup
endmodule

module mod2;
    int x, y;
    covergroup cg @(posedge clk);
        cp_x: coverpoint x;
        cp_y: coverpoint y;
        cr: cross cp_x, cp_y;  
    endgroup
endmodule
