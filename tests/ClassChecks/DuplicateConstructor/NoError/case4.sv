// Single constructor only
class SingleConstructor;
    bit [63:0] data;
    
    function new(bit [63:0] init_data);
        data = init_data;
    endfunction
endclass
