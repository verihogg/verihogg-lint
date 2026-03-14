// Constructor in parent and child classes
class Parent;
    int x;
    
    function new(int val);
        x = val;
    endfunction
endclass

class Child extends Parent;
    string info;
    
    function new(int val, string s);
        super.new(val);
        info = s;
    endfunction
endclass
