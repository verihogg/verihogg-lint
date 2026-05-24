interface bad_if;
  checker D();
    assert property (1);
  endchecker

  checker D();  // duplicate
    assert property (0);
  endchecker
endinterface
