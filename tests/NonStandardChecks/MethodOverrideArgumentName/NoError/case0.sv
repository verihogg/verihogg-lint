class Packet;
  virtual function void set_length(int pkt_size);
    $display("Packet::set_length(%0d)", pkt_size);
  endfunction

  virtual function bit check_crc(bit [7:0] expected_crc);
    $display("Packet::check_crc(0x%0h)", expected_crc);
    return 1'b1;
  endfunction

  virtual function void print_info(string tag, int pkt_size);
    $display("[%s] size = %0d", tag, pkt_size);
  endfunction
endclass

class EthernetPacket extends Packet;
  virtual function void set_length(int pkt_size);
    int local_frame_size = pkt_size + 18;
    $display("EthernetPacket::set_length(%0d), frame=%0d",
             pkt_size, local_frame_size);
  endfunction
endclass