class Base;
  virtual function void process(int data, string name);
  endfunction
endclass

class Child extends Base;
  virtual function void process(int value, string name);
  endfunction
endclass

module top;
endmodule
