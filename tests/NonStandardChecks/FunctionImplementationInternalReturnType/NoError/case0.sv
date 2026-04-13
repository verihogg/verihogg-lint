class DataProcessor;

  typedef logic [7:0] byte_t;

  typedef enum logic [1:0] {
    ST_IDLE  = 2'b00,
    ST_BUSY  = 2'b01,
    ST_ERROR = 2'b10
  } state_t;

  typedef struct packed {
    byte_t  payload;
    state_t status;
    logic   valid;
  } packet_t;

  byte_t   data_reg;
  state_t  current_state;
  packet_t last_packet;

  extern function byte_t   get_data();
  extern function state_t  get_state();
  extern function packet_t build_packet(byte_t data, state_t st);

  extern function void reset();

endclass : DataProcessor

function DataProcessor::byte_t DataProcessor::get_data();
  return data_reg;
endfunction


function DataProcessor::state_t DataProcessor::get_state();
  return current_state;
endfunction


function DataProcessor::packet_t DataProcessor::build_packet(
  DataProcessor::byte_t  data,
  DataProcessor::state_t st     
);
  packet_t pkt;
  pkt.payload = data;
  pkt.status  = st;
  pkt.valid   = 1'b1;
  return pkt;
endfunction

function void DataProcessor::reset();
  data_reg      = '0;
  current_state = ST_IDLE;
endfunction