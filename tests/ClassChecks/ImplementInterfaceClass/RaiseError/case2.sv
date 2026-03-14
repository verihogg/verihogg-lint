// Multiple undefined interface implementations
class Impl1 implements UndefinedInterface1;
    function void run();
        $display("Run");
    endfunction
endclass

class Impl2 implements UndefinedInterface2;
    function void execute();
        $display("Execute");
    endfunction
endclass
