module wrong_array;
  real arr[0:3];
  covergroup cg;
    cp_arr: coverpoint arr[0];
  endgroup
endmodule