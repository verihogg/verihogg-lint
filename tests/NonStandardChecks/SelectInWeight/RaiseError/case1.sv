module invalid_randsequence_select_weight;

  int array[3] = '{5, 10, 15};
  int add;

  initial begin
    randsequence(seq)
      seq : first;

      first : add := array[0]
            | add := array[1];
    endsequence
  end

endmodule