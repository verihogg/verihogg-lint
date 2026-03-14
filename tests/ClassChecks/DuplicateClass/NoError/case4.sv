// Classes in different scopes are allowed
module mod1;
    class MyData;
        int x;
    endclass
endmodule

module mod2;
    class MyData;
        int y;
    endclass
endmodule

program test;
    class MyData;
        int z;
    endclass
endprogram
