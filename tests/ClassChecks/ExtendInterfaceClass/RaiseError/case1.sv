interface class base_if_a;
    pure virtual function void method_a();
endclass

class Class;
    int value;
    function new(int value);
        this.value = value;
    endfunction : new
endclass : Class

interface class derived_if extends base_if_a, Class;
    pure virtual function void method_c();
endclass
