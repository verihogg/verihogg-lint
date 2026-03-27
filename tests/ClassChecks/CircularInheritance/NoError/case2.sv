package dag_pkg;

    class Base;
        string name;
        
        function new(string name = "Base");
            this.name = name;
        endfunction
        
        virtual function void show();
            $display("Base: %s", name);
        endfunction
    endclass
    
    class Derived1 extends Base;
        function new(string name = "Derived1");
            super.new(name);
        endfunction
        
        virtual function void show();
            $display("Derived1: %s", name);
        endfunction
    endclass
    
    class Derived2 extends Base;
        Derived1 d1_ref;
        
        function new(string name = "Derived2", Derived1 d1 = null);
            super.new(name);
            this.d1_ref = d1;
        endfunction
        
        virtual function void show();
            $display("Derived2: %s", name);
            if (d1_ref != null) d1_ref.show();
        endfunction
    endclass
    
    class Derived3 extends Base;
        Derived2 d2_ref;
        
        function new(string name = "Derived3", Derived2 d2 = null);
            super.new(name);
            this.d2_ref = d2;
        endfunction
        
        virtual function void show();
            $display("Derived3: %s", name);
            if (d2_ref != null) d2_ref.show();
        endfunction
    endclass
    
endpackage

module test_dag;
    import dag_pkg::*;
    
    initial begin
        Derived1 d1 = new("D1");
        Derived2 d2 = new("D2", d1);
        Derived3 d3 = new("D3", d2);
        
        d3.show();
    end
endmodule
