module mod1;
  checker C();
    assert property (1);
  endchecker
endmodule

module mod2;
  checker C();
    assert property (0);
  endchecker
endmodule
