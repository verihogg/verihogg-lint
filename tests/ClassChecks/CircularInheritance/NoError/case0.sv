package linear_inheritance_pkg;

    // Simple linear inheritance chain - NO cycle
    class Grandparent;
        int grandparent_data;
        
        function new();
            grandparent_data = 100;
        endfunction
        
        virtual function void display();
            $display("Grandparent");
        endfunction
    endclass
    
    class Parent extends Grandparent;
        int parent_data;
        
        function new();
            super.new();
            parent_data = 200;
        endfunction
        
        virtual function void display();
            super.display();
            $display("Parent");
        endfunction
    endclass
    
    class Child extends Parent;
        int child_data;
        
        function new();
            super.new();
            child_data = 300;
        endfunction
        
        virtual function void display();
            super.display();
            $display("Child");
        endfunction
    endclass
    
endpackage

module test_linear;
    import linear_inheritance_pkg::*;
    
    initial begin
        Child c = new();
        c.display();  // Should print: Grandparent, Parent, Child
    end
endmodule
