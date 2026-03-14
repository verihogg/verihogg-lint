// Multiple implementations of non-interface class
class NonInterfaceA;
    function void method_a();
    endfunction
endclass

class NonInterfaceB;
    function void method_b();
    endfunction
endclass

class Impl1 implements NonInterfaceA;
    function void method_a();
        $display("A");
    endfunction
endclass

class Impl2 implements NonInterfaceB;
    function void method_b();
        $display("B");
    endfunction
endclass
