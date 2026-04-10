class Base;
  virtual function void configure(int addr, int data, bit wr_en);
  endfunction

  virtual function void send(int pkt_size = 64, bit verbose = 0);
  endfunction
endclass


class Child extends Base;
  virtual function void configure(int addr, int data, bit write_enable);
  endfunction
endclass