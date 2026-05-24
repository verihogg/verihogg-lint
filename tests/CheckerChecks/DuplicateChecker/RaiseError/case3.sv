program test;
  checker P();
    assert property (1);
  endchecker

  checker P();
    assert property (0);
  endchecker
endprogram
