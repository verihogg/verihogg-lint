module m;
  integer fd;
  initial begin
    fd = $fopen("log.txt", "w");
    $fdisplay(fd, "value = %0d", 1);
    $fwrite(fd, "value = %0d", 2);
    $strobe("value = %0d", 3);
    $monitor("value = %0d", 4);
  end
endmodule