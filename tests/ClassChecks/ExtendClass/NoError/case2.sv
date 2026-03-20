module top;

  class A;

    virtual function void display();
      $display("Display.. Class A");
    endfunction

    function void message();
      $display("Message.. Class A");
    endfunction

  endclass // A
   
endmodule
