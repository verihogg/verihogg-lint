module m;
  typedef enum {RED, GREEN} color_t;
  initial begin
    fork : w
      $display("%0d", RED);
    join : w
  end
endmodule
