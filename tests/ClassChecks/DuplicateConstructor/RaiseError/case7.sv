// Multiple duplicate constructors
class Request;
    int id;
    bit [31:0] payload;
    
    function new();
        id = 0;
        payload = 0;
    endfunction
    
    function new();
        id = 1;
        payload = 32'hFFFFFFFF;
    endfunction
    
    function new(int init_id);
        id = init_id;
    endfunction
    
    function new(int init_id);
        id = init_id + 1;
    endfunction
endclass
