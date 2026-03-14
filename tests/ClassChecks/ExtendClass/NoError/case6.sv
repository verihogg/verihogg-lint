// Inheritance in package
package my_pkg;
    class BaseTransaction;
        int id;
        virtual function void execute();
        endfunction
    endclass
    
    class WriteTransaction extends BaseTransaction;
        bit [63:0] data;
        function void execute();
            $display("Write");
        endfunction
    endclass
endpackage
