package complex_circular_pkg;

    class X extends Z;
        function void display();
            $display("X");
        endfunction
    endclass
    
    class Y extends X;
        function void display();
            $display("Y");
        endfunction
    endclass
    
    class Z extends Y;
        function void display();
            $display("Z");
        endfunction
    endclass
    
endpackage
