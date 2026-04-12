typedef bit my_type;

class foo;
  extern function bit boo();
endclass

function bit foo::boo();
  return 1'b1;
endfunction

module tb;
  initial begin
    foo f = new();
    $display("Result: %b", f.boo());
  end
endmodule