`ifdef FOO
module m;
endmodule
`else
module n;
endmodule
`endif

`ifndef BAR
`define BAR
`endif

`include "child.svh"
