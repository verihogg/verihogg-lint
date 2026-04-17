class Config;
    string name;
    function new(string n);
        name = n;
    endfunction
endclass

class Driver;
    Config cfg;

    function void run();
        string label = (!cfg) ? "NO_CFG" : cfg.name;
        $display("label = %s", label);
    endfunction
endclass