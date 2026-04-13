class DataProcessor;

  typedef logic [7:0] byte_t;

  typedef enum logic [1:0] {
    ST_IDLE  = 2'b00,
    ST_BUSY  = 2'b01,
    ST_ERROR = 2'b10
  } state_t;

  byte_t  data_reg;
  state_t current_state;

  extern function state_t get_state();

endclass : DataProcessor

function state_t DataProcessor::get_state();
  return current_state;
endfunction