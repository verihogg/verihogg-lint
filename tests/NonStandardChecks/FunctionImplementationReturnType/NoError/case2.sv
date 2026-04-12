typedef logic [7:0] byte_t;

class packet_parser;
  extern function byte_t get_header();
  extern function byte_t get_payload();
endclass

function byte_t packet_parser::get_header();
  return 8'hAA;
endfunction

function byte_t packet_parser::get_payload();
  return 8'h55;
endfunction

module tb;
  initial begin
    packet_parser pp = new();
    $display("Header:  %h", pp.get_header());
    $display("Payload: %h", pp.get_payload());
  end
endmodule