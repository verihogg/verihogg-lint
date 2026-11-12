class c1;
  int id;
  function string get_name();
    return $sformatf("item_%0d", id);
  endfunction
endclass
