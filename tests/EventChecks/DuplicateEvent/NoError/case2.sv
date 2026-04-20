module outer;
  event ev_outer;

  module inner;
    event ev_inner;
    event ev_outer; 
  endmodule
endmodule
