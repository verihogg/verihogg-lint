module assignment_pattern_valid;
  typedef struct {
    logic [7:0] addr;
    logic [7:0] data;
  } Packet;

  initial begin
    Packet p;
    int arr[3];

    p   = '{addr: 8'hFF, data: 8'hAB};
    arr = '{1, 2, 3};
    $display("addr=%0h data=%0h", p.addr, p.data);
  end
endmodule
