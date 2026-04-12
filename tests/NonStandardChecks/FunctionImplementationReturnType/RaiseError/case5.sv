typedef bit        raw_t;
typedef raw_t      cooked_t;
typedef cooked_t   final_t;

class complex_module;
  extern function raw_t    compute_raw();
  extern function cooked_t compute_cooked();
endclass

function cooked_t complex_module::compute_raw();
  return 1'b0;
endfunction

function final_t complex_module::compute_cooked();
  return 1'b1;
endfunction

module tb;
  initial begin
    complex_module cm = new();
    $display("Raw:    %b", cm.compute_raw());
    $display("Cooked: %b", cm.compute_cooked());
  end
endmodule