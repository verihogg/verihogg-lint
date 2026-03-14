// Inheritance in module scope
module top;
    class Controller;
        virtual function void control();
        endfunction
    endclass
    
    class APBController extends Controller;
        bit [31:0] addr;
        function void control();
            $display("APB Control");
        endfunction
    endclass
endmodule
