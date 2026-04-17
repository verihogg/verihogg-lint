class Database;

  function void clear();
  endfunction

  function int commit() ;
    return 5;
  endfunction

  function void finalize();
    clear();

    void'(commit());
  endfunction

endclass

module top;
  initial begin
    Database db = new();
    db.finalize();
  end
endmodule