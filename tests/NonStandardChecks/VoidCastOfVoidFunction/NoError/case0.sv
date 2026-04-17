module top;

  function int enqueue(ref int queue[$], int val);
    queue.push_back(val);
    return queue.size();
  endfunction

  initial begin
    int q[$];
    void'(enqueue(q, 42));
  end

endmodule