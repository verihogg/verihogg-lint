// Undefined interface in package scope
package test_pkg;
    class Impl implements test_pkg::UndefinedInt;
        function void process();
            $display("Processing");
        endfunction
    endclass
endpackage
