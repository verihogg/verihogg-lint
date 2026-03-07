class SimpleClass;
    int id;
    string name;
    
    function new(int init_id, string init_name);
        id = init_id;
        name = init_name;
        $display("SimpleClass constructor: id=%0d, name=%s", id, name);
    endfunction // new
    
    function void display();
        $display("SimpleClass: id=%0d, name=%s", id, name);
    endfunction
endclass : SimpleClass

module top;
   
class SimpleClass;
    int id;
    string name;

   function new(int init_id, string init_name);
        id = init_id;
        name = init_name;
        $display("Another SimpleClass constructor: id=%0d, name=%s", id, name);
    endfunction
    
    function void display();
        $display("SimpleClass: id=%0d, name=%s", id, name);
    endfunction
endclass : SimpleClass

endmodule; // top
