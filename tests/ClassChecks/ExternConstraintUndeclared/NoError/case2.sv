// Multiple extern constraints properly declared
class Transaction;
    rand int addr;
    rand bit [31:0] data;
    
    extern constraint addr_constraint;
    extern constraint data_constraint;
endclass

constraint Transaction::addr_constraint {
    addr != 0;
}

constraint Transaction::data_constraint {
    data != 32'h0;
}
