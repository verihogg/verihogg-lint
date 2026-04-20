package pkg;
    covergroup cg;
        cp: coverpoint sig;
    endgroup
endpackage

module top;
    int sig;
    covergroup cg;
        cp: coverpoint sig;
    endgroup
endmodule
