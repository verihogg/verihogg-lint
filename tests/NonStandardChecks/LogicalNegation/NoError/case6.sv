class Transaction;
    int payload;
endclass

class Monitor;
    Transaction last_txn;

    function void check();
        assert (last_txn != null)
            else $fatal(1, "last_txn must not be null");
    endfunction
endclass