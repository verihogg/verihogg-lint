class inner_t;
  bit [3:0] val;
endclass

class outer_t;
  inner_t sub;
endclass

module cg_deep_access;
  covergroup cg with function sample(outer_t obj);
    cp_deep   : coverpoint obj.sub.val;
  endgroup
endmodule
