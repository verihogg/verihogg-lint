class A;

   virtual function void display();
      $display("Display.. Class A");
   endfunction

   function void message();
      $display("Message.. Class A");
   endfunction

endclass // A


class B extends NotExistingClass;

   function void display();
      $display("Display: Class B");
   endfunction

   function void message();
      $display("Message: Class B");
   endfunction

endclass

