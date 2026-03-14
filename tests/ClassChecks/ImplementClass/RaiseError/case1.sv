// Implementing non-interface class
class RegularClass;
    int value;
    function void display();
    endfunction
endclass

class Implementor implements RegularClass;
    int id;
    function void display();
        $display("ID: %0d", id);
    endfunction
endclass
