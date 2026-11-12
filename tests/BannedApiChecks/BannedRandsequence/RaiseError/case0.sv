module a;
  initial
    repeat (8)
      randsequence (main)
        main : one | two | three;
        one : {$display("one");};
        two : {$display("two");};
        three : {$display("three");};
      endsequence
endmodule
