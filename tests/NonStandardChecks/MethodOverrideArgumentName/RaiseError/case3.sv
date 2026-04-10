class Packet;
  virtual function void set_length(int pkt_size);
    $display("Packet: setting length = %0d", pkt_size);
  endfunction

  virtual function bit check_crc(bit [7:0] expected_crc);
    $display("Packet: checking CRC = %0h", expected_crc);
    return 1'b1;
  endfunction
endclass

class EthernetPacket extends Packet;
  virtual function bit check_crc(bit [7:0] crc_val);
    $display("Ethernet: checking CRC = %0h", crc_val);
    return 1'b0;
  endfunction
endclass