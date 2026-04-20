module top1;
    int a;
    covergroup cg @(posedge clk);
        option.per_instance = 1;
        cp: coverpoint a;
    endgroup
endmodule

module top2;
    int b;
    covergroup cg @(posedge clk);
        cp2: coverpoint b;
    endgroup
endmodule
