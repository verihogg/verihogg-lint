// Package scope undeclared extern function
package test_pkg;
    class Processor;
        int result;
        function void execute();
    endclass
endpackage

function void test_pkg::Processor::execute();
    result = 42;
endfunction
