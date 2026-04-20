interface my_if;
    logic clk;
    int data;
    covergroup cg @(posedge clk);
        cp: coverpoint data;
    endgroup

    covergroup cg @(posedge clk); 
        cp2: coverpoint data;
    endgroup
endinterface
