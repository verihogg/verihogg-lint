module dp_ram #(parameter int SIZE = 1024) (
  input  logic [$clog2(SIZE)-1:0] addr,
  output logic [31:0]             rdata
);
endmodule

module top;
  parameter int TCM_SIZE = 512;
  logic [$clog2(TCM_SIZE):0] imem_addr;
  logic [31:0]               rdata;

  dp_ram #(.SIZE(TCM_SIZE)) i_ram (
    .addr (imem_addr[$clog2(TCM_SIZE)-1:0]),
    .rdata(rdata)
  );
endmodule
