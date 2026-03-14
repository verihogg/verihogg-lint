// Nested class with duplicate constructors
class Outer;
    class Inner;
        int value;
        
        function new(int v);
            value = v;
        endfunction
        
        function new(int v);
            value = v + 10;
        endfunction
    endclass
endclass
