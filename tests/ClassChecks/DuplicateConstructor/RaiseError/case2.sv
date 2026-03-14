// Multiple constructors with different signatures (overloading) - should be error
class MyClass;
    int id;
    
    function new(int init_id);
        id = init_id;
    endfunction
    
    function new(int init_id);
        id = init_id + 1;
    endfunction
endclass
