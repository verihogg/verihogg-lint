class BaseProcessor;
  typedef logic [15:0] word_t;
endclass

class AdvancedProcessor extends BaseProcessor;
  typedef struct packed {
    word_t  data;
    logic   overflow;
  } result_t;

  extern function result_t compute(word_t in);
endclass

function result_t AdvancedProcessor::compute(word_t in);
  result_t r;
  r.data     = in;
  r.overflow = 1'b0;
  return r;
endfunction