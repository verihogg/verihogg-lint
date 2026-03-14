// Duplicate constructor in inherited class
class Parent;
    int x;
    
    function new(int val);
        x = val;
    endfunction
endclass

class Child extends Parent;
    string name;
    
    function new(string n);
        name = n;
    endfunction
    
    function new(string n);
        name = n + "_copy";
    endfunction
endclass
