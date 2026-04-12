package my_types_pkg;
  typedef logic [31:0] data_word_t;
  typedef logic [7:0]  status_byte_t;
  typedef bit          flag_t;
endpackage

import my_types_pkg::*;

class dut_driver;
  local data_word_t  internal_data;
  local status_byte_t status;

  extern function data_word_t  read_data();
  extern function status_byte_t get_status();
  extern function flag_t        is_ready();
  extern function void          load(input data_word_t d);
endclass

function data_word_t dut_driver::read_data();
  return internal_data;
endfunction

function status_byte_t dut_driver::get_status();
  return status;
endfunction

function flag_t dut_driver::is_ready();
  return (status == 8'hFF) ? 1'b1 : 1'b0;
endfunction

function void dut_driver::load(input data_word_t d);
  internal_data = d;
  status = 8'hFF;
endfunction

module tb;
  initial begin
    dut_driver drv = new();
    drv.load(32'hDEAD_BEEF);
    $display("Data:   %h", drv.read_data());
    $display("Status: %h", drv.get_status());
    $display("Ready:  %b", drv.is_ready());
  end
endmodule