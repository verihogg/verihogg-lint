interface class Printable;
    pure virtual function string toString();
endclass : Printable

class MyClass implements Printable;
    int value;

    function new(int value);
        this.value = value;
    endfunction : new

    virtual function string toString();
        return $sformatf("MyClass value: %0d", value);
    endfunction : toString
endclass : MyClass

