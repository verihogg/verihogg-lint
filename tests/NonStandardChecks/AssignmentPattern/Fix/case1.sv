module assignment_pattern_bad;
  typedef struct {
    logic [7:0] addr;
    logic [7:0] data;
  } Packet;

  initial begin
    Packet p;
    p = {8'hFF, 8'hAB};
    $display("addr=%0h data=%0h", p.addr, p.data);
  end
endmodule
