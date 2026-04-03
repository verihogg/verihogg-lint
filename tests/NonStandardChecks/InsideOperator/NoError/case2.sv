module test1;
  int a = 5;
  int b = 3;
  int c = 7;
  initial begin
    if (a inside {b, c}) begin
      $display("ok");
    end
  end
endmodule