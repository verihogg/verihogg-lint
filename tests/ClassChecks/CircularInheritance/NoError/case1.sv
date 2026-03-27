package reference_pkg;

    // Class A contains reference to B, B contains reference to C
    // No cycle: A -> B -> C
    class A;
        B b_ref;
        
        function new(B b);
            this.b_ref = b;
        endfunction
        
        function void process();
            $display("A processing");
            if (b_ref != null) b_ref.process();
        endfunction
    endclass
    
    class B;
        C c_ref;
        
        function new(C c);
            this.c_ref = c;
        endfunction
        
        function void process();
            $display("B processing");
            if (c_ref != null) c_ref.process();
        endfunction
    endclass
    
    class C;
        int data;
        
        function new();
            data = 42;
        endfunction
        
        function void process();
            $display("C processing, data=%0d", data);
        endfunction
    endclass
    
endpackage

module test_references;
    import reference_pkg::*;
    
    initial begin
        C c = new();
        B b = new(c);
        A a = new(b);
        
        a.process();  // A -> B -> C (no cycle)
    end
endmodule
