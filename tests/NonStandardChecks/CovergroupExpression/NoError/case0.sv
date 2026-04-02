module covergroup_expr_valid;
  logic [7:0] data;

  covergroup cg (int threshold) @(posedge data[0]);
    option.per_instance = 1;
    cp_data: coverpoint data {
      bins low  = {[0:threshold-1]};
      bins high = {[threshold:255]};
    }
  endgroup

  cg cg_inst = new(128);
endmodule
