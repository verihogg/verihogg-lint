class c1;
  function string psprintf(string fmt);
    return fmt;
  endfunction

  function void foo();
    string s;
    s = psprintf("$psprintf");
  endfunction
endclass
