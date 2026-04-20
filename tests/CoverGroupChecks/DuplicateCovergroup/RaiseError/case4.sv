module m;
    int a;
    covergroup cg @(posedge clk);
        option.per_instance = 1;
        cp: coverpoint a;
    endgroup

    covergroup cg @(posedge clk); 
        option.per_instance = 0;
        cp2: coverpoint a;
    endgroup
endmodule
