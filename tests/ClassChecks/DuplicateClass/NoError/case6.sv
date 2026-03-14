// Base class and derived classes (different names)
class BaseTransaction;
    int id;
    
    virtual function void execute();
    endfunction
endclass

class ReadTransaction extends BaseTransaction;
    bit [31:0] addr;
endclass

class WriteTransaction extends BaseTransaction;
    bit [63:0] data;
endclass
