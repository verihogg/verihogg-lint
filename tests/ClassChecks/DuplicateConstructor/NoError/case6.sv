// Classes in different modules with same constructor
module mod1;
    class Data;
        int value;
        
        function new(int v);
            value = v;
        endfunction
    endclass
endmodule

module mod2;
    class Data;
        string name;
        
        function new(string n);
            name = n;
        endfunction
    endclass
endmodule
