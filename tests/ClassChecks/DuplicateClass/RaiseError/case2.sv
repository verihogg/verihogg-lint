module top;

class SimpleClass;
    int id;
    string name;

   class SimplerClass;
      int a;
      function new (int init_a);
         a = init_a;
         $display("SimplerClass constructor: a=%0d", a);
      endfunction; // new
   endclass; // SimplerClass

   class SimplerClass;
      int a;
      function new (int init_a);
         a = init_a;
         $display("SimplerClass constructor: a=%0d", a);
      endfunction; // new
   endclass; // SimplerClass
   
    
    function new(int init_id, string init_name);
        id = init_id;
        name = init_name;
        $display("SimpleClass constructor: id=%0d, name=%s", id, name);
    endfunction // new
    
    function void display();
        $display("SimpleClass: id=%0d, name=%s", id, name);
    endfunction
endclass : SimpleClass

endmodule; // top
