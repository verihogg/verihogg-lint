// Valid inheritance
class Base;
    int x;
    
    virtual function void method();
        $display("Base");
    endfunction
endclass

class Derived extends Base;
    string y;
    
    function void method();
        $display("Derived");
    endfunction
endclass
