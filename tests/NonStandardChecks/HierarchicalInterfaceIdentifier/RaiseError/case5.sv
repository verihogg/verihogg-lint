interface intf_c;
  logic clk;
endinterface

module mid;
  intf_c sub_ifc();
endmodule

module child(input intf_c ifc);
endmodule

module top;
  mid u_mid();
  child u2(.ifc(u_mid.sub_ifc.clk));
endmodule
