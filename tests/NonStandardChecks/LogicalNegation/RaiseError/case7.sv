class Transaction;
    int payload;
endclass

class Monitor;
    Transaction last_txn;

    function void check();
        assert (!last_txn == 0)
            else $fatal("last_txn must not be null");
    endfunction
endclass