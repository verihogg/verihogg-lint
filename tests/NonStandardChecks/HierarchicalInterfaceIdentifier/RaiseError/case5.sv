interface intf_c;
  logic clk;
endinterface

module child(input intf_c ifc);
endmodule

module top;
  intf_c top_ifc();
  child u1(.ifc(top_ifc));
  child u2(.ifc(top_ifc.sub_ifc));
endmodule