class item;
  rand int value;
endclass

module m;
  item it;

  initial begin
    it = new();
    void'(it.randomize());
  end
endmodule
