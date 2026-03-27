// For some reason, linter gets stuck at this file in SURELOG::compile function

module top;
   
   class B extends C;

      function void display();
         $display("Display: Class B");
      endfunction

      function void message();
         $display("Message: Class B");
      endfunction

   endclass

   class C extends B;

      // As display() function is virtual in
      // class A, hece class B as well, no
      // need to explicitly diclare it virtual here
      function void display();
         $display("Display: Class C");
      endfunction

      function void message();
         $display("Message: Class C");
      endfunction

   endclass


   A a_h;
   B b_h;
   C c_h;

   initial begin
      a_h = new();
      b_h = new();
      c_h = new();

      a_h = b_h;

      $display("\n\n----------------");
      $display("Accessing object B using class A handle.\n");
      a_h.display();
      a_h.message();
      $display("----------------");


      b_h = c_h;

      $display("\n\n----------------");
      $display("Accessing object C using class B handle.\n");
      b_h.display();
      b_h.message();
      $display("----------------\n\n");
   end

endmodule // top
