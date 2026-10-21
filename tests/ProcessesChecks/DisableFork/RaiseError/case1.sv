module m;
  task automatic t();
    fork
      #10;
    join_none
    disable fork;
  endtask
endmodule
