// Extend non-existent with typo
class Base;
    virtual function void run();
    endfunction
endclass

class Derived extends Basse;
    function void run();
        $display("Derived run");
    endfunction
endclass
