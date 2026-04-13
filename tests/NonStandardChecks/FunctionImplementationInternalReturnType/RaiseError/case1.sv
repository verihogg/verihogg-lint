class DataProcessor;

  typedef logic [7:0] byte_t;

  typedef enum logic [1:0] {
    ST_IDLE  = 2'b00,
    ST_BUSY  = 2'b01,
    ST_ERROR = 2'b10
  } state_t;

  byte_t  data_reg;
  state_t current_state;

  extern function byte_t  get_data();

endclass : DataProcessor


function byte_t DataProcessor::get_data();
  return data_reg;
endfunction
