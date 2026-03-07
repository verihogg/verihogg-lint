class SimpleClass1;
    int id;
    string name;
    
    function new(int init_id, string init_name);
        id = init_id;
        name = init_name;
        $display("SimpleClass constructor: id=%0d, name=%s", id, name);
    endfunction; // new

   
    
    function void display();
        $display("SimpleClass: id=%0d, name=%s", id, name);
    endfunction
endclass : SimpleClass1

class SimpleClass2;
    int id;
    string name;
    
    function new(int init_id, string init_name);
        id = init_id;
        name = init_name;
        $display("SimpleClass constructor: id=%0d, name=%s", id, name);
    endfunction; // new

   
    
    function void display();
        $display("SimpleClass: id=%0d, name=%s", id, name);
    endfunction
endclass : SimpleClass2

class SimpleClass3;
    int id;
    string name;
    
    function new(int init_id, string init_name);
        id = init_id;
        name = init_name;
        $display("SimpleClass constructor: id=%0d, name=%s", id, name);
    endfunction; // new

   class NestedClass;
          int id;
    string name;

      class NestedNestedClass;
         int id;
         string name;
         
         function new(int init_id, string init_name);
            id = init_id;
            name = init_name;
            $display("SimpleClass constructor: id=%0d, name=%s", id, name);
         endfunction; // new
         
         function new(int init_id, string init_name);
            id = init_id;
            name = init_name;
            $display("SimpleClass constructor: id=%0d, name=%s", id, name);
         endfunction; // new
      endclass; // NestedNestedClass
      
      
      function new(int init_id, string init_name);
         id = init_id;
         name = init_name;
         $display("SimpleClass constructor: id=%0d, name=%s", id, name);
      endfunction; // new

      
   endclass; // NestedClass
   

      
   
    
    function void display();
        $display("SimpleClass: id=%0d, name=%s", id, name);
    endfunction
endclass : SimpleClass3
