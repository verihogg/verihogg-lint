module dpi_valid;
  import "DPI-C" function int  c_add(input int a, b);
  import "DPI"   function void legacy_init();

  export "DPI-C" function sv_callback;

  function void sv_callback();
    $display("called from C");
  endfunction

  initial begin
    int result = c_add(3, 4);
    $display("result = %0d", result);
  end
endmodule
