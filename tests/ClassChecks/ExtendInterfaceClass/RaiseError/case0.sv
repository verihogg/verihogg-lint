class Class;
    int value;
    function new(int value);
        this.value = value;
    endfunction : new
endclass : Class

interface class derived_if extends Class;
endclass
