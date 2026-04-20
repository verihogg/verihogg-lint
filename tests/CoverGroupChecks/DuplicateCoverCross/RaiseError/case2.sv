module top;
    int a, b;
    covergroup cg;
        cp_a: coverpoint a;
        cp_b: coverpoint b;
        my_cross: cross cp_a, cp_b {
            bins low = binsof(cp_a) intersect {[0:50]};
        }
        my_cross: cross cp_a, cp_b {
            bins high = binsof(cp_a) intersect {[51:100]};
        } 
    endgroup
endmodule
