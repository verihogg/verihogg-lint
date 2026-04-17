class Agent;

  function bit get_config(string name, output int val);
    val = 0;
    return 0;
  endfunction

  function void build();
    int cfg_val;
    void'(get_config("timeout", cfg_val));
  endfunction

endclass

module top;
  initial begin
    Agent a = new();
    a.build();
  end
endmodule