// Multiple interface implementations
interface class Readable;
    pure virtual function bit [31:0] read();
endclass

interface class Writable;
    pure virtual function void write(bit [31:0] data);
endclass

class Memory implements Readable, Writable;
    bit [31:0] storage[];
    
    function bit [31:0] read();
        return storage[0];
    endfunction
    
    function void write(bit [31:0] data);
        storage[0] = data;
    endfunction
endclass
