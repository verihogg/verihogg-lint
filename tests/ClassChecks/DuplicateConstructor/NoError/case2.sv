// Different constructors with different signatures
class MyClass;
    int id;
    string name;
    
    function new();
        id = 0;
        name = "";
    endfunction
    
    function new(int init_id);
        id = init_id;
        name = "";
    endfunction
    
    function new(int init_id, string init_name);
        id = init_id;
        name = init_name;
    endfunction
endclass
