class BaseProcessor;
  typedef logic [15:0] word_t;
  extern virtual function word_t process(word_t in);
endclass

class AdvancedProcessor extends BaseProcessor;
  typedef struct packed {
    word_t  data;
    logic   overflow;
  } result_t;

  extern function result_t compute(word_t in);
endclass

function BaseProcessor::word_t BaseProcessor::process(
  BaseProcessor::word_t in
);
  return in << 1;
endfunction

function AdvancedProcessor::result_t AdvancedProcessor::compute(
  AdvancedProcessor::word_t in
);
  result_t r;
  r.data     = in;
  r.overflow = (in > 16'h7FFF);
  return r;
endfunction