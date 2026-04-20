module top;
    int a;
    covergroup cg;
        my_cp: coverpoint a { bins low = {[0:127]}; }
        my_cp: coverpoint a { bins high = {[128:255]}; } 
    endgroup
endmodule
