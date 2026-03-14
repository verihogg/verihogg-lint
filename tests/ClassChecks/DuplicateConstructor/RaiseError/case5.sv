// Constructor with default arguments - duplicates
class Config;
    int timeout;
    string mode;
    
    function new(int t = 100, string m = "default");
        timeout = t;
        mode = m;
    endfunction
    
    function new(int t = 100, string m = "default");
        timeout = t * 2;
        mode = m + "_ext";
    endfunction
endclass
