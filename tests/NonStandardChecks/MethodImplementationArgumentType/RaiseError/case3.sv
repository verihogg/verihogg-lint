typedef logic [7:0] byte_t;
typedef int         count_t;

class DataProcessor;
  extern function void process(logic [7:0] data, int count);
endclass

function void DataProcessor::process(byte_t data, count_t count);
endfunction
