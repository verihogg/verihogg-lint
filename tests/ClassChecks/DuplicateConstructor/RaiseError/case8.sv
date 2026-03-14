// Package scope with duplicate constructors
package test_pkg;
    class Counter;
        int count;
        
        function new(int init = 0);
            count = init;
        endfunction
        
        function new(int init = 0);
            count = init + 1;
        endfunction
    endclass
endpackage
