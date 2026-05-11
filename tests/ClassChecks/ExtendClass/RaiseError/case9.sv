module top;
    class derived_class extends my_class;
        string name;
        
        function new(int initial_data = 0, string initial_name = "default");
            super.new(initial_data);  // Call base class constructor
            name = initial_name;
        endfunction
        
        // Override virtual method
        virtual function void display();
            $display("Derived class - Name: %s, Data: %0d", name, data);
        endfunction
        
        function string get_name();
            return name;
        endfunction
        
        // New method not in base class
        function void set_name(string new_name);
            name = new_name;
        endfunction
    endclass

endmodule
