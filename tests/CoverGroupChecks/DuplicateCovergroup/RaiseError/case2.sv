class Outer;
    class Inner;
        int a;
        covergroup cg;
            cp: coverpoint a;
        endgroup

        covergroup cg;
            cp2: coverpoint a;
        endgroup
    endclass
endclass
