module m;
  initial begin
    $displayb("%b", 1);
    $displayo("%o", 2);
    $displayh("%h", 3);
    $writeb("%b", 4);
    $writeo("%o", 5);
    $writeh("%h", 6);
  end
endmodule