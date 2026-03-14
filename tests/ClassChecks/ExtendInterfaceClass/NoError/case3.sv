// Regular classes extending regular classes (allowed)
class BaseClass;
    virtual function void do_something();
    endfunction
endclass

class DerivedClass extends BaseClass;
    function void do_something();
        $display("Doing");
    endfunction
endclass
