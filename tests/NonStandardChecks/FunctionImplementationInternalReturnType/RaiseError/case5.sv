class BaseProcessor;
  typedef logic [15:0] word_t;
  extern virtual function word_t process(word_t in);
endclass

class AdvancedProcessor extends BaseProcessor;
  typedef struct packed {
    word_t  data;
    logic   overflow;
  } result_t;

endclass

function word_t BaseProcessor::process(word_t in);
  return in << 1;
endfunction
