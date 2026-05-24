module top;
  checker Double();
    assert property (1);
  endchecker

  checker Double();
    assert property (0);
  endchecker

  Double d1();
endmodule
