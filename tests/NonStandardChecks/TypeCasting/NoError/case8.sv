typedef struct packed {
  logic [31:0] haddr;
  logic [31:0] hrdata;
  logic        hwrite;
  logic [3:0]  hstrb;
} type_bus_req_s;

module struct_field_no_cast;
  type_bus_req_s req;

  initial begin
    req.haddr  = 32'hDEAD_BEEF;
    req.hrdata = 32'h0;
    req.hwrite = 1'b1;
  end
endmodule
