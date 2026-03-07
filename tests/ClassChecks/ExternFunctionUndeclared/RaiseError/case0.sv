class ABC;
    function void display();
endclass

function void ABC::display();
  $display("Hello world from ABC class");
endfunction

module tb;
  initial begin
    ABC abc = new();
    abc.display();
  end
endmodule
