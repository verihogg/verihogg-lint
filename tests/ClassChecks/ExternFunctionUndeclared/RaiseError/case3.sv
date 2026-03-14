
module top;

class Parent;
   virtual function void run();
   endfunction
endclass

class Child extends Parent;
   virtual function void run();
endclass // Child

function Child::run();
   $display("Running Child");
endfunction

endmodule; // top
