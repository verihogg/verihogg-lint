module m;
  typedef struct packed {
    int display;
  } cfg_t;

  cfg_t cfg;

  initial begin
    cfg.display = 1;
  end
endmodule