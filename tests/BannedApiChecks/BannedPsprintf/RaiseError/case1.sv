class c1;
  int id;
  function string get_name();
    string s;
    s = $psprintf("item_%0d", id);
    return s;
  endfunction
endclass
