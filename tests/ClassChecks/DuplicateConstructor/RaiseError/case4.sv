// Three constructors - last two are duplicates
class Widget;
    bit [31:0] data;
    
    function new();
        data = 0;
    endfunction
    
    function new(bit [31:0] init_data);
        data = init_data;
    endfunction
    
    function new(bit [31:0] init_data);
        data = init_data ^ 32'hFFFFFFFF;
    endfunction
endclass
