module top;
    int a, b;
    covergroup cg;
        cp1: coverpoint a;
        cp2: coverpoint b;
        my_cross: cross cp1, cp2 {
            bins auto = binsof(cp1) intersect {[0:100]};
        }
        my_cross: cross cp1, cp2 {
            bins manual = binsof(cp2) intersect {[200:300]};
        } 
    endgroup
endmodule
