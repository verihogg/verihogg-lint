// Base interface classes
interface class base_if_a;
    pure virtual function void method_a();
endclass

interface class base_if_b;
    pure virtual function void method_b();
endclass

// Derived interface class extending multiple bases
interface class derived_if extends base_if_a, base_if_b;
    pure virtual function void method_c();
endclass
