program test;
  checker Conflict();
    assert property (1);
  endchecker

  checker Conflict();  
    assert property (0);
  endchecker

  initial begin
    Conflict c();
  end
endprogram
