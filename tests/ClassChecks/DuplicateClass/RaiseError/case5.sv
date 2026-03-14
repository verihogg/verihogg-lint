// Duplicate with inheritance
class BaseClass;
    virtual function void run();
    endfunction
endclass

class BaseClass;
    virtual function void execute();
    endfunction
endclass

class DerivedClass extends BaseClass;
    function void run();
        $display("Derived run");
    endfunction
endclass
